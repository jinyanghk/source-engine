#include <cstdio>

#include "ui/EditorPanels.h"

#include "imgui.h"
#include "app/EditorLayout.h"
#include "scene/SceneGlobals.h"
#include "app/FgdManager.h"
#include "fgdlib/gdvar.h"   // GDIV_TYPE 枚举

// 文件顶部
static void (*s_pfnExportVMF)() = nullptr;

void SetExportVMFHandler(void (*pfn)())
{
    s_pfnExportVMF = pfn;
}

//-----------------------------------------------------------------------------
void DrawMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Export VMF"))
            {
                if (s_pfnExportVMF) s_pfnExportVMF();
            }
            ImGui::Separator();
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

                // ★★★ Properties 区（在 if 块内，sel 有效）★★★
                ImGui::Separator();
                ImGui::Text("Properties:");

                std::vector<FgdKeyValue> kvList;
                if (FgdManager::Instance().GetKeyValuesForClass(sel.m_szName, kvList))
                {
                    for (const auto& kv : kvList)
                    {
                        ImGui::PushID(kv.name.c_str());

                        // 当前值：优先用 m_properties，否则用默认值
                        std::string curValue;
                        auto it = sel.m_properties.find(kv.name);
                        if (it != sel.m_properties.end())
                            curValue = it->second;
                        else
                            curValue = kv.defaultValue;

                        // 显示名（如果 FGD 没给，用 key 名）
                        std::string label = kv.displayName.empty() ? kv.name : kv.displayName;

                        switch ((GDIV_TYPE)kv.type)
                        {
                        case ivString:
                        case ivTargetDest:
                        case ivTargetNameOrClass:
                        case ivTargetSrc:
                        case ivStudioModel:
                        case ivSprite:
                        case ivSound:
                        case ivMaterial:
                        case ivDecal:
                        case ivScene:
                        case ivInstanceFile:
                        case ivPointEntityClass:
                        case ivNodeDest:
                        {
                            char buf[512];
                            strncpy(buf, curValue.c_str(), sizeof(buf) - 1);
                            buf[sizeof(buf) - 1] = '\0';
                            if (ImGui::InputText(label.c_str(), buf, sizeof(buf)))
                                sel.m_properties[kv.name] = buf;
                            break;
                        }
                        case ivInteger:
                        {
                            int v = atoi(curValue.c_str());
                            if (ImGui::InputInt(label.c_str(), &v))
                                sel.m_properties[kv.name] = std::to_string(v);
                            break;
                        }
                        case ivFloat:
                        case ivAngle:
                        case ivAngleNegativePitch:
                        {
                            float v = (float)atof(curValue.c_str());
                            if (ImGui::InputFloat(label.c_str(), &v, 1.0f, 10.0f, "%.2f"))
                            {
                                char buf[64];
                                snprintf(buf, sizeof(buf), "%.2f", v);
                                sel.m_properties[kv.name] = buf;
                            }
                            break;
                        }
                        case ivChoices:
                        {
                            int curIdx = 0;
                            for (size_t ci = 0; ci < kv.choices.size(); ++ci)
                            {
                                if (kv.choices[ci].value == curValue) { curIdx = (int)ci; break; }
                            }
                            std::vector<const char*> captions;
                            for (const auto& ch : kv.choices) captions.push_back(ch.caption.c_str());

                            if (!captions.empty())
                            {
                                if (ImGui::Combo(label.c_str(), &curIdx, captions.data(), (int)captions.size()))
                                    sel.m_properties[kv.name] = kv.choices[curIdx].value;
                            }
                            break;
                        }
                        case ivFlags:
                        {
                            unsigned long curFlags = (unsigned long)strtoul(curValue.c_str(), nullptr, 10);
                            unsigned long newFlags = curFlags;
                            bool changed = false;
                            for (size_t fi = 0; fi < kv.choices.size(); ++fi)
                            {
                                bool bSet = (curFlags & kv.flagMasks[fi]) != 0;
                                if (ImGui::Checkbox(kv.choices[fi].caption.c_str(), &bSet))
                                {
                                    if (bSet) newFlags |= kv.flagMasks[fi];
                                    else      newFlags &= ~kv.flagMasks[fi];
                                    changed = true;
                                }
                            }
                            if (changed)
                                sel.m_properties[kv.name] = std::to_string(newFlags);
                            break;
                        }
                        case ivColor255:
                        case ivColor1:
                        {
                            int r = 255, g = 255, b = 255;
                            sscanf(curValue.c_str(), "%d %d %d", &r, &g, &b);
                            float col[3] = { r / 255.0f, g / 255.0f, b / 255.0f };
                            if (ImGui::ColorEdit3(label.c_str(), col))
                            {
                                char buf[64];
                                snprintf(buf, sizeof(buf), "%d %d %d",
                                        (int)(col[0] * 255), (int)(col[1] * 255), (int)(col[2] * 255));
                                sel.m_properties[kv.name] = buf;
                            }
                            break;
                        }
                        case ivVector:
                        case ivOrigin:
                        case ivVecLine:
                        {
                            float x = 0, y = 0, z = 0;
                            sscanf(curValue.c_str(), "%f %f %f", &x, &y, &z);
                            float vec[3] = { x, y, z };
                            if (ImGui::InputFloat3(label.c_str(), vec, "%.2f"))
                            {
                                char buf[128];
                                snprintf(buf, sizeof(buf), "%.2f %.2f %.2f", vec[0], vec[1], vec[2]);
                                sel.m_properties[kv.name] = buf;
                            }
                            break;
                        }
                        default:
                        {
                            char buf[512];
                            strncpy(buf, curValue.c_str(), sizeof(buf) - 1);
                            buf[sizeof(buf) - 1] = '\0';
                            if (ImGui::InputText(label.c_str(), buf, sizeof(buf)))
                                sel.m_properties[kv.name] = buf;
                            break;
                        }
                        }

                        // 鼠标悬停显示描述
                        if (!kv.description.empty() && ImGui::IsItemHovered())
                            ImGui::SetTooltip("%s", kv.description.c_str());

                        ImGui::PopID();
                    }
                }
                else
                {
                    ImGui::TextDisabled("(no FGD data)");
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