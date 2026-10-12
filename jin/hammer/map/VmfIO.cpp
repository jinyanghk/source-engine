#include "VmfIO.h"
#include "MapDocument.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstdlib>

//-----------------------------------------------------------------------------
// 内部辅助：把 3 个点格式化成 VMF 平面字符串 "(x y z) (x y z) (x y z)"
//-----------------------------------------------------------------------------
/*
static std::string FormatVmfPlane(const Vector3D& p1, const Vector3D& p2, const Vector3D& p3) {
    std::ostringstream ss;
    ss << "(" << p1.x << " " << p1.y << " " << p1.z << ") "
       << "(" << p2.x << " " << p2.y << " " << p2.z << ") "
       << "(" << p3.x << " " << p3.y << " " << p3.z << ")";
    return ss.str();
}
*/
static std::string FormatVmfPlane(const Vector3D& p1, const Vector3D& p2, const Vector3D& p3) {
    std::ostringstream ss;
    // ★ 交换 p1 和 p3（即原来的 p0 和 p2）
    ss << "(" << p3.x << " " << p3.y << " " << p3.z << ") "
       << "(" << p2.x << " " << p2.y << " " << p2.z << ") "
       << "(" << p1.x << " " << p1.y << " " << p1.z << ")";
    return ss.str();
}

//-----------------------------------------------------------------------------
// 内部辅助：把 4 元轴格式化成 "[x y z shift]"
//-----------------------------------------------------------------------------
static std::string FormatVmfAxis(const float axis[4]) {
    std::ostringstream ss;
    ss << "[" << axis[0] << " " << axis[1] << " " << axis[2] << " " << axis[3] << "]";
    return ss.str();
}

//-----------------------------------------------------------------------------
// 内部辅助：从一行里解析出第一个引号字符串。
// 返回 true 并推进 pos；失败返回 false。
//-----------------------------------------------------------------------------
static bool ParseQuoted(const std::string& line, size_t& pos, std::string& out) {
    // 跳过空白
    while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) pos++;
    if (pos >= line.size() || line[pos] != '"') return false;
    pos++; // 跳过开引号
    size_t start = pos;
    while (pos < line.size() && line[pos] != '"') pos++;
    if (pos >= line.size()) return false;
    out = line.substr(start, pos - start);
    pos++; // 跳过闭引号
    return true;
}

//-----------------------------------------------------------------------------
// 内部辅助：解析 "(x y z) (x y z) (x y z)" 到一个 VmfFace 的 planePoints
//-----------------------------------------------------------------------------
static bool ParseVmfPlane(const std::string& value, Vector3D outPoints[3]) {
    size_t pos = 0;
    for (int i = 0; i < 3; i++) {
        while (pos < value.size() && value[pos] != '(') pos++;
        if (pos >= value.size()) return false;
        pos++; // 跳过 '('
        float x = 0, y = 0, z = 0;
        if (sscanf(value.c_str() + pos, "%f %f %f", &x, &y, &z) != 3) return false;
        outPoints[i].x = x;
        outPoints[i].y = y;
        outPoints[i].z = z;
        while (pos < value.size() && value[pos] != ')') pos++;
        if (pos < value.size()) pos++; // 跳过 ')'
    }
    return true;
}

//-----------------------------------------------------------------------------
// 内部辅助：解析 "[x y z shift]" 到一个 float[4]
//-----------------------------------------------------------------------------
static bool ParseVmfAxis(const std::string& value, float outAxis[4]) {
    size_t pos = 0;
    while (pos < value.size() && value[pos] != '[') pos++;
    if (pos >= value.size()) return false;
    pos++;
    float x = 0, y = 0, z = 0, s = 0;
    if (sscanf(value.c_str() + pos, "%f %f %f %f", &x, &y, &z, &s) != 4) return false;
    outAxis[0] = x; outAxis[1] = y; outAxis[2] = z; outAxis[3] = s;
    return true;
}

//-----------------------------------------------------------------------------
// 内部辅助：解析 "x y z" 到一个 Vector
//-----------------------------------------------------------------------------
static bool ParseVmfVector(const std::string& value, Vector& out) {
    float x = 0, y = 0, z = 0;
    if (sscanf(value.c_str(), "%f %f %f", &x, &y, &z) != 3) return false;
    out.x = x; out.y = y; out.z = z;
    return true;
}

//-----------------------------------------------------------------------------
bool VmfIO::SaveToFile(const std::string& filePath, const MapDocument& doc) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    file << std::fixed << std::setprecision(2);

    // 1. world 块
    file << "world\n"
         << "{\n"
         << "\t\"id\" \"1\"\n"
         << "\t\"mapversion\" \"1\"\n"
         << "\t\"classname\" \"worldspawn\"\n"
         << "\t\"skyname\" \"sky_day01_01\"\n"
         << "\t\"maxpropscreenwidth\" \"-1\"\n";

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
            file << "\t\t\t\"uaxis\" \"" << FormatVmfAxis(face.uAxis) << " " << face.uScale << "\"\n";
            file << "\t\t\t\"vaxis\" \"" << FormatVmfAxis(face.vAxis) << " " << face.vScale << "\"\n";
            file << "\t\t\t\"rotation\" \"" << face.rotation << "\"\n";
            file << "\t\t\t\"lightmapscale\" \"" << face.lightmapScale << "\"\n";
            file << "\t\t\t\"smoothing_groups\" \"" << face.smoothingGroups << "\"\n";
            file << "\t\t}\n";
        }
        file << "\t}\n";
    }

    file << "}\n";

    // 2. entity 块
    const auto& entities = doc.GetEntities();
    for (const auto& ent : entities) {
        file << "entity\n"
             << "{\n"
             << "\t\"id\" \"" << ent.id << "\"\n"
             << "\t\"classname\" \"" << ent.classname << "\"\n"
             << "\t\"origin\" \"" << ent.origin.x << " " << ent.origin.y << " " << ent.origin.z << "\"\n";

        for (const auto& kv : ent.properties) {
            if (kv.key != "id" && kv.key != "classname" && kv.key != "origin") {
                file << "\t\"" << kv.key << "\" \"" << kv.value << "\"\n";
            }
        }

        file << "}\n";
    }

    file.close();
    return true;
}

//-----------------------------------------------------------------------------
bool VmfIO::LoadFromFile(const std::string& filePath, MapDocument& doc) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    doc.Clear();

    std::string line;
    bool inSolid = false;
    bool inSide = false;
    bool inEntity = false;

    VmfSolid tempSolid;
    VmfFace tempFace;
    VmfEntity tempEntity;

    while (std::getline(file, line)) {
        // 去首尾空白
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        size_t last = line.find_last_not_of(" \t\r\n");
        std::string cleanLine = line.substr(first, (last - first + 1));

        // 块入口
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
        if (cleanLine == "entity") {
            inEntity = true;
            tempEntity = VmfEntity();
            continue;
        }

        // 块边界
        if (cleanLine == "}") {
            if (inSide) {
                tempSolid.faces.push_back(tempFace);
                inSide = false;
            } else if (inSolid) {
                doc.AddSolid(tempSolid);
                inSolid = false;
            } else if (inEntity) {
                doc.AddEntity(tempEntity);
                inEntity = false;
            }
            continue;
        }

        // 解析 "key" "value"
        size_t pos = 0;
        std::string key, val;
        if (!ParseQuoted(cleanLine, pos, key)) continue;
        if (!ParseQuoted(cleanLine, pos, val)) continue;

        if (inSide) {
            if (key == "id") tempFace.id = std::atoi(val.c_str());
            else if (key == "material") tempFace.material = val;
            else if (key == "plane") ParseVmfPlane(val, tempFace.planePoints);
            else if (key == "uaxis") {
                // uaxis 的值是 "[x y z shift] scale"，ParseVmfAxis 只取方括号里的内容
                ParseVmfAxis(val, tempFace.uAxis);
            }
            else if (key == "vaxis") {
                ParseVmfAxis(val, tempFace.vAxis);
            }
            else if (key == "rotation") tempFace.rotation = (float)std::atof(val.c_str());
            else if (key == "lightmapscale") tempFace.lightmapScale = (float)std::atof(val.c_str());
            else if (key == "smoothing_groups") tempFace.smoothingGroups = std::atoi(val.c_str());
        } else if (inSolid && !inSide) {
            if (key == "id") tempSolid.id = std::atoi(val.c_str());
        } else if (inEntity) {
            if (key == "id") {
                tempEntity.id = std::atoi(val.c_str());
            } else if (key == "classname") {
                tempEntity.classname = val;
            } else if (key == "origin") {
                ParseVmfVector(val, tempEntity.origin);
            } else {
                VmfKeyValuePair kv;
                kv.key = key;
                kv.value = val;
                tempEntity.properties.push_back(kv);
            }
        }
    }

    file.close();
    return true;
}