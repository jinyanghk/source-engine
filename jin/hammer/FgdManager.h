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

class FgdManager {
public:
    // Singleton entry reference accessor
    static FgdManager& Instance();

    bool LoadFgdFile(const std::string& filePath);
    void Clear();

    // UI Data Feed Query Targets
    std::vector<std::string> GetAvailablePointClasses() const;
    bool FindTemplate(const std::string& classname, FgdEntityTemplate& outTemplate) const;

private:
    FgdManager();
    ~FgdManager();
    FgdManager(const FgdManager&) = delete;
    FgdManager& operator=(const FgdManager&) = delete;

    GameData* m_pGameData; // Managed underlying static lib allocation context
};

#endif // FGD_MANAGER_H
