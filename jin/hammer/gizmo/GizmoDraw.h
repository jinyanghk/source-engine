#ifndef GIZMO_DRAW_H
#define GIZMO_DRAW_H

#include "mathlib/vector.h"

//-----------------------------------------------------------------------------
// Draws a translate gizmo (3 axes + arrowheads) centered at 'center'.
// 'highlightAxis' is -1 for none, 0/1/2 for X/Y/Z highlight.
//-----------------------------------------------------------------------------
void DrawTranslateGizmo(const Vector& center, float length, int highlightAxis);

//-----------------------------------------------------------------------------
// Draws a rotation ring around one world axis.
// axis: 0 = X (ring in YZ plane), 1 = Y (ring in XZ plane), 2 = Z (ring in XY plane)
//-----------------------------------------------------------------------------
void DrawRotationRing(const Vector& center, int axis, float radius, int segments,
                      unsigned char r, unsigned char g, unsigned char b);

//-----------------------------------------------------------------------------
// Draws a scale gizmo (3 axes + small cubes at the ends) centered at 'center'.
// 'highlightAxis' is -1 for none, 0/1/2 for X/Y/Z highlight.
//-----------------------------------------------------------------------------
void DrawScaleGizmo(const Vector& center, float length, int highlightAxis);

void DrawGlowSphere(float cx, float cy, float cz, float r, int segments);

//-----------------------------------------------------------------------------
// Draws the info_player_start placeholder (green capsule + facing arrow).
//-----------------------------------------------------------------------------
void DrawPlayerStartModel(const Vector& pos, const QAngle& ang);

#endif // GIZMO_DRAW_H