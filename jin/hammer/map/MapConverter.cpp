#include "MapConverter.h"
#include "MapDocument.h"
#include "MapDataTypes.h"
#include "scene/Brush.h"
#include "scene/Entity.h"
#include "scene/SceneGlobals.h"
#include "mathlib/vector.h"

#include <cstdio>

// 贴图 ID → VMF material 路径
static const char* TexIdToMaterial(int texId)
{
    switch (texId)
    {
    case 0: return "DEV/DEV_MEASUREGENERIC01";   // Checker
    case 1: return "BRICK/BRICKWALL001A";         // Brick
    case 2: return "CONCRETE/CONCRETEFLOOR001A";  // Floor
    default: return "DEV/DEV_MEASUREGENERIC01";
    }
}

// VMF 约定：每个面 3 个平面点，逆时针 = 法线朝外
// 顺序来自 Valve 官方 hammer 导出的轴对齐 brush 模板
static void BuildAABBFaces(const CBrush& b, VmfSolid& solid, int& nextFaceId)
{
    Vector mins = b.m_vecPos - b.m_vecSize;
    Vector maxs = b.m_vecPos + b.m_vecSize;
    const char* mat = TexIdToMaterial(b.m_iTexId);

    // 每个面 3 个点：p0, p1, p2
    struct FaceDef {
        Vector3D p0, p1, p2;
        // uaxis / vaxis 默认值（轴对齐 brush 的标准值）
        float uAxis[4];
        float vAxis[4];
    };

    FaceDef faces[6];

    // +X (East)
    faces[0].p0 = Vector3D(maxs.x, mins.y, mins.z);
    faces[0].p1 = Vector3D(maxs.x, maxs.y, mins.z);
    faces[0].p2 = Vector3D(maxs.x, maxs.y, maxs.z);
    faces[0].uAxis[0]=0; faces[0].uAxis[1]=-1; faces[0].uAxis[2]=0; faces[0].uAxis[3]=0;
    faces[0].vAxis[0]=0; faces[0].vAxis[1]=0; faces[0].vAxis[2]=-1; faces[0].vAxis[3]=0;

    // -X (West)
    faces[1].p0 = Vector3D(mins.x, mins.y, mins.z);
    faces[1].p1 = Vector3D(mins.x, mins.y, maxs.z);
    faces[1].p2 = Vector3D(mins.x, maxs.y, maxs.z);
    faces[1].uAxis[0]=0; faces[1].uAxis[1]=1; faces[1].uAxis[2]=0; faces[1].uAxis[3]=0;
    faces[1].vAxis[0]=0; faces[1].vAxis[1]=0; faces[1].vAxis[2]=-1; faces[1].vAxis[3]=0;

    // -Y (South)
    faces[2].p0 = Vector3D(mins.x, mins.y, mins.z);
    faces[2].p1 = Vector3D(maxs.x, mins.y, mins.z);
    faces[2].p2 = Vector3D(maxs.x, mins.y, maxs.z);
    faces[2].uAxis[0]=1; faces[2].uAxis[1]=0; faces[2].uAxis[2]=0; faces[2].uAxis[3]=0;
    faces[2].vAxis[0]=0; faces[2].vAxis[1]=0; faces[2].vAxis[2]=-1; faces[2].vAxis[3]=0;

    // +Y (North)
    faces[3].p0 = Vector3D(mins.x, maxs.y, mins.z);
    faces[3].p1 = Vector3D(mins.x, maxs.y, maxs.z);
    faces[3].p2 = Vector3D(maxs.x, maxs.y, maxs.z);
    faces[3].uAxis[0]=-1; faces[3].uAxis[1]=0; faces[3].uAxis[2]=0; faces[3].uAxis[3]=0;
    faces[3].vAxis[0]=0; faces[3].vAxis[1]=0; faces[3].vAxis[2]=-1; faces[3].vAxis[3]=0;

    // -Z (Bottom)
    faces[4].p0 = Vector3D(mins.x, mins.y, mins.z);
    faces[4].p1 = Vector3D(mins.x, maxs.y, mins.z);
    faces[4].p2 = Vector3D(maxs.x, maxs.y, mins.z);
    faces[4].uAxis[0]=1; faces[4].uAxis[1]=0; faces[4].uAxis[2]=0; faces[4].uAxis[3]=0;
    faces[4].vAxis[0]=0; faces[4].vAxis[1]=-1; faces[4].vAxis[2]=0; faces[4].vAxis[3]=0;

    // +Z (Top)
    faces[5].p0 = Vector3D(mins.x, mins.y, maxs.z);
    faces[5].p1 = Vector3D(maxs.x, mins.y, maxs.z);
    faces[5].p2 = Vector3D(maxs.x, maxs.y, maxs.z);
    faces[5].uAxis[0]=1; faces[5].uAxis[1]=0; faces[5].uAxis[2]=0; faces[5].uAxis[3]=0;
    faces[5].vAxis[0]=0; faces[5].vAxis[1]=-1; faces[5].vAxis[2]=0; faces[5].vAxis[3]=0;

    for (int i = 0; i < 6; i++)
    {
        VmfFace f;
        f.id = nextFaceId++;
        f.planePoints[0] = faces[i].p0;
        f.planePoints[1] = faces[i].p1;
        f.planePoints[2] = faces[i].p2;
        f.material = mat;
        for (int k = 0; k < 4; k++) { f.uAxis[k] = faces[i].uAxis[k]; f.vAxis[k] = faces[i].vAxis[k]; }
        f.uScale = 0.25f;
        f.vScale = 0.25f;
        f.rotation = 0.0f;
        f.lightmapScale = 16.0f;
        f.smoothingGroups = 0;
        solid.faces.push_back(f);
    }
}

void ConvertEditorToDocument(MapDocument& doc)
{
    doc.Clear();

    printf("[VMF] Converting %zu brushes, %zu entities\n",
           g_brushes.size(), g_entities.size());

    // Brushes
    int nextFaceId = 1;
    for (size_t i = 0; i < g_brushes.size(); i++)
    {
        const CBrush& b = g_brushes[i];

        if (fabsf(b.m_angRot.x) > 0.01f ||
            fabsf(b.m_angRot.y) > 0.01f ||
            fabsf(b.m_angRot.z) > 0.01f)
        {
            printf("[VMF] WARNING: brush %zu has rotation (%.1f, %.1f, %.1f), ignored\n",
                   i, b.m_angRot.x, b.m_angRot.y, b.m_angRot.z);
            // 第一版：忽略旋转，只导出 AABB
        }

        VmfSolid solid;
        solid.id = (int)i + 1;
        BuildAABBFaces(b, solid, nextFaceId);
        doc.AddSolid(solid);
    }

    // Entities
    for (size_t i = 0; i < g_entities.size(); i++)
    {
        const CEntity& e = g_entities[i];

        VmfEntity ent;
        ent.id = (int)i + 1;
        ent.classname = e.m_szName;
        ent.origin = e.m_vecPos;

        // angles: "pitch yaw roll"
        if (fabsf(e.m_angRot.x) > 0.01f ||
            fabsf(e.m_angRot.y) > 0.01f ||
            fabsf(e.m_angRot.z) > 0.01f)
        {
            char buf[64];
            snprintf(buf, sizeof(buf), "%.2f %.2f %.2f",
                     e.m_angRot.x, e.m_angRot.y, e.m_angRot.z);
            ent.properties.push_back({"angles", buf});
        }

        doc.AddEntity(ent);
    }

    printf("[VMF] Conversion done: %zu solids, %zu entities\n",
           doc.GetSolids().size(), doc.GetEntities().size());
}