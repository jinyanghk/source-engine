#include "MapConverter.h"
#include "MapDocument.h"
#include "MapDataTypes.h"
#include "scene/Brush.h"
#include "scene/Entity.h"
#include "scene/SceneGlobals.h"
#include "app/FgdManager.h"
#include "app/LogBuffer.h"
#include "datacache/imdlcache.h"

extern IMDLCache* g_pMDLCache;   // ★ 如果还没 extern

#include "mathlib/vector.h"

#include <cstdio>
#include <algorithm>

//-----------------------------------------------------------------------------
// 取一个 brush face 的最终材质：
//   face.m_szMaterial 为空 -> 继承 brush.m_szMaterial
//   否则 -> 用 face 自己的
//-----------------------------------------------------------------------------
static std::string GetEffectiveFaceMaterial(const CBrush& b, int faceEnum)
{
    const BrushFace_t& face = b.m_faces[faceEnum];
    return face.m_szMaterial.empty() ? b.m_szMaterial : face.m_szMaterial;
}

//-----------------------------------------------------------------------------
// 导出单个 Brush 的所有面到 VmfSolid
//-----------------------------------------------------------------------------
static void BuildMeshFaces(const CBrush& b, VmfSolid& solid, int& nextFaceId)
{
    // 顺序对应 Brush.h 里的枚举：
    // FACE_POS_Z=0, FACE_NEG_Z=1, FACE_POS_X=2, FACE_NEG_X=3, FACE_POS_Y=4, FACE_NEG_Y=5
    // 但为了搭配标准的贴图轴方向，我们按以下顺序处理：
    int faceEnums[6] = {
        FACE_POS_X,  // 0
        FACE_NEG_X,  // 1
        FACE_NEG_Y,  // 2
        FACE_POS_Y,  // 3
        FACE_NEG_Z,  // 4
        FACE_POS_Z   // 5
    };

    // Standard Valve Hammer default texture axis alignment profiles
    float baseUAxes[6][4] = {
        {0.0f, 1.0f, 0.0f, 0.0f},  // +X
        {0.0f, 1.0f, 0.0f, 0.0f},  // -X
        {1.0f, 0.0f, 0.0f, 0.0f},  // -Y
        {-1.0f, 0.0f, 0.0f, 0.0f}, // +Y
        {1.0f, 0.0f, 0.0f, 0.0f},  // -Z
        {1.0f, 0.0f, 0.0f, 0.0f}   // +Z
    };

    float baseVAxes[6][4] = {
        {0.0f, 0.0f, -1.0f, 0.0f}, // +X
        {0.0f, 0.0f, -1.0f, 0.0f}, // -X
        {0.0f, 0.0f, -1.0f, 0.0f}, // -Y
        {0.0f, 0.0f, -1.0f, 0.0f}, // +Y
        {0.0f, -1.0f, 0.0f, 0.0f}, // -Z
        {0.0f, -1.0f, 0.0f, 0.0f}  // +Z
    };

    for (int i = 0; i < 6; i++)
    {
        VmfFace f;
        f.id = nextFaceId++;

        int faceIndex = faceEnums[i];
        const BrushFace_t& brushFace = b.m_faces[faceIndex];

        // ★ 修复：直接取 m_faces 里定义的前 3 个顶点，保证法线朝外 ★
        // Brush.cpp 里定义的顶点顺是逆时针的（从外面看），因此 (P2-P1)x(P3-P1) 算出来的法线必定朝外。
        for (int p = 0; p < 3; p++)
        {
            int vertexIndex = brushFace.m_nVertexIndices[p];
            Vector worldV = b.m_vertices[vertexIndex];

            f.planePoints[p].x = worldV.x;
            f.planePoints[p].y = worldV.y;
            f.planePoints[p].z = worldV.z;
        }

        // 获取材质
        f.material = GetEffectiveFaceMaterial(b, faceIndex);

        // 贴图轴
        for (int k = 0; k < 4; k++)
        {
            f.uAxis[k] = baseUAxes[i][k];
            f.vAxis[k] = baseVAxes[i][k];
        }

        f.uScale = 0.25f;
        f.vScale = 0.25f;
        f.rotation = 0.0f;
        f.lightmapScale = 16.0f;
        f.smoothingGroups = 0;
        solid.faces.push_back(f);
    }
}

//-----------------------------------------------------------------------------
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

        VmfSolid solid;
        solid.id = (int)i + 1;

        BuildMeshFaces(b, solid, nextFaceId);

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

        // 把 CEntity 的通用 keyvalue 写进 VMF
        for (const auto& kv : e.m_properties) {
            if (kv.first == "origin" || kv.first == "classname" || kv.first == "angles")
                continue;
            ent.properties.push_back({kv.first, kv.second});
        }

        doc.AddEntity(ent);
    }

    printf("[VMF] Conversion done: %zu solids, %zu entities\n",
           doc.GetSolids().size(), doc.GetEntities().size());
}

//-----------------------------------------------------------------------------
// 反向转换：从 MapDocument 恢复到 g_brushes / g_entities
//-----------------------------------------------------------------------------
void ConvertDocumentToEditor(const MapDocument& doc)
{
    g_brushes.clear();
    g_entities.clear();

    // 1. Brushes
    for (const auto& solid : doc.GetSolids())
    {
        // 收集所有 planePoints，算 AABB
        Vector mins( 1e9f,  1e9f,  1e9f);
        Vector maxs(-1e9f, -1e9f, -1e9f);
        std::string brushMaterial = "DEV/DEV_MEASUREGENERIC01";
        bool bHasGeometry = false;
        bool bHasBrushMaterial = false;

        for (const auto& face : solid.faces)
        {
            // brush 默认材质：取第一个非空 face.material
            if (!bHasBrushMaterial && !face.material.empty())
            {
                brushMaterial = face.material;
                bHasBrushMaterial = true;
            }

            for (int p = 0; p < 3; p++)
            {
                const Vector3D& pt = face.planePoints[p];

                // 跳过 (0,0,0) 的无效点
                if (pt.x == 0.0f && pt.y == 0.0f && pt.z == 0.0f)
                    continue;

                bHasGeometry = true;
                mins.x = std::min(mins.x, pt.x); maxs.x = std::max(maxs.x, pt.x);
                mins.y = std::min(mins.y, pt.y); maxs.y = std::max(maxs.y, pt.y);
                mins.z = std::min(mins.z, pt.z); maxs.z = std::max(maxs.z, pt.z);
            }
        }

        // 跳过"没几何"的 brush
        if (!bHasGeometry || mins.x > maxs.x) continue;

        CBrush b;
        b.m_vecPos  = (mins + maxs) * 0.5f;
        b.m_vecSize = (maxs - mins) * 0.5f;
        b.m_szMaterial = brushMaterial;
        b.RebuildPrimitiveMesh();

        // 映射表，把 VMF side 顺序对应回 m_faces 的枚举
        static const int kFaceEnumForWinding[6] = {
            FACE_POS_X,  // 0
            FACE_NEG_X,  // 1
            FACE_NEG_Y,  // 2
            FACE_POS_Y,  // 3
            FACE_NEG_Z,  // 4
            FACE_POS_Z   // 5
        };

        for (int i = 0; i < 6 && i < (int)solid.faces.size(); i++)
        {
            const std::string& mat = solid.faces[i].material;
            int faceEnum = kFaceEnumForWinding[i];

            if (mat.empty() || mat == brushMaterial)
                b.m_faces[faceEnum].m_szMaterial.clear();
            else
                b.m_faces[faceEnum].m_szMaterial = mat;
        }

        g_brushes.push_back(b);
    }

    // 2. Entities
    for (const auto& ent : doc.GetEntities())
    {
        // 跳过 worldspawn
        if (ent.classname == "worldspawn")
            continue;

        CEntity e;
        e.m_iType = ENTITY_MODEL;
        e.m_szName = ent.classname;
        e.m_vecPos = ent.origin;
        e.m_angRot = QAngle(0, 0, 0);
        e.m_hMdl = MDLHANDLE_INVALID;

        for (const auto& kv : ent.properties)
        {
            if (kv.key == "angles")
            {
                float p = 0, y = 0, r = 0;
                sscanf(kv.value.c_str(), "%f %f %f", &p, &y, &r);
                e.m_angRot = QAngle(p, y, r);
            }
            else if (kv.key == "origin" || kv.key == "classname" || kv.key == "id")
            {
                // 跳过（已经处理过）
            }
            else
            {
                e.m_properties[kv.key] = kv.value;
            }
        }

        // 加载模型
        std::string modelPath = FgdManager::Instance().GetModelPathForClass(ent.classname);
        if (!modelPath.empty())
            e.m_hMdl = g_pMDLCache->FindMDL(modelPath.c_str());

        // info_player_start 特殊处理
        if (ent.classname == "info_player_start")
            e.m_iType = ENTITY_PLAYER_START;

        g_entities.push_back(e);
    }

    // 日志
    char buf[256];
    snprintf(buf, sizeof(buf), "[VMF] Imported %zu brushes, %zu entities",
             g_brushes.size(), g_entities.size());
    LogBuffer::Instance().Add(buf);
    printf("%s\n", buf);
}