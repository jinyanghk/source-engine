#include "scene/EntityPick.h"
#include "gizmo/GizmoHit.h"

#include <algorithm>
#include <cmath>

bool IsMouseOverEntityBBox(const CEntity& ent,
                           const Vector& camEye, const Vector& camLeft, const Vector& camUp, const Vector& camForward,
                           float tanHalfFov, float aspect,
                           int screenW, int screenH,
                           int mouseX, int mouseY)
{
    Vector corners[8] = {
        Vector(ent.m_vecBBoxMins.x, ent.m_vecBBoxMins.y, ent.m_vecBBoxMins.z),
        Vector(ent.m_vecBBoxMaxs.x, ent.m_vecBBoxMins.y, ent.m_vecBBoxMins.z),
        Vector(ent.m_vecBBoxMins.x, ent.m_vecBBoxMaxs.y, ent.m_vecBBoxMins.z),
        Vector(ent.m_vecBBoxMaxs.x, ent.m_vecBBoxMaxs.y, ent.m_vecBBoxMins.z),
        Vector(ent.m_vecBBoxMins.x, ent.m_vecBBoxMins.y, ent.m_vecBBoxMaxs.z),
        Vector(ent.m_vecBBoxMaxs.x, ent.m_vecBBoxMins.y, ent.m_vecBBoxMaxs.z),
        Vector(ent.m_vecBBoxMins.x, ent.m_vecBBoxMaxs.y, ent.m_vecBBoxMaxs.z),
        Vector(ent.m_vecBBoxMaxs.x, ent.m_vecBBoxMaxs.y, ent.m_vecBBoxMaxs.z),
    };
    float sx[8], sy[8]; bool ok[8];
    float minX = 1e9f, maxX = -1e9f, minY = 1e9f, maxY = -1e9f;
    int okCount = 0;
    for (int i = 0; i < 8; i++)
    {
        ok[i] = WorldToScreen(corners[i], camEye, camLeft, camUp, camForward, tanHalfFov, aspect, screenW, screenH, sx[i], sy[i]);
        if (ok[i]) { minX = std::min(minX, sx[i]); maxX = std::max(maxX, sx[i]); minY = std::min(minY, sy[i]); maxY = std::max(maxY, sy[i]); okCount++; }
    }
    if (okCount < 2) return false;
    return (mouseX >= minX && mouseX <= maxX && mouseY >= minY && mouseY <= maxY);
}