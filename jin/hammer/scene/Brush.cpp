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
    // 1. Fetch our unified dual-resource container from the cache manager
    MaterialTextureResource_t tex = MaterialPreview::Instance().GetMaterialResource(b.m_szMaterial);
    
    if (tex.worldRenderTexID == 0)
    {
        glBindTexture(GL_TEXTURE_2D, GetBrushTexture(b.m_iTexId));
        tex.nativeWidth = 64;
        tex.nativeHeight = 64;
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, tex.worldRenderTexID);
    }

    const float textureScale = 0.25f; 
    const float shiftU = 0.0f;
    const float shiftV = 0.0f;

    glColor3ub(255, 255, 255);
    glBegin(GL_QUADS);

    // ★ THE CORE CORRECTION: Loop over the 6 explicit index faces rather than static box bounds! ★
    for (int f = 0; f < 6; f++)
    {
        const BrushFace_t& face = b.m_faces[f];
        
        Vector uAxis, vAxis;
        GetBaseTextureVectors(face.m_vecNormal, uAxis, vAxis);

        for (int v = 0; v < 4; v++)
        {
            // Lookup the physical vertex index dynamically out of the mesh cache!
            int vertexIndex = face.m_nVertexIndices[v];
            const Vector& vertexPos = b.m_vertices[vertexIndex];

            // Linear Dot Product World-to-Texture projections
            float rawU = DotProduct(vertexPos, uAxis) + shiftU;
            float rawV = DotProduct(vertexPos, vAxis) + shiftV;

            float finalU = rawU * 0.25f;   // 直接乘以 0.25
            float finalV = rawV * 0.25f;

            glTexCoord2f(finalU, finalV);
            glVertex3f(vertexPos.x, vertexPos.y, vertexPos.z);
        }
    }

    glEnd();
}

//-----------------------------------------------------------------------------
void DrawBrushEdges(const CBrush& b)
{
    glLineWidth(1.5f);
    glColor3ub(60, 60, 60);
    glBegin(GL_LINES);

    // Loop through the faces to draw edge boundaries out of explicit vertex loops
    for (int f = 0; f < 6; f++)
    {
        const BrushFace_t& face = b.m_faces[f];
        for (int v = 0; v < 4; v++)
            {
                int i0 = face.m_nVertexIndices[v];
                int i1 = face.m_nVertexIndices[(v + 1) % 4];
                
                glVertex3f(b.m_vertices[i0].x, b.m_vertices[i0].y, b.m_vertices[i0].z);
                glVertex3f(b.m_vertices[i1].x, b.m_vertices[i1].y, b.m_vertices[i1].z);
            }
    }
    
    glEnd();
    glLineWidth(1.0f);
}


// Append this method inside hammer/scene/Brush.cpp:
void CBrush::RebuildPrimitiveMesh()
{
    Vector mins = GetBBoxMins();
    Vector maxs = GetBBoxMaxs();

    // 1. Generate the 8 unique local/world point coordinates of a standard bounding box
    m_vertices.resize(8);
    m_vertices[0] = Vector(mins.x, mins.y, mins.z); // Bottom-Back-Left
    m_vertices[1] = Vector(maxs.x, mins.y, mins.z); // Bottom-Back-Right
    m_vertices[2] = Vector(maxs.x, maxs.y, mins.z); // Bottom-Forward-Right
    m_vertices[3] = Vector(mins.x, maxs.y, mins.z); // Bottom-Forward-Left
    m_vertices[4] = Vector(mins.x, mins.y, maxs.z); // Top-Back-Left
    m_vertices[5] = Vector(maxs.x, mins.y, maxs.z); // Top-Back-Right
    m_vertices[6] = Vector(maxs.x, maxs.y, maxs.z); // Top-Forward-Right
    m_vertices[7] = Vector(mins.x, maxs.y, maxs.z); // Top-Forward-Left

    // 2. Define standard polygon index rings mapping our vertices counter-clockwise
    m_faces.resize(6);
    
    // +Z (Top Face)
    m_faces[FACE_POS_Z] = { Vector(0,0,1),  {4, 5, 6, 7}, m_szMaterial };
    // -Z (Bottom Face)
    m_faces[FACE_NEG_Z] = { Vector(0,0,-1), {0, 3, 2, 1}, m_szMaterial };
    // +X (Right Face)
    m_faces[FACE_POS_X] = { Vector(1,0,0),  {1, 2, 6, 5}, m_szMaterial };
    // -X (Left Face)
    m_faces[FACE_NEG_X] = { Vector(-1,0,0), {0, 4, 7, 3}, m_szMaterial };
    // +Y (Front Face)
    m_faces[FACE_POS_Y] = { Vector(0,1,0),  {3, 7, 6, 2}, m_szMaterial };
    // -Y (Back Face)
    m_faces[FACE_NEG_Y] = { Vector(0,-1,0), {0, 1, 5, 4}, m_szMaterial };
}

void CBrush::RecalcBoundsFromVertices()
{
    if (m_vertices.size() < 8) return;

    Vector vMins(1e9f, 1e9f, 1e9f);
    Vector vMaxs(-1e9f, -1e9f, -1e9f);
    for (const auto& p : m_vertices)
    {
        vMins.x = std::min(vMins.x, p.x); vMaxs.x = std::max(vMaxs.x, p.x);
        vMins.y = std::min(vMins.y, p.y); vMaxs.y = std::max(vMaxs.y, p.y);
        vMins.z = std::min(vMins.z, p.z); vMaxs.z = std::max(vMaxs.z, p.z);
    }
    m_vecPos = (vMins + vMaxs) * 0.5f;
    m_vecSize = (vMaxs - vMins) * 0.5f;
}

void CBrush::RecalcFaceNormals()
{
    for (auto& face : m_faces)
    {
        int i0 = face.m_nVertexIndices[0];
        int i1 = face.m_nVertexIndices[1];
        int i2 = face.m_nVertexIndices[2];

        Vector edge1 = m_vertices[i1] - m_vertices[i0];
        Vector edge2 = m_vertices[i2] - m_vertices[i0];
        Vector n;
        CrossProduct(edge1, edge2, n);
        VectorNormalize(n);
        face.m_vecNormal = n;
    }
}

void CBrush::RotateVertices(const QAngle& deltaAngles)
{
    if (m_vertices.size() < 8) return;

    matrix3x4_t matRot;
    AngleMatrix(deltaAngles, matRot);

    for (int v = 0; v < 8; v++)
    {
        Vector local = m_vertices[v] - m_vecPos;
        Vector rotated;
        VectorRotate(local, matRot, rotated);
        m_vertices[v] = m_vecPos + rotated;
    }
}