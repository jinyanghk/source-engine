#include <cstdio>
#include <vector>

#include "ui/EditorPanels.h"
#include "render/MaterialList.h"
#include "app/CompileSettings.h"
#include "render/MaterialPreview.h"

#include "imgui.h"
#include "app/EditorLayout.h"
#include "scene/SceneGlobals.h"
#include "app/FgdManager.h"
#include "fgdlib/gdvar.h"   // GDIV_TYPE 枚举
#include "ui/EntityPalette.h"
#include "app/LogBuffer.h"
#include "app/UndoStack.h"

// 文件顶部
static void (*s_pfnExportVMF)() = nullptr;
static void (*s_pfnCompileAndRun)() = nullptr;

void SetExportVMFHandler(void (*pfn)())
{
    s_pfnExportVMF = pfn;
}

void SetCompileAndRunHandler(void (*pfn)())
{
    s_pfnCompileAndRun = pfn;
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
            if (ImGui::MenuItem("Compile & Run"))
            {
                if (s_pfnCompileAndRun) s_pfnCompileAndRun();
            }
            if (ImGui::MenuItem("Compile Options..."))
            {
                OpenCompileOptionsWindow();
            }                  
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) { /* TODO */ }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            std::string undoLabel = UndoStack::Instance().GetUndoLabel();
            std::string redoLabel = UndoStack::Instance().GetRedoLabel();

            char undoBuf[128];
            if (!undoLabel.empty())
                snprintf(undoBuf, sizeof(undoBuf), "Undo %s", undoLabel.c_str());
            else
                snprintf(undoBuf, sizeof(undoBuf), "Undo");

            char redoBuf[128];
            if (!redoLabel.empty())
                snprintf(redoBuf, sizeof(redoBuf), "Redo %s", redoLabel.c_str());
            else
                snprintf(redoBuf, sizeof(redoBuf), "Redo");

            if (ImGui::MenuItem(undoBuf, "Ctrl+Z", false, UndoStack::Instance().CanUndo()))
            {
                UndoStack::Instance().Undo();
            }
            if (ImGui::MenuItem(redoBuf, "Ctrl+Y", false, UndoStack::Instance().CanRedo()))
            {
                UndoStack::Instance().Redo();
            }
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
// Entity 面板：list + 属性 + 折叠的 palette
//-----------------------------------------------------------------------------
void DrawEntityPanel(CEntity* entities, int entityCount, int* pSelectedEntity,
                     int* pSelectedBrush,
                     BrushPanelResult& outResult)
{
    if (!ImGui::Begin("Entity"))
    {
        ImGui::End();
        return;
    }

    // --- 1. Entity list ---
    ImGui::Text("Entities:");
    for (int i = 0; i < entityCount; i++)
    {
        ImGui::PushID(i);

        bool bSel = (i == *pSelectedEntity);
        if (ImGui::Selectable(entities[i].m_szName.c_str(), bSel, 0,
                              ImVec2(ImGui::GetContentRegionAvail().x - 60.0f, 0)))
        {
            *pSelectedEntity = i;
            *pSelectedBrush  = -1;   // 互斥
        }

        ImGui::SameLine();
        if (ImGui::SmallButton("Del"))
        {
            outResult.bRequestDeleteEntity = true;
            outResult.iTargetEntityIndex = i;
        }

        ImGui::PopID();
    }

    // --- 2. Selected entity 属性 ---
    ImGui::Separator();
    if (*pSelectedEntity >= 0 && *pSelectedEntity < entityCount)
    {
        CEntity& sel = entities[*pSelectedEntity];
        ImGui::Text("Selected: %s", sel.m_szName.c_str());

        ImGui::SliderFloat("Pos X", &sel.m_vecPos.x, -512.0f, 512.0f, "%.2f");
        ImGui::SliderFloat("Pos Y", &sel.m_vecPos.y, -512.0f, 512.0f, "%.2f");
        ImGui::SliderFloat("Pos Z", &sel.m_vecPos.z, -128.0f, 256.0f, "%.2f");

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

        // Properties（FGD keyvalue）
        ImGui::Separator();
        ImGui::Text("Properties:");

        std::vector<FgdKeyValue> kvList;
        if (FgdManager::Instance().GetKeyValuesForClass(sel.m_szName, kvList))
        {
            for (const auto& kv : kvList)
            {
                ImGui::PushID(kv.name.c_str());

                std::string curValue;
                auto it = sel.m_properties.find(kv.name);
                if (it != sel.m_properties.end())
                    curValue = it->second;
                else
                    curValue = kv.defaultValue;

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

    // --- 3. Entity Palette（折叠） ---
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Entity Palette"))
    {
        ImGui::BeginChild("##PaletteChild", ImVec2(0, 250), true);

        std::string newClass;
        DrawEntityPaletteContent(newClass);
        if (!newClass.empty())
        {
            outResult.bRequestNewEntity = true;
            outResult.szNewEntityClass = newClass;
        }

        ImGui::EndChild();
    }

    ImGui::End();
}

//-----------------------------------------------------------------------------
// Brush 面板：list + 属性
//-----------------------------------------------------------------------------
void DrawBrushPanel(const CBrush* brushes, int brushCount, int* pSelectedBrush,
                    int* pSelectedEntity,
                    BrushPanelResult& outResult)
{
    if (!ImGui::Begin("Brush"))
    {
        ImGui::End();
        return;
    }

    // --- 1. 工具栏：只保留 + New ---
    if (ImGui::Button("+ New"))
    {
        outResult.bRequestNew = true;
    }

    ImGui::Separator();

    // --- 2. Brush list（每项带 Dup / Del） ---
    ImGui::Text("Brushes (%d):", brushCount);
    for (int i = 0; i < brushCount; i++)
    {
        ImGui::PushID(i);

        char label[64];
        snprintf(label, sizeof(label), "brush #%d", i);
        bool bSel = (i == *pSelectedBrush);
        if (ImGui::Selectable(label, bSel, 0,
                              ImVec2(ImGui::GetContentRegionAvail().x - 100.0f, 0)))
        {
            *pSelectedBrush  = i;
            *pSelectedEntity = -1;   // 互斥
        }

        ImGui::SameLine();
        if (ImGui::SmallButton("Dup"))
        {
            outResult.bRequestDuplicate = true;
            outResult.iTargetIndex = i;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Del"))
        {
            outResult.bRequestDelete = true;
            outResult.iTargetIndex = i;
        }

        ImGui::PopID();
    }

    // --- 3. Selected brush 属性 ---
    ImGui::Separator();
    if (*pSelectedBrush >= 0 && *pSelectedBrush < brushCount)
    {
        CBrush& b = const_cast<CBrush&>(brushes[*pSelectedBrush]);

        ImGui::Text("Brush #%d", *pSelectedBrush);

        ImGui::Text("Center");
        Vector oldPos = b.m_vecPos;
        bool posChanged = false;
        posChanged |= ImGui::SliderFloat("CX", &b.m_vecPos.x, -512.0f, 512.0f, "%.1f");
        posChanged |= ImGui::SliderFloat("CY", &b.m_vecPos.y, -512.0f, 512.0f, "%.1f");
        posChanged |= ImGui::SliderFloat("CZ", &b.m_vecPos.z, -128.0f, 256.0f, "%.1f");

        ImGui::Text("Half-size");
        Vector oldSize = b.m_vecSize;
        bool sizeChanged = false;
        sizeChanged |= ImGui::SliderFloat("SX", &b.m_vecSize.x, 1.0f, 512.0f, "%.1f");
        sizeChanged |= ImGui::SliderFloat("SY", &b.m_vecSize.y, 1.0f, 512.0f, "%.1f");
        sizeChanged |= ImGui::SliderFloat("SZ", &b.m_vecSize.z, 1.0f, 512.0f, "%.1f");

        // ★ 关键：如果 pos/size 变了，把 m_vertices 也重建
        if (posChanged || sizeChanged)
        {
            Vector delta = b.m_vecPos - oldPos;
            Vector scale;
            scale.x = (oldSize.x > 0.0001f) ? (b.m_vecSize.x / oldSize.x) : 1.0f;
            scale.y = (oldSize.y > 0.0001f) ? (b.m_vecSize.y / oldSize.y) : 1.0f;
            scale.z = (oldSize.z > 0.0001f) ? (b.m_vecSize.z / oldSize.z) : 1.0f;

            for (auto& v : b.m_vertices)
            {
                Vector local = v - oldPos;
                local.x *= scale.x;
                local.y *= scale.y;
                local.z *= scale.z;
                v = oldPos + local + delta;
            }

            b.RecalcFaceNormals();
        }

        ImGui::Text("Material:");
        ImGui::TextWrapped("%s", b.m_szMaterial.c_str());

        static char s_materialFilter[64] = "";
        ImGui::InputText("Filter##mat", s_materialFilter, sizeof(s_materialFilter));

        if (ImGui::BeginCombo("##MaterialCombo", "Select..."))
        {
            auto filtered = MaterialList::Instance().Filter(s_materialFilter, 200);

            ImGuiListClipper clipper;
            clipper.Begin((int)filtered.size(), ImGui::GetTextLineHeightWithSpacing() + 32);
            while (clipper.Step())
            {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                {
                    const auto& mat = filtered[i];
                    ImGui::PushID(i);

                    GLuint tex = MaterialPreview::Instance().GetPreviewTexture(mat);
                    if (tex) {
                        ImGui::Image((ImTextureID)(intptr_t)tex, ImVec2(32, 32));
                        ImGui::SameLine();
                    } else {
                        ImGui::Dummy(ImVec2(32, 32));
                        ImGui::SameLine();
                    }

                    bool bSelected = (mat == b.m_szMaterial);
                    if (ImGui::Selectable(mat.c_str(), bSelected, 0, ImVec2(0, 32)))
                        b.m_szMaterial = mat;
                    if (bSelected) ImGui::SetItemDefaultFocus();

                    ImGui::PopID();
                }
            }
            clipper.End();

            ImGui::EndCombo();
        }

        ImGui::Separator();
        if (ImGui::Button("Deselect"))
            *pSelectedBrush = -1;
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

    // --- 工具栏 ---
    if (ImGui::Button("Clear"))
        LogBuffer::Instance().Clear();
    ImGui::SameLine();
    ImGui::TextDisabled("(%d lines)", (int)LogBuffer::Instance().GetLines().size());

    // 保留相机提示作为快速参考（也可以删掉）
    if (ImGui::CollapsingHeader("Controls"))
    {
        ImGui::Text("Camera: LMB empty = orbit | RMB/MMB = pan | Wheel = zoom");
        ImGui::Text("        W/A/S/D = fly | Q/E = down/up | Z / Shift+Z = zoom");
        ImGui::Text("        [R] = Reset Camera");
        ImGui::Text("Gizmo:  1 = Translate | 2 = Rotate | 3 = Scale | 4 = Face | 5 = Vertex | 0 = None");
        ImGui::Text("Cam Target: (%.1f, %.1f, %.1f)", camTarget.x, camTarget.y, camTarget.z);
        ImGui::Text("Cam Dist: %.1f", camDist);
    }

    ImGui::Separator();

    // --- 日志区域 ---
    ImGui::BeginChild("##LogScroll", ImVec2(0, 0), false,
                     ImGuiWindowFlags_HorizontalScrollbar);

    const auto& lines = LogBuffer::Instance().GetLines();
    for (const auto& line : lines)
    {
        // 根据内容着色
        ImVec4 color(0.85f, 0.85f, 0.85f, 1.0f);   // 默认灰白

        if (line.find("FAILED") != std::string::npos ||
            line.find("ERROR") != std::string::npos ||
            line.find("Cannot") != std::string::npos ||
            line.find("cannot") != std::string::npos)
        {
            color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);   // 红
        }
        else if (line.find("WARNING") != std::string::npos ||
                 line.find("WARN") != std::string::npos)
        {
            color = ImVec4(1.0f, 0.8f, 0.3f, 1.0f);   // 黄
        }
        else if (line.find("exit code = 0") != std::string::npos ||
                 line.find("OK") != std::string::npos)
        {
            color = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);   // 绿
        }
        else if (line.find("$ ") != std::string::npos)
        {
            color = ImVec4(0.5f, 0.7f, 1.0f, 1.0f);   // 蓝（命令行）
        }

        ImGui::TextColored(color, "%s", line.c_str());
    }

    // 有新日志时自动滚到底部
    if (LogBuffer::Instance().IsDirty())
    {
        ImGui::SetScrollHereY(1.0f);
        LogBuffer::Instance().SetClean();
    }

    ImGui::EndChild();
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

//-----------------------------------------------------------------------------
// 编译配置窗口
//-----------------------------------------------------------------------------
static bool s_bCompileOptionsOpen = false;

void OpenCompileOptionsWindow()
{
    s_bCompileOptionsOpen = true;
}

void DrawCompileOptionsWindow()
{
    if (!s_bCompileOptionsOpen) return;

    ImGui::SetNextWindowSize(ImVec2(480, 360), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Compile Options", &s_bCompileOptionsOpen))
    {
        ImGui::InputText("Map Name", g_compileSettings.mapName, sizeof(g_compileSettings.mapName));
        ImGui::InputText("Game Dir", g_compileSettings.gameDir, sizeof(g_compileSettings.gameDir));

        ImGui::Separator();

        ImGui::Checkbox("Run VBSP", &g_compileSettings.bRunVbsp);
        ImGui::Checkbox("Run VVIS", &g_compileSettings.bRunVvis);
        ImGui::Checkbox("Run VRAD", &g_compileSettings.bRunVrad);
        ImGui::Checkbox("Copy BSP to maps/", &g_compileSettings.bCopyToMaps);
        ImGui::Checkbox("Launch game", &g_compileSettings.bLaunchGame);

        ImGui::Separator();
        ImGui::Text("Extra Args:");

        ImGui::InputText("VBSP args", g_compileSettings.vbspArgs, sizeof(g_compileSettings.vbspArgs));
        ImGui::InputText("VVIS args", g_compileSettings.vvisArgs, sizeof(g_compileSettings.vvisArgs));
        ImGui::InputText("VRAD args", g_compileSettings.vradArgs, sizeof(g_compileSettings.vradArgs));

        ImGui::Separator();

        if (ImGui::Button("Compile & Run"))
        {
            if (s_pfnCompileAndRun) s_pfnCompileAndRun();
        }
        ImGui::SameLine();
        if (ImGui::Button("Close"))
            s_bCompileOptionsOpen = false;
    }
    ImGui::End();
}