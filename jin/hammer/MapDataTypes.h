#ifndef MAP_DATA_TYPES_H
#define MAP_DATA_TYPES_H

#include <string>
#include <vector>
#include "mathlib/vector.h"

// 1. Vector primitives matching Valve coordinates
struct Vector3D {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// 2. Brush primitive definitions
struct VmfFace {
    int id = 0;
    
    // FIX: Declare this as an explicit 3-element array to track the 3 plane anchor points
    Vector3D planePoints[3]; 
    
    std::string material = "DEV/DEV_MEASUREGENERIC01";
    float uAxis[4] = {1.0f, 0.0f, 0.0f, 0.0f}; // Ensure brackets are here from our previous fix
    float vAxis[4] = {0.0f, -1.0f, 0.0f, 0.0f};
    float rotation = 0.0f;
    float uScale = 0.25f;
    float vScale = 0.25f;
};

struct VmfSolid {
    int id = 0;
    std::vector<VmfFace> faces;
};

// 3. Dynamic KeyValue property structures
struct VmfKeyValuePair {
    std::string key;
    std::string value;
};

// 4. CLEAN SINGLE DEFINITION: Expose the dynamic property entity structure
struct VmfEntity {
    int id = 0;
    std::string classname = "info_player_start";
    Vector origin = Vector(0.0f, 0.0f, 0.0f);
    std::vector<VmfKeyValuePair> properties;
};

#endif // MAP_DATA_TYPES_H
