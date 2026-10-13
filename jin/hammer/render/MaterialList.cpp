#include "render/MaterialList.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <algorithm>

MaterialList& MaterialList::Instance()
{
    static MaterialList instance;
    return instance;
}

//-----------------------------------------------------------------------------
// "materials/brick/brickwall001a.vmt" -> "BRICK/BRICKWALL001A"
//-----------------------------------------------------------------------------
static bool ConvertPath(const std::string& in, std::string& out)
{
    // 1. 去掉 "materials/" 前缀
    const char* prefix = "materials/";
    size_t prefixLen = strlen(prefix);
    if (in.size() <= prefixLen) return false;
    if (in.compare(0, prefixLen, prefix) != 0) return false;

    std::string s = in.substr(prefixLen);

    // 2. 去掉 ".vmt" 后缀
    const char* ext = ".vmt";
    size_t extLen = strlen(ext);
    if (s.size() <= extLen) return false;
    if (s.compare(s.size() - extLen, extLen, ext) != 0) return false;
    s = s.substr(0, s.size() - extLen);

    // 3. 转大写
    for (auto& c : s)
    {
        if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    }

    out = s;
    return true;
}

//-----------------------------------------------------------------------------
bool MaterialList::LoadFromFile(const std::string& path)
{
    m_materials.clear();

    std::ifstream file(path);
    if (!file.is_open())
    {
        printf("[MAT] Cannot open '%s'\n", path.c_str());
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        // 去掉行尾 \r（Windows 换行）
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        // 跳过空行
        if (line.empty()) continue;

        std::string mat;
        if (ConvertPath(line, mat))
            m_materials.push_back(mat);
    }

    printf("[MAT] Loaded %zu materials from '%s'\n",
           m_materials.size(), path.c_str());
    return !m_materials.empty();
}

//-----------------------------------------------------------------------------
std::vector<std::string> MaterialList::Filter(const std::string& filter, int maxCount) const
{
    std::vector<std::string> result;
    result.reserve(maxCount);

    if (filter.empty())
    {
        // 无 filter：返回前 maxCount 个
        int n = (int)m_materials.size();
        if (n > maxCount) n = maxCount;
        result.assign(m_materials.begin(), m_materials.begin() + n);
        return result;
    }

    // 转大写 filter
    std::string upperFilter = filter;
    for (auto& c : upperFilter)
    {
        if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    }

    for (const auto& mat : m_materials)
    {
        if (mat.find(upperFilter) != std::string::npos)
        {
            result.push_back(mat);
            if ((int)result.size() >= maxCount) break;
        }
    }
    return result;
}