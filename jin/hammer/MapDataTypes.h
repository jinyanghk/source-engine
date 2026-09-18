#ifndef MAP_DATA_TYPES_H
#define MAP_DATA_TYPES_H

#include <string>
#include <vector>

// Pure data structures representing Valve's VMF coordinate system
struct Vector3D {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct VmfFace {
    int id = 0;
    // Plane defined by 3 points: (x1 y1 z1) (x2 y2 z2) (x3 y3 z3)
    Vector3D planePoints[3];
    std::string material = "DEV/DEV_MEASUREGENERIC01";
    
    // Texture mapping attributes
    float uAxis[4] = {1.0f, 0.0f, 0.0f, 0.0f}; // [x y z shift]
    float vAxis[4] = {0.0f, -1.0f, 0.0f, 0.0f};
    float rotation = 0.0f;
    float uScale = 0.25f;
    float vScale = 0.25f;
};

struct VmfSolid {
    int id = 0;
    std::vector<VmfFace> faces;
};

struct VmfEntity {
    int id = 0;
    std::string classname = "info_player_start";
    Vector3D origin; // Pure data coordinates
    
    // Future expansion for KeyValues target fields: e.g. "targetname", "spawnflags"
};

#endif // MAP_DATA_TYPES_H
