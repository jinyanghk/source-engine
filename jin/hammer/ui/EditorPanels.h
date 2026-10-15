#ifndef EDITOR_PANELS_H
#define EDITOR_PANELS_H

#include <cstdio>
#include <string>

#include "mathlib/vector.h"

#include "scene/Entity.h"
#include "scene/Brush.h"
#include "app/EditorState.h"

//-----------------------------------------------------------------------------
// DrawSelectionPanel 的输出：告诉调用者用户想做什么操作。
//-----------------------------------------------------------------------------
struct BrushPanelResult
{
    bool bRequestNew = false;
    bool bRequestDelete = false;
    bool bRequestDuplicate = false;
    int  iTargetIndex = -1;
    bool bRequestDeleteEntity = false;
    int  iTargetEntityIndex = -1;

    // ★ 新增
    bool bRequestNewEntity = false;
    std::string szNewEntityClass;
};

//-----------------------------------------------------------------------------
// 顶部菜单栏。
//-----------------------------------------------------------------------------
void DrawMenuBar();

void SetExportVMFHandler(void (*pfn)());
void SetCompileAndRunHandler(void (*pfn)());
// 编译配置面板（Compile Options 窗口 + 菜单项）
void DrawCompileOptionsWindow();
void OpenCompileOptionsWindow();

// 替代原来的 DrawSelectionPanel
void DrawEntityPanel(CEntity* entities, int entityCount, int* pSelectedEntity,
                     int* pSelectedBrush,
                     BrushPanelResult& outResult);

void DrawBrushPanel(const CBrush* brushes, int brushCount, int* pSelectedBrush,
                    int* pSelectedEntity,
                    BrushPanelResult& outResult);

void DrawToolsPanel();

//-----------------------------------------------------------------------------
// 底部控制台面板（操作提示 + 相机信息）。
//-----------------------------------------------------------------------------
void DrawConsolePanel(const Vector& camTarget, float camDist,
                      bool bShowGizmo, bool bXRayGizmo);

void DrawViewportPanel();

// 独立窗口版本（保留兼容，但 main.cpp 不再调用）
void DrawEntityPalette(std::string& outSelectedClass);

// ★ 新增：只画内容，不画窗口。调用方负责 Begin/End
void DrawEntityPaletteContent(std::string& outSelectedClass);

#endif // EDITOR_PANELS_H