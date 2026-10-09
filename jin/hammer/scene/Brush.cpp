#include "scene/Brush.h"
#include "render/TextureManager.h"

#include <GL/gl.h>

//-----------------------------------------------------------------------------
void DrawBrush(const CBrush& b)
{
    Vector mins = b.GetBBoxMins();
    Vector maxs = b.GetBBoxMaxs();
    float x0 = mins.x, x1 = maxs.x;
    float y0 = mins.y, y1 = maxs.y;
    float z0 = mins.z, z1 = maxs.z;
    const float TEX_SCALE = 1.0f / 64.0f;

    glBindTexture(GL_TEXTURE_2D, GetBrushTexture(b.m_iTexId));
    glColor3ub(255, 255, 255);
    glBegin(GL_QUADS);
    // +Z
    {
        float su = (x1 - x0) * TEX_SCALE, sv = (y1 - y0) * TEX_SCALE;
        glTexCoord2f(0,0); glVertex3f(x0, y0, z1);
        glTexCoord2f(su,0); glVertex3f(x1, y0, z1);
        glTexCoord2f(su,sv); glVertex3f(x1, y1, z1);
        glTexCoord2f(0,sv); glVertex3f(x0, y1, z1);
    }
    // -Z
    {
        float su = (x1 - x0) * TEX_SCALE, sv = (y1 - y0) * TEX_SCALE;
        glTexCoord2f(0,0); glVertex3f(x0, y0, z0);
        glTexCoord2f(0,sv); glVertex3f(x0, y1, z0);
        glTexCoord2f(su,sv); glVertex3f(x1, y1, z0);
        glTexCoord2f(su,0); glVertex3f(x1, y0, z0);
    }
    // +X
    {
        float su = (y1 - y0) * TEX_SCALE, sv = (z1 - z0) * TEX_SCALE;
        glTexCoord2f(0,0); glVertex3f(x1, y0, z0);
        glTexCoord2f(su,0); glVertex3f(x1, y1, z0);
        glTexCoord2f(su,sv); glVertex3f(x1, y1, z1);
        glTexCoord2f(0,sv); glVertex3f(x1, y0, z1);
    }
    // -X
    {
        float su = (y1 - y0) * TEX_SCALE, sv = (z1 - z0) * TEX_SCALE;
        glTexCoord2f(0,0); glVertex3f(x0, y0, z0);
        glTexCoord2f(0,sv); glVertex3f(x0, y0, z1);
        glTexCoord2f(su,sv); glVertex3f(x0, y1, z1);
        glTexCoord2f(su,0); glVertex3f(x0, y1, z0);
    }
    // +Y
    {
        float su = (x1 - x0) * TEX_SCALE, sv = (z1 - z0) * TEX_SCALE;
        glTexCoord2f(0,0); glVertex3f(x0, y1, z0);
        glTexCoord2f(su,0); glVertex3f(x1, y1, z0);
        glTexCoord2f(su,sv); glVertex3f(x1, y1, z1);
        glTexCoord2f(0,sv); glVertex3f(x0, y1, z1);
    }
    // -Y
    {
        float su = (x1 - x0) * TEX_SCALE, sv = (z1 - z0) * TEX_SCALE;
        glTexCoord2f(0,0); glVertex3f(x0, y0, z0);
        glTexCoord2f(0,sv); glVertex3f(x0, y0, z1);
        glTexCoord2f(su,sv); glVertex3f(x1, y0, z1);
        glTexCoord2f(su,0); glVertex3f(x1, y0, z0);
    }
    glEnd();
}

//-----------------------------------------------------------------------------
void DrawBrushEdges(const CBrush& b)
{
    Vector mins = b.GetBBoxMins();
    Vector maxs = b.GetBBoxMaxs();
    float x0 = mins.x, x1 = maxs.x;
    float y0 = mins.y, y1 = maxs.y;
    float z0 = mins.z, z1 = maxs.z;
    glLineWidth(1.5f);
    glColor3ub(60, 60, 60);
    glBegin(GL_LINES);
    glVertex3f(x0,y0,z0); glVertex3f(x1,y0,z0);
    glVertex3f(x1,y0,z0); glVertex3f(x1,y1,z0);
    glVertex3f(x1,y1,z0); glVertex3f(x0,y1,z0);
    glVertex3f(x0,y1,z0); glVertex3f(x0,y0,z0);
    glVertex3f(x0,y0,z1); glVertex3f(x1,y0,z1);
    glVertex3f(x1,y0,z1); glVertex3f(x1,y1,z1);
    glVertex3f(x1,y1,z1); glVertex3f(x0,y1,z1);
    glVertex3f(x0,y1,z1); glVertex3f(x0,y0,z1);
    glVertex3f(x0,y0,z0); glVertex3f(x0,y0,z1);
    glVertex3f(x1,y0,z0); glVertex3f(x1,y0,z1);
    glVertex3f(x1,y1,z0); glVertex3f(x1,y1,z1);
    glVertex3f(x0,y1,z0); glVertex3f(x0,y1,z1);
    glEnd();
    glLineWidth(1.0f);
}