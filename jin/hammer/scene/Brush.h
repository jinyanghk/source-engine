#ifndef SCENE_BRUSH_H
#define SCENE_BRUSH_H

#include "mathlib/vector.h"

struct CBrush
{
    Vector m_vecPos;
    Vector m_vecSize;
    QAngle m_angRot;
    int    m_iTexId;

    CBrush() : m_vecPos(0,0,0), m_vecSize(32,32,32), m_angRot(0,0,0), m_iTexId(0) {}

    Vector GetBBoxMins() const { return m_vecPos - m_vecSize; }
    Vector GetBBoxMaxs() const { return m_vecPos + m_vecSize; }
};

void DrawBrush(const CBrush& b);
void DrawBrushEdges(const CBrush& b);

#endif // SCENE_BRUSH_H