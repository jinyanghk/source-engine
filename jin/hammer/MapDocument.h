#ifndef MAP_DOCUMENT_H
#define MAP_DOCUMENT_H

#include "MapDataTypes.h"
#include "mathlib/vector.h"
#include <vector>

class MapDocument {
public:
    MapDocument();
    ~MapDocument();

    // Brush management
    int CreateNewSolid(const Vector& mins, const Vector& maxs);
    void AddSolid(const VmfSolid& solid);
    bool DeleteSolid(int id);
    const std::vector<VmfSolid>& GetSolids() const;
    bool TranslateSolid(int id, const Vector& delta3D);
    bool ResizeSolid(int id, const Vector& newMins, const Vector& newMaxs);
    
    // Entity management (NEW)
    int CreateNewEntity(const std::string& classname, const Vector& origin);
    const std::vector<VmfEntity>& GetEntities() const;
    bool DeleteEntity(int id);

    void Clear();

private:
    int m_nextSolidID;  // Separated counter names for clarity
    int m_nextEntityID; // Independent entity ID counter
    std::vector<VmfSolid> m_solids;
    std::vector<VmfEntity> m_entities; // Decoupled entity collection
};

#endif // MAP_DOCUMENT_H
