#include "gizmo/GizmoPickTarget.h"

#include <cmath>
#include <GL/gl.h>

#include "mathlib/mathlib.h"

namespace GizmoPick
{

//-----------------------------------------------------------------------------
// Internal state
//-----------------------------------------------------------------------------
static int    s_width  = 0;
static int    s_height = 0;
static GLuint s_fbo       = 0;
static GLuint s_colorTex  = 0;
static GLuint s_depthRbo  = 0;
static bool   s_bInRender = false;

//-----------------------------------------------------------------------------
// ID color encoding:
//   red   = axis + 1   (0 = none, 1 = X, 2 = Y, 3 = Z)
//   green = handle type (0 = none, 1 = translate, 2 = rotate)
//   blue  = 0
//-----------------------------------------------------------------------------
static void EncodeColor(int axis, HandleType type, unsigned char outRGB[3])
{
    outRGB[0] = (unsigned char)(axis + 1);
    outRGB[1] = (unsigned char)type;
    outRGB[2] = 0;
}

//-----------------------------------------------------------------------------
bool Init(int width, int height)
{
    if (width <= 0 || height <= 0) return false;
    Resize(width, height);
    return s_fbo != 0;
}

//-----------------------------------------------------------------------------
void Shutdown()
{
    if (s_depthRbo) { glDeleteRenderbuffers(1, &s_depthRbo); s_depthRbo = 0; }
    if (s_colorTex) { glDeleteTextures(1, &s_colorTex); s_colorTex = 0; }
    if (s_fbo)      { glDeleteFramebuffers(1, &s_fbo); s_fbo = 0; }
    s_width = s_height = 0;
    s_bInRender = false;
}

//-----------------------------------------------------------------------------
void Resize(int width, int height)
{
    if (width <= 0 || height <= 0) return;
    if (width == s_width && height == s_height) return;

    s_width = width;
    s_height = height;

    // (Re)create resources.
    if (!s_fbo)
    {
        glGenFramebuffers(1, &s_fbo);
        glGenTextures(1, &s_colorTex);
        glGenRenderbuffers(1, &s_depthRbo);
    }

    glBindTexture(GL_TEXTURE_2D, s_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, s_width, s_height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, s_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, s_width, s_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, s_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, s_colorTex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, s_depthRbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//-----------------------------------------------------------------------------
void BeginRender(const float projMatrix[16], const float viewMatrix[16])
{
    if (!s_fbo) return;

    // Save GL state we are about to change. We assume nothing about what
    // the engine left us with.
    GLint prevFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);

    glBindFramebuffer(GL_FRAMEBUFFER, s_fbo);
    glViewport(0, 0, s_width, s_height);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  // 0,0,0 -> no hit
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    // Set up projection / view exactly as the visible gizmo uses.
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projMatrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(viewMatrix);

    s_bInRender = true;

    // Note: we do NOT restore prevFbo here; EndRender will do that.
    (void)prevFbo;
}

//-----------------------------------------------------------------------------
void EndRender()
{
    if (!s_bInRender) return;
    s_bInRender = false;

    // Restore default framebuffer (0).
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//-----------------------------------------------------------------------------
// Draw an axis line with the correct ID color, from 'from' to 'to'.
//-----------------------------------------------------------------------------
static void DrawIdLine(const Vector& from, const Vector& to, const unsigned char rgb[3])
{
    glColor3ub(rgb[0], rgb[1], rgb[2]);
    glVertex3f(from.x, from.y, from.z);
    glVertex3f(to.x, to.y, to.z);
}

//-----------------------------------------------------------------------------
// Draws the same triangle-fan arrowhead that the visible gizmo draws, using
// the given ID color. 'center' is the axis origin; 'length' is the axis
// length; tip is at center + axisDir * length.
//-----------------------------------------------------------------------------
static void DrawArrowHead(int axis, const Vector& center, float length,
                          const unsigned char rgb[3])
{
    const float arrowSize  = length * 0.12f;
    const float arrowWidth = length * 0.06f;

    Vector axisDir = (axis == 0) ? Vector(1,0,0)
                   : (axis == 1) ? Vector(0,1,0)
                                 : Vector(0,0,1);

    // Tip.
    Vector tip = center + axisDir * length;

    // The 4 "wing" endpoints, matching the visible gizmo's geometry.
    // For X-axis arrow, the wings splay in +Y/-Y and +Z/-Z.
    // We use a generic construction: pick two perpendicular unit vectors to
    // the axis; call them u and v.
    Vector u, v;
    switch (axis)
    {
    case 0: u = Vector(0,1,0); v = Vector(0,0,1); break;
    case 1: u = Vector(1,0,0); v = Vector(0,0,1); break;
    default: u = Vector(1,0,0); v = Vector(0,1,0); break;
    }

    Vector base = center + axisDir * (length - arrowSize);

    Vector w1a = base + u * arrowWidth;
    Vector w1b = base - u * arrowWidth;
    Vector w2a = base + v * arrowWidth;
    Vector w2b = base - v * arrowWidth;

    glColor3ub(rgb[0], rgb[1], rgb[2]);
    glBegin(GL_TRIANGLES);
    // +u wing
    glVertex3f(tip.x, tip.y, tip.z);
    glVertex3f(w1a.x, w1a.y, w1a.z);
    glVertex3f(w1b.x, w1b.y, w1b.z);
    // +v wing
    glVertex3f(tip.x, tip.y, tip.z);
    glVertex3f(w2a.x, w2a.y, w2a.z);
    glVertex3f(w2b.x, w2b.y, w2b.z);
    glEnd();
}

//-----------------------------------------------------------------------------
void DrawPickableTranslateArrow(int axis, const Vector& center, float length)
{
    if (!s_bInRender) return;

    unsigned char rgb[3];
    EncodeColor(axis, HANDLE_TRANSLATE, rgb);

    // Draw the whole axis as a thick colored line, and the arrowhead as a
    // solid triangle. Either region will register as a pick.
    //
    // We deliberately make the line reasonably thick (4 px) so the user can
    // also grab the middle of the axis, not just the arrowhead.
    Vector tip = center + ((axis == 0) ? Vector(1,0,0)
                       : (axis == 1) ? Vector(0,1,0)
                                     : Vector(0,0,1)) * length;

    glLineWidth(6.0f);
    glBegin(GL_LINES);
    DrawIdLine(center, tip, rgb);
    glEnd();
    glLineWidth(1.0f);

    DrawArrowHead(axis, center, length, rgb);
}

//-----------------------------------------------------------------------------
void DrawPickableRotateRing(int axis, const Vector& center, float radius, int segments)
{
    if (!s_bInRender) return;

    unsigned char rgb[3];
    EncodeColor(axis, HANDLE_ROTATE, rgb);

    glColor3ub(rgb[0], rgb[1], rgb[2]);
    glLineWidth(8.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++)
    {
        float angle = (float)i / segments * 2.0f * (float)M_PI;
        float c = cosf(angle) * radius;
        float s = sinf(angle) * radius;
        switch (axis)
        {
        case 0: glVertex3f(center.x, center.y + c, center.z + s); break;
        case 1: glVertex3f(center.x + c, center.y, center.z + s); break;
        case 2: glVertex3f(center.x + c, center.y + s, center.z); break;
        }
    }
    glEnd();
    glLineWidth(1.0f);
}

//-----------------------------------------------------------------------------
int Pick(int mouseX, int mouseY, HandleType& outType)
{
    outType = HANDLE_NONE;
    if (!s_fbo) return -1;
    if (mouseX < 0 || mouseY < 0 || mouseX >= s_width || mouseY >= s_height)
        return -1;

    // SDL mouse Y is top-down; GL readback Y is bottom-up.
    int glY = s_height - 1 - mouseY;

    // Preserve current FBO.
    GLint prevFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);

    glBindFramebuffer(GL_FRAMEBUFFER, s_fbo);
    unsigned char px[4] = {0,0,0,0};
    glReadPixels(mouseX, glY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
    glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);

    if (px[0] == 0) return -1;

    outType = (HandleType)px[1];
    return (int)px[0] - 1;
}

} // namespace GizmoPick