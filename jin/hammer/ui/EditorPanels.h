#ifndef EDITOR_PANELS_H
#define EDITOR_PANELS_H

#include "scene/Entity.h"
#include "mathlib/vector.h"

//-----------------------------------------------------------------------------
// 顶部菜单栏。
//-----------------------------------------------------------------------------
void DrawMenuBar();

//-----------------------------------------------------------------------------
// 右侧实体列表面板（列表 + 选中实体的属性编辑）。
//-----------------------------------------------------------------------------
void DrawEntityPanel(CEntity* entities, int entityCount, int* pSelectedIndex);

//-----------------------------------------------------------------------------
// 底部控制台面板（操作提示 + 相机信息）。
//-----------------------------------------------------------------------------
void DrawConsolePanel(const Vector& camTarget, float camDist,
                      bool bShowGizmo, bool bXRayGizmo);

#endif // EDITOR_PANELS_H