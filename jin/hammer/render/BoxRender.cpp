#include "render/BoxRender.h"

#include <GL/gl.h>

//-----------------------------------------------------------------------------
void DrawEntityBBox(const CEntity& ent, bool bSelected)
{
    float xmin = ent.m_vecBBoxMins.x, xmax = ent.m_vecBBoxMaxs.x;
    float ymin = ent.m_vecBBoxMins.y, ymax = ent.m_vecBBoxMaxs.y;
    float zmin = ent.m_vecBBoxMins.z, zmax = ent.m_vecBBoxMaxs.z;
    if (bSelected) { glLineWidth(3.0f); glColor3ub(255, 255, 0); }
    else { glLineWidth(1.5f); glColor3ub(120, 120, 120); }
    glBegin(GL_LINES);
    glVertex3f(xmin, ymin, zmin); glVertex3f(xmax, ymin, zmin);
    glVertex3f(xmax, ymin, zmin); glVertex3f(xmax, ymax, zmin);
    glVertex3f(xmax, ymax, zmin); glVertex3f(xmin, ymax, zmin);
    glVertex3f(xmin, ymax, zmin); glVertex3f(xmin, ymin, zmin);
    glVertex3f(xmin, ymin, zmax); glVertex3f(xmax, ymin, zmax);
    glVertex3f(xmax, ymin, zmax); glVertex3f(xmax, ymax, zmax);
    glVertex3f(xmax, ymax, zmax); glVertex3f(xmin, ymax, zmax);
    glVertex3f(xmin, ymax, zmax); glVertex3f(xmin, ymin, zmax);
    glVertex3f(xmin, ymin, zmin); glVertex3f(xmin, ymin, zmax);
    glVertex3f(xmax, ymin, zmin); glVertex3f(xmax, ymin, zmax);
    glVertex3f(xmax, ymax, zmin); glVertex3f(xmax, ymax, zmax);
    glVertex3f(xmin, ymax, zmin); glVertex3f(xmin, ymax, zmax);
    glEnd();
    glLineWidth(1.0f);
}

//-----------------------------------------------------------------------------
void DrawBrushBBox(const CBrush& brush, bool bSelected)
{
    Vector mins = brush.GetBBoxMins();
    Vector maxs = brush.GetBBoxMaxs();
    float xmin = mins.x, xmax = maxs.x;
    float ymin = mins.y, ymax = maxs.y;
    float zmin = mins.z, zmax = maxs.z;

    if (bSelected) { glLineWidth(3.0f); glColor3ub(255, 255, 0); }
    else           { glLineWidth(1.0f); glColor3ub(180, 180, 180); }

    glBegin(GL_LINES);
    glVertex3f(xmin, ymin, zmin); glVertex3f(xmax, ymin, zmin);
    glVertex3f(xmax, ymin, zmin); glVertex3f(xmax, ymax, zmin);
    glVertex3f(xmax, ymax, zmin); glVertex3f(xmin, ymax, zmin);
    glVertex3f(xmin, ymax, zmin); glVertex3f(xmin, ymin, zmin);
    glVertex3f(xmin, ymin, zmax); glVertex3f(xmax, ymin, zmax);
    glVertex3f(xmax, ymin, zmax); glVertex3f(xmax, ymax, zmax);
    glVertex3f(xmax, ymax, zmax); glVertex3f(xmin, ymax, zmax);
    glVertex3f(xmin, ymax, zmax); glVertex3f(xmin, ymin, zmax);
    glVertex3f(xmin, ymin, zmin); glVertex3f(xmin, ymin, zmax);
    glVertex3f(xmax, ymin, zmin); glVertex3f(xmax, ymin, zmax);
    glVertex3f(xmax, ymax, zmin); glVertex3f(xmax, ymax, zmax);
    glVertex3f(xmin, ymax, zmin); glVertex3f(xmin, ymax, zmax);
    glEnd();
    glLineWidth(1.0f);
}