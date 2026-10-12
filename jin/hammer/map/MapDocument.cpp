#include "MapDocument.h"
#include <algorithm>

// Update the constructor initializer lists to use the new counter names
MapDocument::MapDocument() : m_nextSolidID(1), m_nextEntityID(1) {}

MapDocument::~MapDocument() {
    Clear();
}

int MapDocument::CreateNewSolid(const Vector& mins, const Vector& maxs) {
    VmfSolid newSolid;
    newSolid.id = m_nextSolidID++;
    
    m_solids.push_back(newSolid);
    return newSolid.id;
}

void MapDocument::AddSolid(const VmfSolid& solid) {
    m_solids.push_back(solid);

    if (solid.id >= m_nextSolidID) {
        m_nextSolidID = solid.id + 1;
    }
}

bool MapDocument::DeleteSolid(int id) {
    auto it = std::remove_if(m_solids.begin(), m_solids.end(), [id](const VmfSolid& s) {
        return s.id == id;
    });
    if (it != m_solids.end()) {
        m_solids.erase(it, m_solids.end());
        return true;
    }
    return false;
}

bool MapDocument::TranslateSolid(int id, const Vector& delta3D) {
    for (auto& solid : m_solids) {
        if (solid.id == id) {
            return true;
        }
    }
    return false;
}

bool MapDocument::ResizeSolid(int id, const Vector& newMins, const Vector& newMaxs) {
    for (auto& solid : m_solids) {
        if (solid.id == id) {
            return true;
        }
    }
    return false;
}

int MapDocument::CreateNewEntity(const std::string& classname, const Vector& origin) {
    VmfEntity ent;
    ent.id = m_nextEntityID++;
    ent.classname = classname;
    
    ent.origin.x = origin.x;
    ent.origin.y = origin.y;
    ent.origin.z = origin.z;
    
    m_entities.push_back(ent);
    return ent.id;
}

const std::vector<VmfEntity>& MapDocument::GetEntities() const {
    return m_entities;
}

bool MapDocument::DeleteEntity(int id) {
    auto it = std::remove_if(m_entities.begin(), m_entities.end(), [id](const VmfEntity& e) {
        return e.id == id;
    });
    if (it != m_entities.end()) {
        m_entities.erase(it, m_entities.end());
        return true;
    }
    return false;
}

const std::vector<VmfSolid>& MapDocument::GetSolids() const {
    return m_solids;
}

void MapDocument::Clear() {
    m_solids.clear();
    m_entities.clear();
    // Reset both separate counter tracking states safely
    m_nextSolidID = 1;
    m_nextEntityID = 1;
}

void MapDocument::AddEntity(const VmfEntity& entity) {
    m_entities.push_back(entity);
    if (entity.id >= m_nextEntityID) {
        m_nextEntityID = entity.id + 1;
    }
}
