#ifndef SCENE_BRUSH_H
#define SCENE_BRUSH_H

#include <string>
#include "mathlib/vector.h"

// Near the top or bottom of hammer/scene/Brush.h
enum BrushFaceIndex {
    FACE_NONE = -1,
    FACE_POS_Z = 0,
    FACE_NEG_Z = 1,
    FACE_POS_X = 2,
    FACE_NEG_X = 3,
    FACE_POS_Y = 4,
    FACE_NEG_Y = 5
};

// Global variables to track state (can be placed inside SceneGlobals or main.cpp)
extern int g_iHoverFace;   // The brush face the mouse is currently hovering over
extern int g_iActiveFace;  // The face locked down during an active drag operation


struct CBrush
{
    Vector m_vecPos;
    Vector m_vecSize;
    QAngle m_angRot;
    int    m_iTexId;                  // 编辑器占位纹理 ID (0/1/2)
    std::string m_szMaterial;         // ★ VMF 材质路径，如 "BRICK/BRICKWALL001A"

    CBrush()
        : m_vecPos(0,0,0), m_vecSize(32,32,32), m_angRot(0,0,0), m_iTexId(0)
        , m_szMaterial("DEV/DEV_MEASUREGENERIC01")
    {}

    Vector GetBBoxMins() const { return m_vecPos - m_vecSize; }
    Vector GetBBoxMaxs() const { return m_vecPos + m_vecSize; }
};

void DrawBrush(const CBrush& b);
void DrawBrushEdges(const CBrush& b);

#endif // SCENE_BRUSH_H