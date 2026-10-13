#ifndef FGD_MANAGER_H
#define FGD_MANAGER_H

#include <string>
#include <vector>

// Forward declare the underlying GameData class to hide it from Qt UI files
class GameData;

struct FgdEntityTemplate {
    std::string classname;
    std::string description;
    bool isPointClass = false;
    bool isSolidClass = false;
    
    // Default bounding box dimensions mapped from the FGD size specifiers
    float mins[3] = {-8.0f, -8.0f, -8.0f};
    float maxs[3] = {8.0f, 8.0f, 8.0f};
    
    // Default viewport display wireframe color
    unsigned char r = 220;
    unsigned char g = 160;
    unsigned char b = 0;
};

// ★ 新增：keyvalue 描述
struct FgdKeyValue {
    std::string name;           // "targetname"
    std::string displayName;    // "Name"
    std::string description;
    std::string defaultValue;   // 默认值（字符串形式）
    int         type = 0;       // GDIV_TYPE 枚举值
    bool        readOnly = false;

    // choices / flags 的选项：(value, caption)
    struct Choice { std::string value; std::string caption; };
    std::vector<Choice> choices;

    // flags 的 mask（仅 ivFlags 类型用）
    std::vector<unsigned long> flagMasks;
};

class FgdManager {
public:
    // Singleton entry reference accessor
    static FgdManager& Instance();

    bool LoadFgdFile(const std::string& filePath);
    void Clear();

    // UI Data Feed Query Targets
    std::vector<std::string> GetAvailablePointClasses() const;
    bool FindTemplate(const std::string& classname, FgdEntityTemplate& outTemplate) const;
    std::string GetModelPathForClass(const std::string& classname) const;

    // ★ 新增：拿某个 class 的所有 keyvalue 定义
    bool GetKeyValuesForClass(const std::string& classname,
                              std::vector<FgdKeyValue>& out) const;

private:
    FgdManager();
    ~FgdManager();
    FgdManager(const FgdManager&) = delete;
    FgdManager& operator=(const FgdManager&) = delete;

    GameData* m_pGameData; // Managed underlying static lib allocation context
};

#endif // FGD_MANAGER_H
