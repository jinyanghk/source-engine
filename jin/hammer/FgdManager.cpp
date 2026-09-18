#include "FgdManager.h"

#include <QDebug>

#include "fgdlib/ieditortexture.h" 
#include "fgdlib/gamedata.h"
#include "fgdlib/gdclass.h"

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
    // Invoke your static lib parser loop mechanism
    return m_pGameData->Load(filePath.c_str()) == TRUE;
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

    // Extract bound boxes bounds safely if the file defined explicit @size tags
    if (pClass->HasBoundBox()) {
        Vector mins = pClass->GetMins();
        Vector maxs = pClass->GetMaxs();
        outTemplate.mins[0] = mins.x; outTemplate.mins[1] = mins.y; outTemplate.mins[2] = mins.z;
        outTemplate.maxs[0] = maxs.x; outTemplate.maxs[1] = maxs.y; outTemplate.maxs[2] = maxs.z;
    }

    // Map color32 specs down to raw display unsigned chars
    color32 fgdColor = pClass->GetColor();
    outTemplate.r = fgdColor.r;
    outTemplate.g = fgdColor.g;
    outTemplate.b = fgdColor.b;

    return true;
}

std::string FgdManager::GetModelPathForClass(const std::string& classname) const {
    if (!m_pGameData) {
        qDebug() << "[FgdManager] ERROR: m_pGameData is NULL!";
        return "models/alyx.mdl";
    }

    GDclass* pClass = m_pGameData->ClassForName(classname.c_str());
    if (!pClass) {
        qDebug() << "[FgdManager] WARNING: Class not found in FGD:" << QString::fromStdString(classname);
        return "models/alyx.mdl";
    }

    int helperCount = pClass->GetHelperCount();
    qDebug() << "[FgdManager] Querying class:" << QString::fromStdString(classname) << "| Total Helpers found:" << helperCount;

    for (int i = 0; i < helperCount; ++i) {
        CHelperInfo* pHelper = pClass->GetHelper(i);
        if (pHelper && pHelper->GetName()) {
            QString helperName = QString::fromUtf8(pHelper->GetName());
            int paramCount = pHelper->GetParameterCount();
            
            qDebug() << "  -> Helper index" << i << ":" << helperName << "| Parameter count:" << paramCount;

            for (int p = 0; p < paramCount; ++p) {
                if (pHelper->GetParameter(p)) {
                    qDebug() << "     [" << p << "]:" << QString::fromUtf8(pHelper->GetParameter(p));
                }
            }

            if (helperName.compare("studio", Qt::CaseInsensitive) == 0) {
                if (paramCount > 0 && pHelper->GetParameter(0)) {
                    std::string mdlPath = pHelper->GetParameter(0);
                    
                    if (mdlPath.size() >= 2 && mdlPath.front() == '"' && mdlPath.back() == '"') {
                        mdlPath = mdlPath.substr(1, mdlPath.size() - 2);
                    }
                    
                    qDebug() << "  >> SUCCESS: Matched studio helper! Returning path:" << QString::fromStdString(mdlPath);
                    return mdlPath;
                }
            }
        }
    }

    qDebug() << "  >> No studio helper matched for this class. Falling back to alyx.mdl.";
    return "models/alyx.mdl";
}