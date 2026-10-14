#include "MapConverter.h"
#include "MapDocument.h"
#include "MapDataTypes.h"
#include "scene/Brush.h"
#include "scene/Entity.h"
#include "scene/SceneGlobals.h"
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
