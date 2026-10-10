#include "ui/EntityPalette.h"

#include <vector>
#include <cstring>
#include "imgui.h"
#include "app/FgdManager.h"

void DrawEntityPalette(std::string& outSelectedClass)
{
    outSelectedClass.clear();

    if (!ImGui::Begin("Entity Palette"))
    {
        ImGui::End();
        return;
    }

    // Filter
    static char filter[128] = "";
    ImGui::InputText("Filter", filter, sizeof(filter));
    ImGui::Separator();

    // 一次性拉取 FGD 里的所有 point classes。
    // 这里假定 FGD 在程序启动时加载、之后不变，所以只拉一次。
    static std::vector<std::string> s_classes;
    static bool s_loaded = false;
    if (!s_loaded)
    {
        s_loaded = true;
        s_classes = FgdManager::Instance().GetAvailablePointClasses();
    }

    // 分组显示：按 class 名的第一个 `_` 前缀分组。
    // 简单起见，这里用 flat list + 过滤。
    std::string filterStr = filter;
    for (const auto& cls : s_classes)
    {
        if (!filterStr.empty())
        {
            // 大小写不敏感子串匹配
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

    ImGui::End();
}