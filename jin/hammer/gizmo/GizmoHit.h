#ifndef GIZMO_HIT_H
#define GIZMO_HIT_H

#include "mathlib/vector.h"

//-----------------------------------------------------------------------------
// Screen-space projection (pure math, no GL matrix dependency).
// Uses the same camera basis vectors that the renderer uses for hand-drawn
// geometry, so its output matches what's actually on screen.
//-----------------------------------------------------------------------------
bool WorldToScreen(const Vector& worldPos,
                   const Vector& camEye, const Vector& camLeft, const Vector& camUp, const Vector& camForward,
                   float tanHalfFov, float aspect,
                   int screenW, int screenH,
                   float& outX, float& outY);

//-----------------------------------------------------------------------------
// 2D point-to-segment distance, returns pixel distance and parametric t.
//-----------------------------------------------------------------------------
float DistPointToSegment2D(float px, float py,
                           float ax, float ay, float bx, float by,
                           float& outT);

//-----------------------------------------------------------------------------
// Screen-space axis direction and world-per-pixel scale along the axis.
// Returns false if the axis projects to fewer than 8 pixels (degenerate).
//-----------------------------------------------------------------------------
bool ComputeAxisScreenMetrics(
    const Vector& center, const Vector& axisDir, float axisLength,
    const Vector& camEye, const Vector& camLeft, const Vector& camUp, const Vector& camForward,
    float tanHalfFov, float aspect, int w, int h,
    float& outScreenDirX, float& outScreenDirY, float& outWorldPerPixel);

//-----------------------------------------------------------------------------
// World-space ray / axis segment closest distance squared.
//-----------------------------------------------------------------------------
float RayToAxisSegmentDistSq(const Vector& rayOrigin, const Vector& rayDir,
                             const Vector& axisOrigin, const Vector& axisDir,
                             float axisLength, float& outT);

//-----------------------------------------------------------------------------
// Ray-cylinder intersection (axis A + s*U, |U|=1, radius r, s in [sMin,sMax]).
//-----------------------------------------------------------------------------
bool RayCylinderIntersect(const Vector& O, const Vector& D,
                          const Vector& A, const Vector& U,
                          float radius, float sMin, float sMax,
                          float& outT, float& outS);

//-----------------------------------------------------------------------------
// Ray vs. ring plane; returns the angle on that ring plane.
//-----------------------------------------------------------------------------
bool RayRingPlaneAngle(const Vector& rayOrigin, const Vector& rayDir,
                       const Vector& center, int axis, float& outAngle);


//-----------------------------------------------------------------------------
// Screen-space hit test for the translate gizmo. Returns 0/1/2 for X/Y/Z,
// or -1 if no axis is unambiguously hit.
//   - Excludes the first 40% of each axis (near-origin region where all
//     three axes overlap).
//   - Requires the winner to be at least 'exclusiveMargin' pixels closer
//     than the runner-up, so overlapping candidates yield no selection.
//-----------------------------------------------------------------------------
int PickTranslateAxis(const Vector& gizmoCenter, const Vector* axisDirs, float axisLength,
                      const Vector& camEye, const Vector& camLeft, const Vector& camUp, const Vector& camForward,
                      float tanHalfFov, float aspect,
                      int screenW, int screenH,
                      int mouseX, int mouseY,
                      float pixelThreshold, float originSkip, float exclusiveMargin);                       


#endif // GIZMO_HIT_H