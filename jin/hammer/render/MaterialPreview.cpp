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
        if (kv.second.thumbnailTexID != 0)
            glDeleteTextures(1, &kv.second.thumbnailTexID);
        if (kv.second.worldRenderTexID != 0)
            glDeleteTextures(1, &kv.second.worldRenderTexID);
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
            return result;
        }
    }

    printf("[PREVIEW] No $basetexture in %s\n", vmtPath.c_str());
    return "";
}

//-----------------------------------------------------------------------------
// ★ FIX: Clean routing wrapper maintaining backward compatibility for UI panels ★
//-----------------------------------------------------------------------------
GLuint MaterialPreview::GetPreviewTexture(const std::string& materialPath)
{
    // Simply fetch our unified dual-resource container and forward the thumbnail handle
    MaterialTextureResource_t res = GetMaterialResource(materialPath);
    return res.thumbnailTexID;
}

//-----------------------------------------------------------------------------
MaterialTextureResource_t MaterialPreview::LoadMaterialResource(const std::string& materialPath)
{
    MaterialTextureResource_t res;

    std::string baseTex = ParseBaseTextureFromVMT(materialPath);
    if (baseTex.empty()) return res;

    std::string vtfPath = "materials/" + baseTex + ".vtf";
    FileHandle_t f = g_pFileSystem->Open(vtfPath.c_str(), "rb", "GAME");
    if (!f) return res;

    int size = g_pFileSystem->Size(f);
    if (size <= 0) { g_pFileSystem->Close(f); return res; }

    std::vector<byte> fileData(size);
    g_pFileSystem->Read(fileData.data(), size, f);
    g_pFileSystem->Close(f);

    IVTFTexture* pVTF = CreateVTFTexture();
    if (!pVTF) return res;

    CUtlBuffer utlBuf(fileData.data(), size, CUtlBuffer::READ_ONLY);
    if (!pVTF->Unserialize(utlBuf)) { DestroyVTFTexture(pVTF); return res; }

    res.nativeWidth = pVTF->Width();
    res.nativeHeight = pVTF->Height();

    int targetMip = 0;
    int mipW = res.nativeWidth, mipH = res.nativeHeight, mipD = 1;
    for (int m = 0; m < pVTF->MipCount(); m++)
    {
        pVTF->ComputeMipLevelDimensions(m, &mipW, &mipH, &mipD);
        targetMip = m;
        if (mipW <= 64 && mipH <= 64) break;
    }

    pVTF->ConvertImageFormat(IMAGE_FORMAT_RGBA8888, false);

    // UPLOAD HANDLE A: Thumbnail Preview for ImGui Drop-Downs
    unsigned char* thumbData = pVTF->ImageData(0, 0, targetMip);
    if (thumbData)
    {
        glGenTextures(1, &res.thumbnailTexID);
        glBindTexture(GL_TEXTURE_2D, res.thumbnailTexID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mipW, mipH, 0, GL_RGBA, GL_UNSIGNED_BYTE, thumbData);
    }

    // UPLOAD HANDLE B: Full-Resolution Layer for 3D Viewport Brush Faces
    unsigned char* worldData = pVTF->ImageData(0, 0, 0); 
    if (worldData)
    {
        glGenTextures(1, &res.worldRenderTexID);
        glBindTexture(GL_TEXTURE_2D, res.worldRenderTexID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res.nativeWidth, res.nativeHeight, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, worldData);
        glGenerateMipmap(GL_TEXTURE_2D); 
    }

    DestroyVTFTexture(pVTF);
    return res;
}

//-----------------------------------------------------------------------------
MaterialTextureResource_t MaterialPreview::GetMaterialResource(const std::string& materialPath)
{
    auto it = m_cache.find(materialPath);
    if (it != m_cache.end()) return it->second;

    MaterialTextureResource_t res = LoadMaterialResource(materialPath);
    m_cache[materialPath] = res;
    return res;
}
