#include "gizmo/GizmoDraw.h"

#include <cmath>
#include <GL/gl.h>

#include "mathlib/mathlib.h"

//-----------------------------------------------------------------------------
void DrawTranslateGizmo(const Vector& center, float length, int highlightAxis)
{
    float arrowSize = length * 0.12f;
    float arrowWidth = length * 0.06f;
    unsigned char cX[3] = {255, 0, 0};
    unsigned char cY[3] = {0, 255, 0};
    unsigned char cZ[3] = {0, 128, 255};
    if (highlightAxis == 0) { cX[0]=255; cX[1]=255; cX[2]=0; }
    if (highlightAxis == 1) { cY[0]=255; cY[1]=255; cY[2]=0; }
    if (highlightAxis == 2) { cZ[0]=255; cZ[1]=255; cZ[2]=0; }
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3ub(cX[0], cX[1], cX[2]);
    glVertex3f(center.x, center.y, center.z); glVertex3f(center.x + length, center.y, center.z);
    glColor3ub(cY[0], cY[1], cY[2]);
    glVertex3f(center.x, center.y, center.z); glVertex3f(center.x, center.y + length, center.z);
    glColor3ub(cZ[0], cZ[1], cZ[2]);
    glVertex3f(center.x, center.y, center.z); glVertex3f(center.x, center.y, center.z + length);
    glEnd();
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    glColor3ub(cX[0], cX[1], cX[2]);
    glVertex3f(center.x + length, center.y, center.z);
    glVertex3f(center.x + length - arrowSize, center.y + arrowWidth, center.z);
    glVertex3f(center.x + length, center.y, center.z);
    glVertex3f(center.x + length - arrowSize, center.y - arrowWidth, center.z);
    glVertex3f(center.x + length, center.y, center.z);
    glVertex3f(center.x + length - arrowSize, center.y, center.z + arrowWidth);
    glVertex3f(center.x + length, center.y, center.z);
    glVertex3f(center.x + length - arrowSize, center.y, center.z - arrowWidth);
    glColor3ub(cY[0], cY[1], cY[2]);
    glVertex3f(center.x, center.y + length, center.z);
    glVertex3f(center.x + arrowWidth, center.y + length - arrowSize, center.z);
    glVertex3f(center.x, center.y + length, center.z);
    glVertex3f(center.x - arrowWidth, center.y + length - arrowSize, center.z);
    glVertex3f(center.x, center.y + length, center.z);
    glVertex3f(center.x, center.y + length - arrowSize, center.z + arrowWidth);
    glVertex3f(center.x, center.y + length, center.z);
    glVertex3f(center.x, center.y + length - arrowSize, center.z - arrowWidth);
    glColor3ub(cZ[0], cZ[1], cZ[2]);
    glVertex3f(center.x, center.y, center.z + length);
    glVertex3f(center.x + arrowWidth, center.y, center.z + length - arrowSize);
    glVertex3f(center.x, center.y, center.z + length);
    glVertex3f(center.x - arrowWidth, center.y, center.z + length - arrowSize);
    glVertex3f(center.x, center.y, center.z + length);
    glVertex3f(center.x, center.y + arrowWidth, center.z + length - arrowSize);
    glVertex3f(center.x, center.y, center.z + length);
    glVertex3f(center.x, center.y - arrowWidth, center.z + length - arrowSize);
    glEnd();
    glLineWidth(1.0f);
}

//-----------------------------------------------------------------------------
void DrawRotationRing(const Vector& center, int axis, float radius, int segments,
                      unsigned char r, unsigned char g, unsigned char b)
{
    glColor3ub(r, g, b);
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
}

//-----------------------------------------------------------------------------
// 画一个轴对齐的"实心"立方体（用于 scale gizmo 的末端手柄）。
// center 是立方体中心，half 是半边长。
//-----------------------------------------------------------------------------
static void DrawSolidCube(const Vector& center, float half)
{
    Vector c[8] = {
        center + Vector(-half, -half, -half),
        center + Vector( half, -half, -half),
        center + Vector( half,  half, -half),
        center + Vector(-half,  half, -half),
        center + Vector(-half, -half,  half),
        center + Vector( half, -half,  half),
        center + Vector( half,  half,  half),
        center + Vector(-half,  half,  half),
    };

    glBegin(GL_QUADS);
    // -Z
    glVertex3f(c[0].x, c[0].y, c[0].z); glVertex3f(c[3].x, c[3].y, c[3].z);
    glVertex3f(c[2].x, c[2].y, c[2].z); glVertex3f(c[1].x, c[1].y, c[1].z);
    // +Z
    glVertex3f(c[4].x, c[4].y, c[4].z); glVertex3f(c[5].x, c[5].y, c[5].z);
    glVertex3f(c[6].x, c[6].y, c[6].z); glVertex3f(c[7].x, c[7].y, c[7].z);
    // -Y
    glVertex3f(c[0].x, c[0].y, c[0].z); glVertex3f(c[1].x, c[1].y, c[1].z);
    glVertex3f(c[5].x, c[5].y, c[5].z); glVertex3f(c[4].x, c[4].y, c[4].z);
    // +Y
    glVertex3f(c[3].x, c[3].y, c[3].z); glVertex3f(c[7].x, c[7].y, c[7].z);
    glVertex3f(c[6].x, c[6].y, c[6].z); glVertex3f(c[2].x, c[2].y, c[2].z);
    // -X
    glVertex3f(c[0].x, c[0].y, c[0].z); glVertex3f(c[4].x, c[4].y, c[4].z);
    glVertex3f(c[7].x, c[7].y, c[7].z); glVertex3f(c[3].x, c[3].y, c[3].z);
    // +X
    glVertex3f(c[1].x, c[1].y, c[1].z); glVertex3f(c[2].x, c[2].y, c[2].z);
    glVertex3f(c[6].x, c[6].y, c[6].z); glVertex3f(c[5].x, c[5].y, c[5].z);
    glEnd();
}

//-----------------------------------------------------------------------------
// Scale gizmo：3 根轴线 + 末端小方块。
//-----------------------------------------------------------------------------
void DrawScaleGizmo(const Vector& center, float length, int highlightAxis)
{
    unsigned char cX[3] = {255, 0, 0};
    unsigned char cY[3] = {0, 255, 0};
    unsigned char cZ[3] = {0, 128, 255};
    if (highlightAxis == 0) { cX[0]=255; cX[1]=255; cX[2]=0; }
    if (highlightAxis == 1) { cY[0]=255; cY[1]=255; cY[2]=0; }
    if (highlightAxis == 2) { cZ[0]=255; cZ[1]=255; cZ[2]=0; }

    // 轴：从 center 画到 length * 0.9（末端留给方块）
    float axisEnd = length * 0.9f;
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3ub(cX[0], cX[1], cX[2]);
    glVertex3f(center.x, center.y, center.z);
    glVertex3f(center.x + axisEnd, center.y, center.z);
    glColor3ub(cY[0], cY[1], cY[2]);
    glVertex3f(center.x, center.y, center.z);
    glVertex3f(center.x, center.y + axisEnd, center.z);
    glColor3ub(cZ[0], cZ[1], cZ[2]);
    glVertex3f(center.x, center.y, center.z);
    glVertex3f(center.x, center.y, center.z + axisEnd);
    glEnd();
    glLineWidth(1.0f);

    // 末端方块
    float half = length * 0.10f;
    if (half < 3.0f) half = 3.0f;

    glColor3ub(cX[0], cX[1], cX[2]);
    DrawSolidCube(center + Vector(length, 0, 0), half);

    glColor3ub(cY[0], cY[1], cY[2]);
    DrawSolidCube(center + Vector(0, length, 0), half);

    glColor3ub(cZ[0], cZ[1], cZ[2]);
    DrawSolidCube(center + Vector(0, 0, length), half);
}

//-----------------------------------------------------------------------------
void DrawGlowSphere(float cx, float cy, float cz, float r, int segments)
{
    for (int i = 0; i < segments; i++)
    {
        float lat0 = (float)M_PI * (-0.5f + (float)(i) / segments);
        float z0  = std::sin(lat0) * r;
        float r0  = std::cos(lat0) * r;

        float lat1 = (float)M_PI * (-0.5f + (float)(i + 1) / segments);
        float z1  = std::sin(lat1) * r;
        float r1  = std::cos(lat1) * r;

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= segments; j++)
        {
            float lng = 2.0f * (float)M_PI * (float)(j) / segments;
            float x = std::cos(lng);
            float y = std::sin(lng);

            glVertex3f(cx + x * r0, cy + y * r0, cz + z0);
            glVertex3f(cx + x * r1, cy + y * r1, cz + z1);
        }
        glEnd();
    }
}


//-----------------------------------------------------------------------------
void DrawPlayerStartModel(const Vector& pos, const QAngle& ang)
{
    const float radius = 16.0f;
    const float height = 72.0f;
    const int SEG = 16;
    glLineWidth(2.0f);
    glColor3ub(0, 200, 0);
    for (int ring = 0; ring < 2; ring++)
    {
        float z = pos.z + (ring == 0 ? 0.0f : height);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < SEG; i++)
        {
            float a = (float)i / SEG * 2.0f * (float)M_PI;
            glVertex3f(pos.x + cosf(a) * radius, pos.y + sinf(a) * radius, z);
        }
        glEnd();
    }
    glBegin(GL_LINES);
    for (int i = 0; i < 4; i++)
    {
        float a = (float)i / 4 * 2.0f * (float)M_PI;
        float cx = pos.x + cosf(a) * radius;
        float cy = pos.y + sinf(a) * radius;
        glVertex3f(cx, cy, pos.z);
        glVertex3f(cx, cy, pos.z + height);
    }
    glEnd();
    float yawRad = ang.y * (float)M_PI / 180.0f;
    float dx = cosf(yawRad), dy = sinf(yawRad);
    Vector tip = Vector(pos.x, pos.y, pos.z + height);
    Vector head = Vector(pos.x + dx * 30.0f, pos.y + dy * 30.0f, pos.z + height);
    glLineWidth(3.0f);
    glColor3ub(0, 255, 0);
    glBegin(GL_LINES);
    glVertex3f(tip.x, tip.y, tip.z);
    glVertex3f(head.x, head.y, head.z);
    float wingL = 8.0f;
    Vector left(-dy, dx, 0);
    Vector tipL = head - Vector(dx, dy, 0) * 8.0f + left * wingL;
    Vector tipR = head - Vector(dx, dy, 0) * 8.0f - left * wingL;
    glVertex3f(head.x, head.y, head.z); glVertex3f(tipL.x, tipL.y, tipL.z);
    glVertex3f(head.x, head.y, head.z); glVertex3f(tipR.x, tipR.y, tipR.z);
    glEnd();
    glLineWidth(1.0f);
}