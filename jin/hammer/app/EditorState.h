#ifndef EDITOR_STATE_H
#define EDITOR_STATE_H

enum GizmoMode {
    GIZMO_NONE = 0,
    GIZMO_TRANSLATE,
    GIZMO_ROTATE,
    GIZMO_SCALE,
    GIZMO_FACE,
    GIZMO_VERTEX
};

extern int   g_iGizmoMode;    // 当前 gizmo 模式
extern float g_flGridSize;    // 网格大小（0 = 关闭）

#endif