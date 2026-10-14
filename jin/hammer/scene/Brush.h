#ifndef SCENE_BRUSH_H
#define SCENE_BRUSH_H

#include <string>
#include <vector>

#include "mathlib/vector.h"
#include "mathlib/mathlib.h"   // matrix3x4_t, AngleMatrix, VectorRotate

enum BrushFaceIndex {
    FACE_NONE = -1,
    FACE_POS_Z = 0,
    FACE_NEG_Z = 1,
    FACE_POS_X = 2,
    FACE_NEG_X = 3,
    FACE_POS_Y = 4,
    FACE_NEG_Y = 5
};

// ★ Create a helper struct to describe individual face mappings cleanly
struct BrushFace_t
{
    Vector m_vecNormal;       // Planar orientation normal
    int m_nVertexIndices[4];  // Indexes pointing back to the master vertex array
    std::string m_szMaterial; // Dynamic face-specific asset texturing path
};

struct CBrush
{
    Vector m_vecPos;
    Vector m_vecSize;
    QAngle m_angRot;
    int    m_iTexId;          
    std::string m_szMaterial; 

    // ★ THE UPGRADE: Explicit vertex and polygon arrays for advanced modeling
    std::vector<Vector> m_vertices; 
    std::vector<BrushFace_t> m_faces;

    CBrush()
        : m_vecPos(0,0,0), m_vecSize(32,32,32), m_angRot(0,0,0), m_iTexId(0)
        , m_szMaterial("DEV/DEV_MEASUREGENERIC01")
    {
        // Automatically populate our structural vertices on spawning
        RebuildPrimitiveMesh();
    }

    // Call this whenever m_vecPos or m_vecSize updates to sync your vertex caches
    void RebuildPrimitiveMesh();

    Vector GetBBoxMins() const { return m_vecPos - m_vecSize; }
    Vector GetBBoxMaxs() const { return m_vecPos + m_vecSize; }

    // 从 m_vertices 重算 m_vecPos 和 m_vecSize
    void RecalcBoundsFromVertices();

    // 把 8 个顶点绕中心旋转（旋转烘进顶点）
    void RotateVertices(const QAngle& deltaAngles);

    // 重算法线
    void RecalcFaceNormals();

};

extern int g_iHoverFace;   
extern int g_iActiveFace;  

void DrawBrush(const CBrush& b);
void DrawBrushEdges(const CBrush& b);

#endif // SCENE_BRUSH_H
