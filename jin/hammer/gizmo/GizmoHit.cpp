#include "gizmo/GizmoHit.h"

#include <cmath>
#include <algorithm>

#include "mathlib/mathlib.h"

//-----------------------------------------------------------------------------
bool WorldToScreen(const Vector& worldPos,
                   const Vector& camEye, const Vector& camLeft, const Vector& camUp, const Vector& camForward,
                   float tanHalfFov, float aspect,
                   int screenW, int screenH,
                   float& outX, float& outY)
{
    Vector rel = worldPos - camEye;
    float depth = DotProduct(rel, camForward);
    if (depth < 0.01f) return false;
    float xCam = DotProduct(rel, camLeft);
    float yCam = DotProduct(rel, camUp);
    float ndcX = (xCam / depth) / (tanHalfFov * aspect);
    float ndcY = (yCam / depth) / tanHalfFov;
    outX = (ndcX * 0.5f + 0.5f) * screenW;
    outY = (1.0f - (ndcY * 0.5f + 0.5f)) * screenH;
    return true;
}

//-----------------------------------------------------------------------------
float DistPointToSegment2D(float px, float py,
                           float ax, float ay, float bx, float by,
                           float& outT)
{
    float dx = bx - ax, dy = by - ay;
    float lenSq = dx * dx + dy * dy;
    if (lenSq < 1e-6f)
    {
        outT = 0.0f;
        float ex = px - ax, ey = py - ay;
        return sqrtf(ex * ex + ey * ey);
    }
    float t = ((px - ax) * dx + (py - ay) * dy) / lenSq;
    t = std::max(0.0f, std::min(1.0f, t));
    outT = t;
    float cx = ax + dx * t, cy = ay + dy * t;
    float ex = px - cx, ey = py - cy;
    return sqrtf(ex * ex + ey * ey);
}

//-----------------------------------------------------------------------------
bool ComputeAxisScreenMetrics(
    const Vector& center, const Vector& axisDir, float axisLength,
    const Vector& camEye, const Vector& camLeft, const Vector& camUp, const Vector& camForward,
    float tanHalfFov, float aspect, int w, int h,
    float& outScreenDirX, float& outScreenDirY, float& outWorldPerPixel)
{
    float sx0, sy0, sx1, sy1;
    Vector p0 = center;
    Vector p1 = center + axisDir * axisLength;
    if (!WorldToScreen(p0, camEye, camLeft, camUp, camForward, tanHalfFov, aspect, w, h, sx0, sy0))
        return false;
    if (!WorldToScreen(p1, camEye, camLeft, camUp, camForward, tanHalfFov, aspect, w, h, sx1, sy1))
        return false;

    float dx = sx1 - sx0;
    float dy = sy1 - sy0;
    float screenLen = sqrtf(dx * dx + dy * dy);

    if (screenLen < 8.0f)
        return false;

    outScreenDirX = dx / screenLen;
    outScreenDirY = dy / screenLen;
    outWorldPerPixel = axisLength / screenLen;
    return true;
}

//-----------------------------------------------------------------------------
float RayToAxisSegmentDistSq(const Vector& rayOrigin, const Vector& rayDir,
                             const Vector& axisOrigin, const Vector& axisDir,
                             float axisLength, float& outT)
{
    Vector w0 = rayOrigin - axisOrigin;
    float a = DotProduct(rayDir, rayDir);
    float b = DotProduct(rayDir, axisDir);
    float c = DotProduct(axisDir, axisDir);
    float d = DotProduct(rayDir, w0);
    float e = DotProduct(axisDir, w0);
    float denom = a * c - b * b;
    if (fabsf(denom) < 1e-6f) { outT = 0; return 1e9f; }
    float tRay  = (b * e - c * d) / denom;
    float tAxis = (a * e - b * d) / denom;
    if (tRay < 0.0f) tRay = 0.0f;
    if (tAxis < 0.0f) tAxis = 0.0f;
    if (tAxis > axisLength) tAxis = axisLength;
    Vector pRay  = rayOrigin + rayDir * tRay;
    Vector pAxis = axisOrigin + axisDir * tAxis;
    outT = tAxis;
    return (pRay - pAxis).LengthSqr();
}

//-----------------------------------------------------------------------------
bool RayCylinderIntersect(const Vector& O, const Vector& D,
                          const Vector& A, const Vector& U,
                          float radius, float sMin, float sMax,
                          float& outT, float& outS)
{
    float dLen = D.Length();
    if (dLen < 1e-6f) return false;
    Vector d = D / dLen;

    Vector m = O - A;
    float md = DotProduct(m, d);
    float mu = DotProduct(m, U);
    float du = DotProduct(d, U);

    float a = 1.0f - du * du;
    float b = 2.0f * (md - mu * du);
    float c = DotProduct(m, m) - mu * mu - radius * radius;

    if (fabsf(a) < 1e-6f) return false;

    float disc = b * b - 4.0f * a * c;
    if (disc < 0.0f) return false;
    float sq = sqrtf(disc);
    float t0 = (-b - sq) / (2.0f * a);
    float t1 = (-b + sq) / (2.0f * a);
    if (t1 < 0.0f) return false;
    float tHit = (t0 >= 0.0f) ? t0 : t1;

    Vector pHit = O + d * (tHit * dLen);
    float s = DotProduct(pHit - A, U);
    if (s < sMin || s > sMax) return false;

    outT = tHit * dLen;
    outS = s;
    return true;
}

//-----------------------------------------------------------------------------
bool RayRingPlaneAngle(const Vector& rayOrigin, const Vector& rayDir,
                       const Vector& center, int axis, float& outAngle)
{
    Vector normal = (axis == 0) ? Vector(1,0,0) : (axis == 1) ? Vector(0,1,0) : Vector(0,0,1);
    float denom = DotProduct(rayDir, normal);
    if (fabsf(denom) < 1e-6f) return false;
    float t = DotProduct(center - rayOrigin, normal) / denom;
    if (t < 0.0f) return false;
    Vector hitPoint = rayOrigin + rayDir * t;
    Vector offset = hitPoint - center;
    Vector u, v;
    switch (axis)
    {
    case 0: u = Vector(0, 1, 0); v = Vector(0, 0, 1); break;
    case 1: u = Vector(1, 0, 0); v = Vector(0, 0, 1); break;
    case 2: u = Vector(1, 0, 0); v = Vector(0, 1, 0); break;
    }
    float pu = DotProduct(offset, u);
    float pv = DotProduct(offset, v);
    outAngle = atan2f(pv, pu);
    return true;
}

//-----------------------------------------------------------------------------
int PickTranslateAxis(const Vector& gizmoCenter, const Vector* axisDirs, float axisLength,
                      const Vector& camEye, const Vector& camLeft, const Vector& camUp, const Vector& camForward,
                      float tanHalfFov, float aspect,
                      int screenW, int screenH,
                      int mouseX, int mouseY,
                      float pixelThreshold, float originSkip, float exclusiveMargin)
{
    float sx0, sy0;
    if (!WorldToScreen(gizmoCenter, camEye, camLeft, camUp, camForward,
                       tanHalfFov, aspect, screenW, screenH, sx0, sy0))
        return -1;

    float pixDist[3] = { 1e9f, 1e9f, 1e9f };

    for (int a = 0; a < 3; a++)
    {
        float sx1, sy1;
        Vector p1 = gizmoCenter + axisDirs[a] * axisLength;
        if (!WorldToScreen(p1, camEye, camLeft, camUp, camForward,
                           tanHalfFov, aspect, screenW, screenH, sx1, sy1))
            continue;

        // Skip near-origin portion: all three axes converge there.
        float ax = sx0 + (sx1 - sx0) * originSkip;
        float ay = sy0 + (sy1 - sy0) * originSkip;

        float tDummy;
        pixDist[a] = DistPointToSegment2D((float)mouseX, (float)mouseY,
                                          ax, ay, sx1, sy1, tDummy);
    }

    // Find best below threshold.
    int bestAxis = -1;
    float bestPix = pixelThreshold;
    for (int a = 0; a < 3; a++)
        if (pixDist[a] < bestPix) { bestPix = pixDist[a]; bestAxis = a; }

    if (bestAxis < 0)
        return -1;

    // Require the winner to beat the runner-up by a clear margin.
    float secondBest = 1e9f;
    for (int a = 0; a < 3; a++)
        if (a != bestAxis && pixDist[a] < secondBest)
            secondBest = pixDist[a];

    if (secondBest - bestPix < exclusiveMargin)
        return -1;

    return bestAxis;
}