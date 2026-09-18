#include "VmfIO.h"
#include "MapDocument.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

// Internal helper to turn custom Vector3D arrays into standard VMF text format
static std::string FormatVmfPlane(const Vector3D& p1, const Vector3D& p2, const Vector3D& p3) {
    std::ostringstream ss;
    // Format required by Valve: "(x y z) (x y z) (x y z)"
    ss << "(" << p1.x << " " << p1.y << " " << p1.z << ") "
       << "(" << p2.x << " " << p2.y << " " << p2.z << ") "
       << "(" << p3.x << " " << p3.y << " " << p3.z << ")";
    return ss.str();
}

static std::string FormatVmfAxis(const float axis[4]) {
    std::ostringstream ss;
    // Format required: "[x y z shift]"
    ss << "[" << axis[0] << " " << axis[1] << " " << axis[2] << " " << axis[3] << "]";
    return ss.str();
}

bool VmfIO::SaveToFile(const std::string& filePath, const MapDocument& doc) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    // Set consistent floating point presentation
    file << std::fixed << std::setprecision(2);

    // 1. Every VMF file requires a world block to contain primitive brushes
    file << "world\n"
         << "{\n"
         << "\t\"id\" \"1\"\n"
         << "\t\"mapversion\" \"1\"\n"
         << "\t\"classname\" \"worldspawn\"\n"
         << "\t\"skyname\" \"sky_day01_01\"\n"
         << "\t\"maxpropscreenwidth\" \"-1\"\n";

    // Export each unique solid tracked in the document
    const auto& solids = doc.GetSolids();
    for (const auto& solid : solids) {
        file << "\tsolid\n"
             << "\t{\n"
             << "\t\t\"id\" \"" << solid.id << "\"\n";

        for (const auto& face : solid.faces) {
            file << "\t\tside\n"
                 << "\t\t{\n"
                 << "\t\t\t\"id\" \"" << face.id << "\"\n";
            file << "\t\t\t\"plane\" \"" << FormatVmfPlane(face.planePoints[0], face.planePoints[1], face.planePoints[2]) << "\"\n";
            file << "\t\t\t\"material\" \"" << face.material << "\"\n";
            file << "\t\t\t\"uaxis\" " << FormatVmfAxis(face.uAxis) << " " << face.uScale << "\n";
            file << "\t\t\t\"vaxis\" " << FormatVmfAxis(face.vAxis) << " " << face.vScale << "\n";
            file << "\t\t\t\"rotation\" \"" << face.rotation << "\"\n";
            file << "\t\t\t\"lightmapscale\" \"16\"\n";
            file << "\t\t\t\"smoothing_groups\" \"0\"\n";
            file << "\t\t}\n";
        }
        file << "\t}\n";
    }

    // Close down the world definition block
    file << "}\n";

    // 2. Export Point Entities with Dynamic KeyValues
    const auto& entities = doc.GetEntities();
    for (const auto& ent : entities) {
        file << "entity\n"
             << "{\n"
             << "\t\"id\" \"" << ent.id << "\"\n"
             << "\t\"classname\" \"" << ent.classname << "\"\n"
             // Format coordinates: "origin" "X Y Z"
             << "\t\"origin\" \"" << ent.origin.x << " " << ent.origin.y << " " << ent.origin.z << "\"\n";

        // Dynamically loop through and export any attached user keys (angles, targetname, lights, etc.)
        for (const auto& kv : ent.properties) {
            // Prevent duplicate writing of base layout attributes if they were appended to the properties map
            if (kv.key != "id" && kv.key != "classname" && kv.key != "origin") {
                file << "\t\"" << kv.key << "\" \"" << kv.value << "\"\n";
            }
        }
        
        file << "}\n";
    }

    file.close();
    return true;
}

bool VmfIO::LoadFromFile(const std::string& filePath, MapDocument& doc) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    doc.Clear(); // Clear current session data

    std::string line;
    bool inSolid = false;
    bool inSide = false;
    bool inEntity = false; // New operational state flag

    VmfSolid tempSolid;
    VmfFace tempFace;
    VmfEntity tempEntity;  // New structural element staging zone

    while (std::getline(file, line)) {
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue; 
        size_t last = line.find_last_not_of(" \t\r\n");
        std::string cleanLine = line.substr(first, (last - first + 1));

        // Track context block entry tokens
        if (cleanLine == "solid") {
            inSolid = true;
            tempSolid = VmfSolid();
            continue;
        }
        if (cleanLine == "side" && inSolid) {
            inSide = true;
            tempFace = VmfFace();
            continue;
        }
        if (cleanLine == "entity") { // Detect entity root entries
            inEntity = true;
            tempEntity = VmfEntity();
            continue;
        }

        // Handle structural boundaries
        if (cleanLine == "}") {
            if (inSide) {
                tempSolid.faces.push_back(tempFace);
                inSide = false;
            } else if (inSolid) {
                doc.AddSolid(tempSolid);
                inSolid = false;
            } else if (inEntity) { 
                // Pass the fully populated struct directly down to our document manager
                doc.AddEntity(tempEntity); 
                inEntity = false;
            }
            continue;
        }

        std::istringstream iss(cleanLine);
        std::string key, val;
        if (iss >> std::quoted(key) >> std::quoted(val)) {
            if (inSide) {
                if (key == "id") tempFace.id = std::stoi(val);
                else if (key == "material") tempFace.material = val;
            } else if (inSolid && !inSide) {
                if (key == "id") tempSolid.id = std::stoi(val);
            } 
            else if (inEntity) { // Update our active entity variable extractor
                if (key == "id") {
                    tempEntity.id = std::stoi(val);
                } else if (key == "classname") {
                    tempEntity.classname = val;
                } else if (key == "origin") {
                    std::istringstream orgTokens(val);
                    orgTokens >> tempEntity.origin.x >> tempEntity.origin.y >> tempEntity.origin.z;
                } else {
                    // DYNAMIC PROPERTY EXTRACTION: Captures all standard target keys
                    VmfKeyValuePair kv;
                    kv.key = key;
                    kv.value = val;
                    tempEntity.properties.push_back(kv);
                }
            }
        }
    }

    file.close();
    return true;
}
