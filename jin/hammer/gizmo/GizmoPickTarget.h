#ifndef GIZMO_PICK_TARGET_H
#define GIZMO_PICK_TARGET_H

#include "mathlib/vector.h"

//-----------------------------------------------------------------------------
// Off-screen color-ID pick buffer for the gizmo.
//
// Each pickable handle (translate arrow, rotate ring) is rendered into an
// off-screen FBO with a unique RGB color that encodes (axis, handle-type).
// To pick, we read a single pixel at the mouse position and decode the color.
//
// This mirrors Hammer's "hit target" approach: no distance math, no
// thresholds, no ambiguity. Whatever pixel the mouse is over is the answer.
//-----------------------------------------------------------------------------
namespace GizmoPick
{
    // Handle types stored in the green channel of the ID color.
    enum HandleType
    {
        HANDLE_NONE      = 0,
        HANDLE_TRANSLATE = 1,
        HANDLE_ROTATE    = 2,
    };

    // Create / destroy the off-screen buffer. Init must be called once after
    // the GL context is ready. Call Resize on window resize. Shutdown before
    // GL context teardown.
    bool Init(int width, int height);
    void Shutdown();
    void Resize(int width, int height);

    // Per-frame rendering into the pick buffer. Call BeginRender once, then
    // any number of DrawPickable* calls, then EndRender.
    //
    // The caller must supply the projection and view matrices that were
    // loaded into GL for hand-drawn geometry (column-major, as passed to
    // glLoadMatrixf). These are the same matrices the visible gizmo uses.
    void BeginRender(const float projMatrix[16], const float viewMatrix[16]);
    void EndRender();

    // Draw the pickable geometry. Geometry must visually match the visible
    // gizmo (same arrow size / orientation / ring radius).
    void DrawPickableTranslateArrow(int axis, const Vector& center, float length);
    void DrawPickableRotateRing(int axis, const Vector& center, float radius, int segments);

    // Decode a mouse pick. Mouse coords are SDL convention (top-left origin,
    // Y grows down). Returns -1 if nothing is under the cursor, otherwise
    // returns the axis (0/1/2). outType receives the handle type.
    int Pick(int mouseX, int mouseY, HandleType& outType);
}

#endif // GIZMO_PICK_TARGET_H