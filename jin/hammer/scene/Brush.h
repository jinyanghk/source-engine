#ifndef SCENE_BRUSH_H
#define SCENE_BRUSH_H

#include <string>
#include "mathlib/vector.h"

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