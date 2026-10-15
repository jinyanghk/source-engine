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

// ★ THE ENHANCEMENT: Extract 3 plane coordinates directly from your explicit vertex arrays! ★
// Follows strict counter-clockwise winding rules so the map compiler doesn't throw inside-out face errors.
static void BuildMeshFaces(const CBrush& b, VmfSolid& solid, int& nextFaceId)
{
    const char* mat = b.m_szMaterial.c_str();

    // Map defining 3 distinct absolute world corners for the 6 side planes
    // Match the standard definitions populated inside Brush.cpp
    int faceWindingIndices[6][3] = {
        {1, 2, 6}, // +X (FACE_POS_X = 2)
        {0, 4, 7}, // -X (FACE_NEG_X = 3)
        {0, 1, 5}, // -Y (FACE_NEG_Y = 5)
        {3, 7, 6}, // +Y (FACE_POS_Y = 4)
        {0, 3, 2}, // -Z (FACE_NEG_Z = 1)
        {4, 5, 6}  // +Z (FACE_POS_Z = 0)
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

        // Fetch the 3 physical world-space vectors cleanly out of the cache map
        for (int p = 0; p < 3; p++)
        {
            int vertexIndex = faceWindingIndices[i][p];
            Vector worldV = b.m_vertices[vertexIndex];

            // Convert to the Document's local Vector3D storage standard
            f.planePoints[p].x = worldV.x;
            f.planePoints[p].y = worldV.y;
            f.planePoints[p].z = worldV.z;
        }

        f.material = mat;
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

void ConvertEditorToDocument(MapDocument& doc)
{
    doc.Clear();

    printf("[VMF] Converting %zu brushes, %zu entities\n",
           g_brushes.size(), g_entities.size()); //

    // Brushes
    int nextFaceId = 1; //
    for (size_t i = 0; i < g_brushes.size(); i++) //
    {
        const CBrush& b = g_brushes[i]; //

        // ★ Notice: We can completely remove the old "ignored rotation" warning message here
        // because our upgraded mesh array system natively carries rotations effortlessly!

        VmfSolid solid; //
        solid.id = (int)i + 1; //
        
        // ★ Call your upgraded mesh vector extractor pass! ★
        BuildMeshFaces(b, solid, nextFaceId);
        
        doc.AddSolid(solid); //
    }

    // Entities
    for (size_t i = 0; i < g_entities.size(); i++) //
    {
        const CEntity& e = g_entities[i]; //

        VmfEntity ent; //
        ent.id = (int)i + 1; //
        ent.classname = e.m_szName; //
        ent.origin = e.m_vecPos; //

        // angles: "pitch yaw roll"
        if (fabsf(e.m_angRot.x) > 0.01f || //
            fabsf(e.m_angRot.y) > 0.01f || //
            fabsf(e.m_angRot.z) > 0.01f) //
        {
            char buf[64]; //
            snprintf(buf, sizeof(buf), "%.2f %.2f %.2f", //
                     e.m_angRot.x, e.m_angRot.y, e.m_angRot.z); //
            ent.properties.push_back({"angles", buf}); //
        }
        
        // 把 CEntity 的通用 keyvalue 写进 VMF
        for (const auto& kv : e.m_properties) { //
            // 跳过已经处理过的
            if (kv.first == "origin" || kv.first == "classname" || kv.first == "angles") //
                continue; //
            ent.properties.push_back({kv.first, kv.second}); //
        }

        doc.AddEntity(ent); //
    }

    printf("[VMF] Conversion done: %zu solids, %zu entities\n", //
           doc.GetSolids().size(), doc.GetEntities().size()); //
}

//-----------------------------------------------------------------------------
// 反向转换：从 MapDocument 恢复到 g_brushes / g_entities
// 第一版：只支持"轴对齐 brush"（从所有 planePoints 算 AABB）
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
        std::string material = "DEV/DEV_MEASUREGENERIC01";
        bool bHasGeometry = false;

        for (const auto& face : solid.faces)
        {
            if (!face.material.empty())
                material = face.material;

            for (int p = 0; p < 3; p++)
            {
                const Vector3D& pt = face.planePoints[p];

                // 跳过 (0,0,0) 的无效点（如果 plane 没解析成功，planePoints 全是 0）
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
        b.m_szMaterial = material;
        b.RebuildPrimitiveMesh();
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

        // 处理 properties
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