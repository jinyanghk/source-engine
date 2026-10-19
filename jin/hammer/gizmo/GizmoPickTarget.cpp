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
static GLint s_prevFbo = 0;
static GLint s_prevViewport[4] = {0,0,0,0};

void BeginRender(const float projMatrix[16], const float viewMatrix[16])
{
    if (!s_fbo) return;

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &s_prevFbo);
    glGetIntegerv(GL_VIEWPORT, s_prevViewport);

    glBindFramebuffer(GL_FRAMEBUFFER, s_fbo);
    glViewport(0, 0, s_width, s_height);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projMatrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(viewMatrix);

    s_bInRender = true;
}

//-----------------------------------------------------------------------------
void EndRender()
{
    if (!s_bInRender) return;
    s_bInRender = false;

    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)s_prevFbo);
    glViewport(s_prevViewport[0], s_prevViewport[1],
               s_prevViewport[2], s_prevViewport[3]);


    GLint currentFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFbo);
    static GLint s_lastFbo = -2;
    if (currentFbo != s_lastFbo) {
        s_lastFbo = currentFbo;
    }
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

    Vector axisDir = (axis == 0) ? Vector(1,0,0)
                   : (axis == 1) ? Vector(0,1,0)
                                 : Vector(0,0,1);

    // 直接画一个"扁平的四棱锥"（两个三角面）覆盖整个箭头区域，
    // 从 40% 处到 tip，宽度随位置渐变（底部宽，尖端窄）。
    // 这样不依赖 glLineWidth，覆盖整条轴的后 60%。
    Vector base40 = center + axisDir * (length * 0.75f);
    Vector tip    = center + axisDir * length;

    Vector u, v;
    switch (axis)
    {
    case 0: u = Vector(0,1,0); v = Vector(0,0,1); break;
    case 1: u = Vector(1,0,0); v = Vector(0,0,1); break;
    default: u = Vector(1,0,0); v = Vector(0,1,0); break;
    }

    // 底部的半宽（覆盖轴的中后段），大约 15% of length
    float halfW = length * 0.10f;

    // 底部四个角（u、v 两个方向的 ±）
    Vector b1 = base40 + u * halfW;
    Vector b2 = base40 - u * halfW;
    Vector b3 = base40 + v * halfW;
    Vector b4 = base40 - v * halfW;

    glColor3ub(rgb[0], rgb[1], rgb[2]);
    glBegin(GL_TRIANGLES);
    // 四个三角面，形成"从底部到 tip 的锥体"
    glVertex3f(tip.x, tip.y, tip.z); glVertex3f(b1.x, b1.y, b1.z); glVertex3f(b3.x, b3.y, b3.z);
    glVertex3f(tip.x, tip.y, tip.z); glVertex3f(b3.x, b3.y, b3.z); glVertex3f(b2.x, b2.y, b2.z);
    glVertex3f(tip.x, tip.y, tip.z); glVertex3f(b2.x, b2.y, b2.z); glVertex3f(b4.x, b4.y, b4.z);
    glVertex3f(tip.x, tip.y, tip.z); glVertex3f(b4.x, b4.y, b4.z); glVertex3f(b1.x, b1.y, b1.z);
    glEnd();
}

//-----------------------------------------------------------------------------
// 用一圈四边形（每个 segment 一个）绘制"带宽"足够的旋转环，
// 不依赖 glLineWidth（Mesa Intel 上限只有 ~7 像素）。
// ringWidth 是世界空间半径方向的厚度。
//-----------------------------------------------------------------------------
void DrawPickableRotateRing(int axis, const Vector& center, float radius, int segments)
{
    if (!s_bInRender) return;

    unsigned char rgb[3];
    EncodeColor(axis, HANDLE_ROTATE, rgb);

    // 世界空间厚度：环内外各 8% of radius
    float ringWidth = radius * 0.16f;
    float rInner = radius - ringWidth * 0.5f;
    float rOuter = radius + ringWidth * 0.5f;

    glColor3ub(rgb[0], rgb[1], rgb[2]);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++)
    {
        float angle = (float)i / segments * 2.0f * (float)M_PI;
        float c = cosf(angle);
        float s = sinf(angle);

        Vector innerP, outerP;
        switch (axis)
        {
        case 0: // X 轴：环在 YZ 平面
            innerP = Vector(center.x, center.y + c * rInner, center.z + s * rInner);
            outerP = Vector(center.x, center.y + c * rOuter, center.z + s * rOuter);
            break;
        case 1: // Y 轴：环在 XZ 平面
            innerP = Vector(center.x + c * rInner, center.y, center.z + s * rInner);
            outerP = Vector(center.x + c * rOuter, center.y, center.z + s * rOuter);
            break;
        default: // Z 轴：环在 XY 平面
            innerP = Vector(center.x + c * rInner, center.y + s * rInner, center.z);
            outerP = Vector(center.x + c * rOuter, center.y + s * rOuter, center.z);
            break;
        }
        // 用四边形带：先内圈顶点，再外圈顶点
        glVertex3f(innerP.x, innerP.y, innerP.z);
        glVertex3f(outerP.x, outerP.y, outerP.z);
    }
    glEnd();
}

//-----------------------------------------------------------------------------
// Scale pick target：末端一个小方块（cube），不依赖 glLineWidth。
// 方块的位置在轴末端，半宽 = length * 0.12f。
//-----------------------------------------------------------------------------
void DrawPickableScaleHandle(int axis, const Vector& center, float length)
{
    if (!s_bInRender) return;

    unsigned char rgb[3];
    EncodeColor(axis, HANDLE_SCALE, rgb);

    Vector axisDir = (axis == 0) ? Vector(1,0,0)
                   : (axis == 1) ? Vector(0,1,0)
                                 : Vector(0,0,1);

    // 方块中心在轴的末端
    Vector boxCenter = center + axisDir * length;

    // 半宽：让方块足够大，鼠标能点中
    float halfW = length * 0.10f;
    if (halfW < 3.0f) halfW = 3.0f;

    Vector u, v;
    switch (axis)
    {
    case 0: u = Vector(0,1,0); v = Vector(0,0,1); break;
    case 1: u = Vector(1,0,0); v = Vector(0,0,1); break;
    default: u = Vector(1,0,0); v = Vector(0,1,0); break;
    }

    // 8 个角点
    Vector c[8] = {
        boxCenter + u*halfW + v*halfW + axisDir*halfW,
        boxCenter + u*halfW + v*halfW - axisDir*halfW,
        boxCenter + u*halfW - v*halfW + axisDir*halfW,
        boxCenter + u*halfW - v*halfW - axisDir*halfW,
        boxCenter - u*halfW + v*halfW + axisDir*halfW,
        boxCenter - u*halfW + v*halfW - axisDir*halfW,
        boxCenter - u*halfW - v*halfW + axisDir*halfW,
        boxCenter - u*halfW - v*halfW - axisDir*halfW,
    };

    glColor3ub(rgb[0], rgb[1], rgb[2]);

    // 6 个面（每面 2 个三角形）
    glBegin(GL_TRIANGLES);
    // +axisDir 面
    glVertex3f(c[0].x, c[0].y, c[0].z); glVertex3f(c[2].x, c[2].y, c[2].z); glVertex3f(c[6].x, c[6].y, c[6].z);
    glVertex3f(c[0].x, c[0].y, c[0].z); glVertex3f(c[6].x, c[6].y, c[6].z); glVertex3f(c[4].x, c[4].y, c[4].z);
    // -axisDir 面
    glVertex3f(c[1].x, c[1].y, c[1].z); glVertex3f(c[5].x, c[5].y, c[5].z); glVertex3f(c[7].x, c[7].y, c[7].z);
    glVertex3f(c[1].x, c[1].y, c[1].z); glVertex3f(c[7].x, c[7].y, c[7].z); glVertex3f(c[3].x, c[3].y, c[3].z);
    // +u 面
    glVertex3f(c[0].x, c[0].y, c[0].z); glVertex3f(c[4].x, c[4].y, c[4].z); glVertex3f(c[5].x, c[5].y, c[5].z);
    glVertex3f(c[0].x, c[0].y, c[0].z); glVertex3f(c[5].x, c[5].y, c[5].z); glVertex3f(c[1].x, c[1].y, c[1].z);
    // -u 面
    glVertex3f(c[2].x, c[2].y, c[2].z); glVertex3f(c[3].x, c[3].y, c[3].z); glVertex3f(c[7].x, c[7].y, c[7].z);
    glVertex3f(c[2].x, c[2].y, c[2].z); glVertex3f(c[7].x, c[7].y, c[7].z); glVertex3f(c[6].x, c[6].y, c[6].z);
    // +v 面
    glVertex3f(c[0].x, c[0].y, c[0].z); glVertex3f(c[1].x, c[1].y, c[1].z); glVertex3f(c[3].x, c[3].y, c[3].z);
    glVertex3f(c[0].x, c[0].y, c[0].z); glVertex3f(c[3].x, c[3].y, c[3].z); glVertex3f(c[2].x, c[2].y, c[2].z);
    // -v 面
    glVertex3f(c[4].x, c[4].y, c[4].z); glVertex3f(c[6].x, c[6].y, c[6].z); glVertex3f(c[7].x, c[7].y, c[7].z);
    glVertex3f(c[4].x, c[4].y, c[4].z); glVertex3f(c[7].x, c[7].y, c[7].z); glVertex3f(c[5].x, c[5].y, c[5].z);
    glEnd();
}

//-----------------------------------------------------------------------------
void DrawPickableBrushFaces(const CBrush& b)
{
    if (b.m_vertices.size() < 8 || b.m_faces.size() < 6) return;

    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    // Face index -> color (red channel = face + 1)
    for (int f = 0; f < 6; f++)
    {
        glColor4ub(f + 1, 0, 0, 255);
        for (int v = 0; v < 4; v++)
        {
            int idx = b.m_faces[f].m_nVertexIndices[v];
            const Vector& p = b.m_vertices[idx];
            glVertex3f(p.x, p.y, p.z);
        }
    }

    glEnd();
}

//-----------------------------------------------------------------------------
// 12 条边的索引表
//-----------------------------------------------------------------------------
static const int s_edgeVertices[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},   // 底面
    {4, 5}, {5, 6}, {6, 7}, {7, 4},   // 顶面
    {0, 4}, {1, 5}, {2, 6}, {3, 7},   // 垂直边
};

//-----------------------------------------------------------------------------
// 画 12 条"粗边"（用 QUADS，不依赖 glLineWidth）
// 颜色编码：R = edgeIndex + 1 (1-12)，G = 0（不是 gizmo handle），B = 0
//-----------------------------------------------------------------------------
void DrawPickableBrushEdges(const CBrush& b)
{
    if (!s_bInRender) return;
    if (b.m_vertices.size() < 8) return;

    // 边的"世界空间"宽度 = 基于 brush 尺寸的 3%
    float edgeWidth = std::max(std::max(b.m_vecSize.x, b.m_vecSize.y), b.m_vecSize.z) * 0.06f;
    if (edgeWidth < 1.5f) edgeWidth = 1.5f;

    glDisable(GL_TEXTURE_2D);

    for (int e = 0; e < 12; e++)
    {
        const Vector& p0 = b.m_vertices[s_edgeVertices[e][0]];
        const Vector& p1 = b.m_vertices[s_edgeVertices[e][1]];

        // 边方向
        Vector dir = p1 - p0;
        float len = dir.Length();
        if (len < 0.001f) continue;
        dir *= (1.0f / len);

        // 找"垂直于边的两个方向"（u, v）
        Vector refUp(0, 0, 1);
        if (fabsf(DotProduct(dir, refUp)) > 0.99f)
            refUp = Vector(0, 1, 0);

        Vector u, v;
        CrossProduct(dir, refUp, u);
        VectorNormalize(u);
        CrossProduct(dir, u, v);
        VectorNormalize(v);

        // 4 个"边角"
        float hw = edgeWidth * 0.5f;
        Vector c0 = p0 + u * hw + v * hw;
        Vector c1 = p0 + u * hw - v * hw;
        Vector c2 = p0 - u * hw - v * hw;
        Vector c3 = p0 - u * hw + v * hw;
        Vector c4 = p1 + u * hw + v * hw;
        Vector c5 = p1 + u * hw - v * hw;
        Vector c6 = p1 - u * hw - v * hw;
        Vector c7 = p1 - u * hw + v * hw;

        // 颜色编码
        unsigned char r = (unsigned char)(e + 1);   // 1-12
        unsigned char g = 0;
        unsigned char bl = 0;

        glColor3ub(r, g, bl);
        glBegin(GL_QUADS);
        // 4 个侧面（"管"的四个面）
        glVertex3f(c0.x, c0.y, c0.z); glVertex3f(c4.x, c4.y, c4.z); glVertex3f(c5.x, c5.y, c5.z); glVertex3f(c1.x, c1.y, c1.z);
        glVertex3f(c1.x, c1.y, c1.z); glVertex3f(c5.x, c5.y, c5.z); glVertex3f(c6.x, c6.y, c6.z); glVertex3f(c2.x, c2.y, c2.z);
        glVertex3f(c2.x, c2.y, c2.z); glVertex3f(c6.x, c6.y, c6.z); glVertex3f(c7.x, c7.y, c7.z); glVertex3f(c3.x, c3.y, c3.z);
        glVertex3f(c3.x, c3.y, c3.z); glVertex3f(c7.x, c7.y, c7.z); glVertex3f(c4.x, c4.y, c4.z); glVertex3f(c0.x, c0.y, c0.z);
        // 两个端面
        glVertex3f(c0.x, c0.y, c0.z); glVertex3f(c3.x, c3.y, c3.z); glVertex3f(c2.x, c2.y, c2.z); glVertex3f(c1.x, c1.y, c1.z);
        glVertex3f(c4.x, c4.y, c4.z); glVertex3f(c5.x, c5.y, c5.z); glVertex3f(c6.x, c6.y, c6.z); glVertex3f(c7.x, c7.y, c7.z);
        glEnd();
    }

    glEnable(GL_TEXTURE_2D);
}

//-----------------------------------------------------------------------------
int Pick(int mouseX, int mouseY, HandleType& outType)
{
    outType = HANDLE_NONE;
    if (!s_fbo) return -1;
    if (mouseX < 0 || mouseY < 0 || mouseX >= s_width || mouseY >= s_height)
        return -1;

    // ★ 去掉 Y 翻转。因为 g_savedView 已经 Y 翻转过了，
    //   FBO 里的图像和屏幕坐标一致（Y 向下）。
    // int glY = s_height - 1 - mouseY;   // ← 删掉
    int glY = mouseY;                      // ← 改成直接用

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