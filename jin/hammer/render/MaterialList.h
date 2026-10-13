#ifndef MATERIAL_LIST_H
#define MATERIAL_LIST_H

#include <string>
#include <vector>

class MaterialList
{
public:
    static MaterialList& Instance();

    // 从 materials.txt 加载（每行一个路径，如 "materials/brick/brickwall001a.vmt"）
    bool LoadFromFile(const std::string& path);

    // 返回所有材质（大写、去掉 "materials/" 前缀和 ".vmt" 后缀）
    const std::vector<std::string>& GetAll() const { return m_materials; }

    // 返回包含 filter 子串的材质（不区分大小写），最多 maxCount 个
    std::vector<std::string> Filter(const std::string& filter, int maxCount = 200) const;

private:
    MaterialList() = default;
    ~MaterialList() = default;
    MaterialList(const MaterialList&) = delete;
    MaterialList& operator=(const MaterialList&) = delete;

    std::vector<std::string> m_materials;
};

#endif // MATERIAL_LIST_H