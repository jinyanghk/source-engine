#include "scene/Brush.h"
#include "render/TextureManager.h"
#include "render/MaterialPreview.h" // ★ ADD: Gain access to the new multi-handle metadata caching tools

#include <GL/gl.h>
#include <cmath>

//-----------------------------------------------------------------------------
// Valve's standard base texturing coordinate projection matrix vectors.
// This determines standard U and V direction guidelines for walls, floors, and ceilings.
//-----------------------------------------------------------------------------
static void GetBaseTextureVectors(const Vector& normal, Vector& outU, Vector& outV)
{
    float ax = fabsf(normal.x);
    float ay = fabsf(normal.y);
    float az = fabsf(normal.z);

    // 1. Plane is facing primarily Up or Down (+Z / -Z) -> Project along X/Y grid
    if (az >= ax && az >= ay)
    {
        outU.Init(1.0f, 0.0f, 0.0f);
        outV.Init(0.0f, -1.0f, 0.0f);
    }
    // 2. Plane is facing primarily Left or Right (+X / -X) -> Project along Y/Z grid
    else if (ax >= ay && ax >= az)
    {
        outU.Init(0.0f, 1.0f, 0.0f);
        outV.Init(0.0f, 0.0f, -1.0f);
    }
    // 3. Plane is facing primarily Forward or Backward (+Y / -Y) -> Project along X/Z grid
    else
    {
        outU.Init(1.0f, 0.0f, 0.0f);
        outV.Init(0.0f, 0.0f, -1.0f);
    }
}

//-----------------------------------------------------------------------------
void DrawBrush(const CBrush& b)
{
    Vector mins = b.GetBBoxMins(); //
    Vector maxs = b.GetBBoxMaxs(); //
    float x0 = mins.x, x1 = maxs.x; //
    float y0 = mins.y, y1 = maxs.y; //
    float z0 = mins.z, z1 = maxs.z; //

    // 1. Fetch our unified dual-resource container from the cache manager
    MaterialTextureResource_t tex = MaterialPreview::Instance().GetMaterialResource(b.m_szMaterial); //
    
    // Fall back to your editor placeholder texture if no VPK asset matches or is still loading
    if (tex.worldRenderTexID == 0)
    {
        glBindTexture(GL_TEXTURE_2D, GetBrushTexture(b.m_iTexId)); //
        tex.nativeWidth = 64;  // Fallback dimension tracking
        tex.nativeHeight = 64;
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, tex.worldRenderTexID); //
    }

    // Default Hammer texturing alignment scalars
    const float textureScale = 0.25f; // Standard Valve editor default scale multiplier (maps pixels perfectly to grid units)
    const float shiftU = 0.0f;
    const float shiftV = 0.0f;

    glColor3ub(255, 255, 255); //
    glBegin(GL_QUADS); //

    // Structure defining the 6 quad face configurations: Face Normal, and its 4 absolute world corner vertices
    struct FaceDefinition_t {
        Vector normal;
        Vector v[4];
    };

    FaceDefinition_t faces[6] = {
        { Vector(0,0,1),  { Vector(x0,y0,z1), Vector(x1,y0,z1), Vector(x1,y1,z1), Vector(x0,y1,z1) } }, // +Z
        { Vector(0,0,-1), { Vector(x0,y0,z0), Vector(x0,y1,z0), Vector(x1,y1,z0), Vector(x1,y0,z0) } }, // -Z
        { Vector(1,0,0),  { Vector(x1,y0,z0), Vector(x1,y1,z0), Vector(x1,y1,z1), Vector(x1,y0,z1) } }, // +X
        { Vector(-1,0,0), { Vector(x0,y0,z0), Vector(x0,y0,z1), Vector(x0,y1,z1), Vector(x0,y1,z0) } }, // -X
        { Vector(0,1,0),  { Vector(x0,y1,z0), Vector(x1,y1,z0), Vector(x1,y1,z1), Vector(x0,y1,z1) } }, // +Y
        { Vector(0,-1,0), { Vector(x0,y0,z0), Vector(x0,y0,z1), Vector(x1,y0,z1), Vector(x1,y0,z0) } }  // -Y
    };

    for (int f = 0; f < 6; f++)
    {
        Vector uAxis, vAxis;
        GetBaseTextureVectors(faces[f].normal, uAxis, vAxis);

        for (int v = 0; v < 4; v++)
        {
            const Vector& vertexPos = faces[f].v[v];

            // 2. Linear Dot Product World-to-Texture projections
            float rawU = DotProduct(vertexPos, uAxis) + shiftU;
            float rawV = DotProduct(vertexPos, vAxis) + shiftV;

            // 3. Normalize UV coordinates based on layout width/height metrics and default texture scales
            float finalU = rawU / ((float)tex.nativeWidth  * textureScale);
            float finalV = rawV / ((float)tex.nativeHeight * textureScale);

            glTexCoord2f(finalU, finalV); //
            glVertex3f(vertexPos.x, vertexPos.y, vertexPos.z); //
        }
    }

    glEnd(); //
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