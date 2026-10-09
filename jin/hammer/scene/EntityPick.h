#ifndef SCENE_ENTITY_PICK_H
#define SCENE_ENTITY_PICK_H

#include "scene/Entity.h"

bool IsMouseOverEntityBBox(const CEntity& ent,
                           const Vector& camEye, const Vector& camLeft, const Vector& camUp, const Vector& camForward,
                           float tanHalfFov, float aspect,
                           int screenW, int screenH,
                           int mouseX, int mouseY);

#endif // SCENE_ENTITY_PICK_H