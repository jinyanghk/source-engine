#include "app/FgdManager.h"

#include <cstdio>
#include <cstring>
#include <strings.h>   // strcasecmp

#include "fgdlib/ieditortexture.h"
#include "fgdlib/gamedata.h"
#include "fgdlib/gdclass.h"
#include "fgdlib/gdvar.h"       // GDinputvariable
#include "fgdlib/helperinfo.h"  // CHelperInfo

//-----------------------------------------------------------------------------
FgdManager& FgdManager::Instance() {
    static FgdManager instance;
    return instance;
}

FgdManager::FgdManager() {
    m_pGameData = new GameData();
}

FgdManager::~FgdManager() {
    Clear();
    delete m_pGameData;
}

bool FgdManager::LoadFgdFile(const std::string& filePath) {
    if (!m_pGameData) return false;

    Clear();
    bool ok = (m_pGameData->Load(filePath.c_str()) == TRUE);
    printf("[FgdManager] LoadFgdFile('%s') -> %s, classes = %d\n",
           filePath.c_str(), ok ? "OK" : "FAILED",
           m_pGameData->GetClassCount());
    fflush(stdout);
    return ok;
}

void FgdManager::Clear() {
    if (m_pGameData) {
        m_pGameData->ClearData();
    }
}

std::vector<std::string> FgdManager::GetAvailablePointClasses() const {
    std::vector<std::string> pointClasses;
    if (!m_pGameData) return pointClasses;

    int totalClasses = m_pGameData->GetClassCount();
    for (int i = 0; i < totalClasses; ++i) {
        GDclass* pClass = m_pGameData->GetClass(i);
        if (pClass && pClass->IsPointClass() && !pClass->IsBaseClass()) {
            pointClasses.push_back(pClass->GetName());
        }
    }
    return pointClasses;
}

bool FgdManager::FindTemplate(const std::string& classname, FgdEntityTemplate& outTemplate) const {
    if (!m_pGameData) return false;

    GDclass* pClass = m_pGameData->ClassForName(classname.c_str());
    if (!pClass) return false;

    outTemplate.classname = pClass->GetName();
    outTemplate.description = pClass->GetDescription();
    outTemplate.isPointClass = pClass->IsPointClass();
    outTemplate.isSolidClass = pClass->IsSolidClass();

    if (pClass->HasBoundBox()) {
        Vector mins = pClass->GetMins();
        Vector maxs = pClass->GetMaxs();
        outTemplate.mins[0] = mins.x; outTemplate.mins[1] = mins.y; outTemplate.mins[2] = mins.z;
        outTemplate.maxs[0] = maxs.x; outTemplate.maxs[1] = maxs.y; outTemplate.maxs[2] = maxs.z;
    }

    color32 fgdColor = pClass->GetColor();
    outTemplate.r = fgdColor.r;
    outTemplate.g = fgdColor.g;
    outTemplate.b = fgdColor.b;

    return true;
}

// 内部辅助：在 class 及所有 base class 里找 studio 模型
static std::string FindModelRecursive(GDclass* pClass, int depth)
{
    if (!pClass) return "";
    if (depth > 16) return "";   // 防止循环继承

    // 1. 当前 class 的 studio helper
    int helperCount = pClass->GetHelperCount();
    for (int i = 0; i < helperCount; ++i) {
        CHelperInfo* pHelper = pClass->GetHelper(i);
        if (!pHelper || !pHelper->GetName()) continue;
        if (strcasecmp(pHelper->GetName(), "studio") != 0) continue;

        int paramCount = pHelper->GetParameterCount();
        if (paramCount > 0 && pHelper->GetParameter(0)) {
            std::string mdlPath = pHelper->GetParameter(0);
            // 去引号
            if (mdlPath.size() >= 2 && mdlPath.front() == '"' && mdlPath.back() == '"')
                mdlPath = mdlPath.substr(1, mdlPath.size() - 2);
            // 反斜杠转正斜杠
            for (auto& c : mdlPath) if (c == '\\') c = '/';
            if (!mdlPath.empty()) return mdlPath;
        }
    }

    // 2. 递归 base classes
    int baseCount = pClass->GetBaseCount();
    for (int i = 0; i < baseCount; ++i) {
        GDclass* pBase = pClass->GetBase(i);
        std::string r = FindModelRecursive(pBase, depth + 1);
        if (!r.empty()) return r;
    }

    return "";
}

//-----------------------------------------------------------------------------
// 递归：在当前 class 及所有 base class 里查找 "model" 变量的默认值。
// FGD 里模型定义有两种写法：
//   1) body 里的 `model(studio) : "World model" : "models/alyx.mdl"` —— 默认值是模型路径
//   2) helper 参数 `studio("models/alyx.mdl")` —— 老写法，参数就是模型路径
// 我们两种都查。
//-----------------------------------------------------------------------------
static std::string FindModelInClassRecursive(GDclass* pClass, int depth)
{
    if (!pClass) return "";
    if (depth > 16) return "";

    // 方法 1: "model" 变量的默认值
    int varIndex = -1;
    GDinputvariable* pModelVar = pClass->VarForName("model", &varIndex);
    if (pModelVar) {
        char defaultBuf[MAX_STRING];
        defaultBuf[0] = '\0';
        pModelVar->GetDefault(defaultBuf);

        if (defaultBuf[0]) {
            std::string mdlPath = defaultBuf;
            // 去引号
            if (mdlPath.size() >= 2 && mdlPath.front() == '"' && mdlPath.back() == '"')
                mdlPath = mdlPath.substr(1, mdlPath.size() - 2);
            // 反斜杠转正斜杠
            for (auto& c : mdlPath) if (c == '\\') c = '/';
            if (!mdlPath.empty() && mdlPath.find(".mdl") != std::string::npos)
                return mdlPath;
        }
    }

    // 方法 2: studio helper 带参数（老写法）
    int helperCount = pClass->GetHelperCount();
    for (int i = 0; i < helperCount; ++i) {
        CHelperInfo* pHelper = pClass->GetHelper(i);
        if (!pHelper || !pHelper->GetName()) continue;
        if (strcasecmp(pHelper->GetName(), "studio") != 0) continue;

        int paramCount = pHelper->GetParameterCount();
        if (paramCount > 0 && pHelper->GetParameter(0)) {
            std::string mdlPath = pHelper->GetParameter(0);
            if (mdlPath.size() >= 2 && mdlPath.front() == '"' && mdlPath.back() == '"')
                mdlPath = mdlPath.substr(1, mdlPath.size() - 2);
            for (auto& c : mdlPath) if (c == '\\') c = '/';
            if (!mdlPath.empty() && mdlPath.find(".mdl") != std::string::npos)
                return mdlPath;
        }
    }

    // 方法 3: 递归 base classes
    int baseCount = pClass->GetBaseCount();
    for (int i = 0; i < baseCount; ++i) {
        GDclass* pBase = pClass->GetBase(i);
        std::string r = FindModelInClassRecursive(pBase, depth + 1);
        if (!r.empty()) return r;
    }

    return "";
}

std::string FgdManager::GetModelPathForClass(const std::string& classname) const {
    if (!m_pGameData) {
        printf("[FgdManager] ERROR: m_pGameData is NULL!\n");
        fflush(stdout);
        return "";
    }

    GDclass* pClass = m_pGameData->ClassForName(classname.c_str());
    if (!pClass) {
        printf("[FgdManager] WARNING: Class not found in FGD: %s\n", classname.c_str());
        fflush(stdout);
        return "";
    }

    // 递归查 model 变量（当前 class + base classes）
    std::string result = FindModelInClassRecursive(pClass, 0);

    if (result.empty()) {
        printf("[FgdManager] No model for class '%s'\n", classname.c_str());
    } else {
        printf("[FgdManager] Model for '%s' -> '%s'\n",
               classname.c_str(), result.c_str());
    }
    fflush(stdout);
    return result;
}