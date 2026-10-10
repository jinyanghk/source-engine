#ifndef EDITOR_PANELS_H
#define EDITOR_PANELS_H

#include <cstdio>

#include "scene/Entity.h"
#include "scene/Brush.h"

#include "mathlib/vector.h"

//-----------------------------------------------------------------------------
// DrawSelectionPanel 的输出：告诉调用者用户想做什么操作。
//-----------------------------------------------------------------------------
struct BrushPanelResult
{
    bool bRequestNew = false;
    bool bRequestDelete = false;
    bool bRequestDuplicate = false;
    int  iTargetIndex = -1;
    // ★ 新增：entity 删除请求
    bool bRequestDeleteEntity = false;
    int  iTargetEntityIndex = -1;    
};

//-----------------------------------------------------------------------------
// 顶部菜单栏。
//-----------------------------------------------------------------------------
void DrawMenuBar();

//-----------------------------------------------------------------------------
// 右侧主面板：把 Entities 和 Brushes 做成两个 Tab。
//-----------------------------------------------------------------------------
void DrawSelectionPanel(CEntity* entities, int entityCount, int* pSelectedEntity,
                        const CBrush* brushes, int brushCount, int* pSelectedBrush,
                        BrushPanelResult& outResult);

//-----------------------------------------------------------------------------
// 底部控制台面板（操作提示 + 相机信息）。
//-----------------------------------------------------------------------------
void DrawConsolePanel(const Vector& camTarget, float camDist,
                      bool bShowGizmo, bool bXRayGizmo);

void DrawViewportPanel();

#endif // EDITOR_PANELS_H