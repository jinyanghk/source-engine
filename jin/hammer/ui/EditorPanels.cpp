#include "ui/EditorPanels.h"

#include "imgui.h"
#include "app/EditorLayout.h"
#include "scene/SceneGlobals.h"

//-----------------------------------------------------------------------------
void DrawMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit")) { /* TODO */ }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            ImGui::MenuItem("Undo", "Ctrl+Z", false, false);
            ImGui::MenuItem("Redo", "Ctrl+Y", false, false);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Reset Layout"))
                EditorLayout::ResetDefaultLayout();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

//-----------------------------------------------------------------------------
void DrawEntityPanel(CEntity* entities, int entityCount, int* pSelectedIndex)
{
    if (!ImGui::Begin("Entities"))
    {
        ImGui::End();
        return;
    }

    ImGui::Text("Entities:");
    for (int i = 0; i < entityCount; i++)
    {
        bool bSel = (i == *pSelectedIndex);
        if (ImGui::Selectable(entities[i].m_szName, bSel))
            *pSelectedIndex = i;
    }

    ImGui::Separator();

    if (*pSelectedIndex >= 0 && *pSelectedIndex < entityCount)
    {
        CEntity& sel = entities[*pSelectedIndex];
        ImGui::Text("Selected: %s", sel.m_szName);
        ImGui::Separator();
        ImGui::SliderFloat("Pos X", &sel.m_vecPos.x, -256.0f, 256.0f, "%.2f");
        ImGui::SliderFloat("Pos Y", &sel.m_vecPos.y, -256.0f, 256.0f, "%.2f");
        ImGui::SliderFloat("Pos Z", &sel.m_vecPos.z, -50.0f, 200.0f, "%.2f");
        ImGui::Separator();

        if (sel.m_iType == ENTITY_PLAYER_START)
        {
            ImGui::SliderFloat("Yaw", &sel.m_angRot.y, -180.0f, 180.0f, "%.1f");
        }
        else
        {
            ImGui::SliderFloat("Pitch", &sel.m_angRot.x, -180.0f, 180.0f, "%.1f");
            ImGui::SliderFloat("Yaw",   &sel.m_angRot.y, -180.0f, 180.0f, "%.1f");
            ImGui::SliderFloat("Roll",  &sel.m_angRot.z, -180.0f, 180.0f, "%.1f");
        }
    }

    ImGui::End();
}

//-----------------------------------------------------------------------------
void DrawConsolePanel(const Vector& camTarget, float camDist,
                      bool bShowGizmo, bool bXRayGizmo)
{
    if (!ImGui::Begin("Console"))
    {
        ImGui::End();
        return;
    }

    ImGui::Text("Camera: LMB empty = orbit | RMB/MMB = pan | Wheel = zoom");
    ImGui::Text("        W/A/S/D = fly | Q/E = down/up | Z / Shift+Z = zoom");
    ImGui::Text("        [R] = Reset Camera");
    ImGui::Text("Gizmo:  1 = Translate | 2 = Rotate | 3 = None");
    ImGui::Separator();
    ImGui::Text("Cam Target: (%.1f, %.1f, %.1f)", camTarget.x, camTarget.y, camTarget.z);
    ImGui::Text("Cam Dist: %.1f", camDist);
    ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Press R to reset camera!");

    ImGui::End();
}