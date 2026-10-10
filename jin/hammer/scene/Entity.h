#ifndef SCENE_ENTITY_H
#define SCENE_ENTITY_H

#include <string>

#include "mathlib/vector.h"
#include "datacache/imdlcache.h"

enum EntityType { ENTITY_MODEL = 0, ENTITY_PLAYER_START = 1 };

struct CEntity
{
    EntityType   m_iType;
    MDLHandle_t  m_hMdl;
    std::string  m_szName;
    Vector       m_vecPos;
    QAngle       m_angRot;
    Vector       m_vecBBoxMins;
    Vector       m_vecBBoxMaxs;

    CEntity()
        : m_iType(ENTITY_MODEL)
        , m_hMdl(MDLHANDLE_INVALID)
        , m_szName("")
        , m_vecPos(0, 0, 0)
        , m_angRot(0, 0, 0)
        , m_vecBBoxMins(-16, -16, 0)
        , m_vecBBoxMaxs(16, 16, 72)
    {}

    Vector GetCenter() const
    {
        return Vector(
            (m_vecBBoxMins.x + m_vecBBoxMaxs.x) * 0.5f,
            (m_vecBBoxMins.y + m_vecBBoxMaxs.y) * 0.5f,
            (m_vecBBoxMins.z + m_vecBBoxMaxs.z) * 0.5f
        );
    }
};

#endif // SCENE_ENTITY_H