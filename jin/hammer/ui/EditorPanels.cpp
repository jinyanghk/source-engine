#include <cstdio>

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
void DrawSelectionPanel(CEntity* entities, int entityCount, int* pSelectedEntity,
                        const CBrush* brushes, int brushCount, int* pSelectedBrush,
                        BrushPanelResult& outResult)
{
    outResult = BrushPanelResult();

    if (!ImGui::Begin("Selection"))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("SelectionTabs"))
    {
        if (ImGui::BeginTabItem("Entities"))
        {
            ImGui::Text("Entities:");
            for (int i = 0; i < entityCount; i++)
            {
                ImGui::PushID(i);

                bool bSel = (i == *pSelectedEntity);
                if (ImGui::Selectable(entities[i].m_szName.c_str(), bSel, 0,
                                    ImVec2(ImGui::GetContentRegionAvail().x - 60.0f, 0)))
                {
                    *pSelectedEntity = i;
                    *pSelectedBrush  = -1;   // ★ 互斥
                }

                ImGui::SameLine();
                if (ImGui::SmallButton("Del"))
                {
                    outResult.bRequestDeleteEntity = true;
                    outResult.iTargetEntityIndex = i;
                }

                ImGui::PopID();
            }

            ImGui::Separator();
            if (*pSelectedEntity >= 0 && *pSelectedEntity < entityCount)
            {
                CEntity& sel = entities[*pSelectedEntity];
                ImGui::Text("Selected: %s", sel.m_szName.c_str());
                ImGui::Separator();
                ImGui::SliderFloat("Pos X", &sel.m_vecPos.x, -512.0f, 512.0f, "%.2f");
                ImGui::SliderFloat("Pos Y", &sel.m_vecPos.y, -512.0f, 512.0f, "%.2f");
                ImGui::SliderFloat("Pos Z", &sel.m_vecPos.z, -128.0f, 256.0f, "%.2f");
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
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Brushes"))
        {
            ImGui::Text("Brushes (%d):", brushCount);

            // 工具栏：新建 / 复制 / 删除
            if (ImGui::Button("+ New"))
            {
                outResult.bRequestNew = true;
            }
            ImGui::SameLine();
            bool bHasSelection = (*pSelectedBrush >= 0 && *pSelectedBrush < brushCount);
            if (!bHasSelection) ImGui::BeginDisabled();
            if (ImGui::Button("Duplicate"))
            {
                outResult.bRequestDuplicate = true;
                outResult.iTargetIndex = *pSelectedBrush;
            }
            ImGui::SameLine();
            if (ImGui::Button("Delete"))
            {
                outResult.bRequestDelete = true;
                outResult.iTargetIndex = *pSelectedBrush;
            }
            if (!bHasSelection) ImGui::EndDisabled();

            ImGui::Separator();

            // 列表
            for (int i = 0; i < brushCount; i++)
            {
                char label[64];
                snprintf(label, sizeof(label), "brush #%d", i);
                bool bSel = (i == *pSelectedBrush);
                if (ImGui::Selectable(label, bSel))
                {
                    *pSelectedBrush  = i;
                    *pSelectedEntity = -1;   // ★ 互斥
                }
            }

            ImGui::Separator();

            if (*pSelectedBrush >= 0 && *pSelectedBrush < brushCount)
            {
                // 注意：brushes 是 const，不能直接改。属性编辑通过 const_cast 或
                // 让 main.cpp 也拿指针。这里为了简单，直接用 const_cast —— 因为
                // 面板本身的职责就是"编辑"，传 const 只是为了让接口明确"面板不增删"。
                CBrush& b = const_cast<CBrush&>(brushes[*pSelectedBrush]);

                ImGui::Text("Brush #%d", *pSelectedBrush);
                ImGui::Separator();

                ImGui::Text("Center");
                ImGui::SliderFloat("CX", &b.m_vecPos.x, -512.0f, 512.0f, "%.1f");
                ImGui::SliderFloat("CY", &b.m_vecPos.y, -512.0f, 512.0f, "%.1f");
                ImGui::SliderFloat("CZ", &b.m_vecPos.z, -128.0f, 256.0f, "%.1f");

                ImGui::Text("Half-size");
                ImGui::SliderFloat("SX", &b.m_vecSize.x, 1.0f, 512.0f, "%.1f");
                ImGui::SliderFloat("SY", &b.m_vecSize.y, 1.0f, 512.0f, "%.1f");
                ImGui::SliderFloat("SZ", &b.m_vecSize.z, 1.0f, 512.0f, "%.1f");

                ImGui::Text("Texture");
                int texId = b.m_iTexId;
                const char* texNames[] = {"Checker", "Brick", "Floor"};
                if (ImGui::Combo("TexId", &texId, texNames, 3))
                    b.m_iTexId = texId;

                ImGui::Separator();
                if (ImGui::Button("Deselect"))
                    *pSelectedBrush = -1;
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
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

//-----------------------------------------------------------------------------
void DrawViewportPanel()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar
                           | ImGuiWindowFlags_NoCollapse
                           | ImGuiWindowFlags_NoResize
                           | ImGuiWindowFlags_NoMove
                           | ImGuiWindowFlags_NoBringToFrontOnFocus
                           | ImGuiWindowFlags_NoNavFocus
                           | ImGuiWindowFlags_NoBackground
                           | ImGuiWindowFlags_NoScrollbar
                           | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr, flags);
    // 什么都不画。PassthruCentralNode 让 3D 透出来。
    ImGui::End();
    ImGui::PopStyleVar();
}