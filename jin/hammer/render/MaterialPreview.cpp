#include "render/MaterialPreview.h"

#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
#include <sstream>

#include "filesystem.h"
#include "tier1/utlbuffer.h"
#include "bitmap/imageformat.h"
#include "vtf/vtf.h"

extern IFileSystem* g_pFileSystem;

//-----------------------------------------------------------------------------
MaterialPreview& MaterialPreview::Instance()
{
    static MaterialPreview instance;
    return instance;
}

//-----------------------------------------------------------------------------
MaterialPreview::~MaterialPreview()
{
    ClearCache();
}

//-----------------------------------------------------------------------------
void MaterialPreview::ClearCache()
{
    for (auto& kv : m_cache)
    {
        if (kv.second != 0)
            glDeleteTextures(1, &kv.second);
    }
    m_cache.clear();
}

//-----------------------------------------------------------------------------
// 从 .vmt 读 $basetexture
// materialPath 形如 "BRICK/BRICKWALL001A"（大写）
// 返回小写的 basetexture 路径，如 "brick/brickwall001a"
//-----------------------------------------------------------------------------
std::string MaterialPreview::ParseBaseTextureFromVMT(const std::string& materialPath)
{
    std::string lower = materialPath;
    for (auto& c : lower) c = (char)tolower(c);

    std::string vmtPath = "materials/" + lower + ".vmt";

    FileHandle_t f = g_pFileSystem->Open(vmtPath.c_str(), "rb", "GAME");
    if (!f)
    {
        printf("[PREVIEW] Cannot open VMT: %s\n", vmtPath.c_str());
        return "";
    }

    int size = g_pFileSystem->Size(f);
    if (size <= 0 || size > 1024 * 1024)
    {
        g_pFileSystem->Close(f);
        return "";
    }

    std::vector<char> buf(size + 1);
    g_pFileSystem->Read(buf.data(), size, f);
    g_pFileSystem->Close(f);
    buf[size] = '\0';

    std::string text(buf.data());

    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line))
    {
        size_t comment = line.find("//");
        if (comment != std::string::npos)
            line = line.substr(0, comment);

        // 提取所有引号字符串
        std::vector<std::string> tokens;
        size_t p = 0;
        while (true)
        {
            size_t a = line.find('"', p);
            if (a == std::string::npos) break;
            size_t b = line.find('"', a + 1);
            if (b == std::string::npos) break;
            tokens.push_back(line.substr(a + 1, b - a - 1));
            p = b + 1;
        }

        // 找 "$basetexture"
        if (tokens.size() >= 2 && tokens[0] == "$basetexture")
        {
            std::string result = tokens[1];
            for (auto& c : result)
            {
                if (c == '\\') c = '/';
                c = (char)tolower(c);
            }
            //printf("[PREVIEW] Parsed $basetexture = '%s'\n", result.c_str());
            return result;
        }
    }

    printf("[PREVIEW] No $basetexture in %s\n", vmtPath.c_str());
    return "";
}

//-----------------------------------------------------------------------------
// 加载 .vtf 并上传到 OpenGL
//-----------------------------------------------------------------------------
GLuint MaterialPreview::LoadMaterialPreview(const std::string& materialPath)
{
    //printf("[PREVIEW] Loading '%s'\n", materialPath.c_str());

    // 1. 从 .vmt 找 $basetexture
    std::string baseTex = ParseBaseTextureFromVMT(materialPath);
    if (baseTex.empty())
    {
        printf("[PREVIEW] No basetexture for '%s'\n", materialPath.c_str());
        return 0;
    }
    //printf("[PREVIEW] basetexture = '%s'\n", baseTex.c_str());

    // 2. 读 .vtf 文件
    std::string vtfPath = "materials/" + baseTex + ".vtf";
    FileHandle_t f = g_pFileSystem->Open(vtfPath.c_str(), "rb", "GAME");
    if (!f)
    {
        printf("[PREVIEW] Cannot open VTF: %s\n", vtfPath.c_str());
        return 0;
    }

    int size = g_pFileSystem->Size(f);
    if (size <= 0)
    {
        g_pFileSystem->Close(f);
        return 0;
    }

    std::vector<byte> fileData(size);
    g_pFileSystem->Read(fileData.data(), size, f);
    g_pFileSystem->Close(f);

    // 3. 解码 VTF
    IVTFTexture* pVTF = CreateVTFTexture();
    if (!pVTF)
    {
        printf("[PREVIEW] CreateVTFTexture failed\n");
        return 0;
    }

    CUtlBuffer utlBuf(fileData.data(), size, CUtlBuffer::READ_ONLY);
    if (!pVTF->Unserialize(utlBuf))
    {
        printf("[PREVIEW] VTF Unserialize failed: %s\n", vtfPath.c_str());
        DestroyVTFTexture(pVTF);
        return 0;
    }

    int fullW = pVTF->Width();
    int fullH = pVTF->Height();
    //printf("[PREVIEW] VTF size: %dx%d, mips=%d, format=%d\n",
    //       fullW, fullH, pVTF->MipCount(), (int)pVTF->Format());

    // 4. 找一个 ≤128 的 mip 层
    int targetMip = 0;
    int mipW = fullW, mipH = fullH, mipD = 1;
    for (int m = 0; m < pVTF->MipCount(); m++)
    {
        int w, h, d;
        pVTF->ComputeMipLevelDimensions(m, &w, &h, &d);
        targetMip = m;
        mipW = w;
        mipH = h;
        mipD = d;
        if (w <= 64 && h <= 64)
            break;
    }

    // 5. 转 RGBA8888
    pVTF->ConvertImageFormat(IMAGE_FORMAT_RGBA8888, false);

    unsigned char* data = pVTF->ImageData(0, 0, targetMip);
    if (!data)
    {
        printf("[PREVIEW] ImageData failed\n");
        DestroyVTFTexture(pVTF);
        return 0;
    }

    // 6. 上传 OpenGL
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mipW, mipH, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);

    DestroyVTFTexture(pVTF);

    //printf("[PREVIEW] OK: '%s' -> tex=%u (%dx%d, mip=%d)\n",
    //       materialPath.c_str(), tex, mipW, mipH, targetMip);

    return tex;
}

//-----------------------------------------------------------------------------
GLuint MaterialPreview::GetPreviewTexture(const std::string& materialPath)
{
    auto it = m_cache.find(materialPath);
    if (it != m_cache.end())
        return it->second;

    GLuint tex = LoadMaterialPreview(materialPath);
    m_cache[materialPath] = tex;
    return tex;
}