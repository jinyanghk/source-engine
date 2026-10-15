#include "ui/EntityPalette.h"

#include <vector>
#include <cstring>
#include <cctype>
#include "imgui.h"
#include "app/FgdManager.h"

//-----------------------------------------------------------------------------
// 只画内容，不画窗口。调用方负责 Begin/End
//-----------------------------------------------------------------------------
void DrawEntityPaletteContent(std::string& outSelectedClass)
{
    outSelectedClass.clear();

    // Filter
    static char filter[128] = "";
    ImGui::InputText("##Filter", filter, sizeof(filter));
    ImGui::Separator();

    // 一次性拉取 FGD 里的所有 point classes
    static std::vector<std::string> s_classes;
    static bool s_loaded = false;
    if (!s_loaded)
    {
        s_loaded = true;
        s_classes = FgdManager::Instance().GetAvailablePointClasses();
    }

    std::string filterStr = filter;
    for (const auto& cls : s_classes)
    {
        if (!filterStr.empty())
        {
            std::string lowerCls = cls;
            std::string lowerFilter = filterStr;
            for (auto& c : lowerCls) c = (char)tolower((unsigned char)c);
            for (auto& c : lowerFilter) c = (char)tolower((unsigned char)c);
            if (lowerCls.find(lowerFilter) == std::string::npos)
                continue;
        }

        if (ImGui::Selectable(cls.c_str()))
        {
            outSelectedClass = cls;
        }
    }
}

//-----------------------------------------------------------------------------
// 独立窗口版本（保留兼容）
//-----------------------------------------------------------------------------
void DrawEntityPalette(std::string& outSelectedClass)
{
    if (!ImGui::Begin("Entity Palette"))
    {
        ImGui::End();
        return;
    }

    DrawEntityPaletteContent(outSelectedClass);

    ImGui::End();
}