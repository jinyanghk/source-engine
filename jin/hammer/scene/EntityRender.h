#ifndef SCENE_ENTITY_RENDER_H
#define SCENE_ENTITY_RENDER_H

#include "scene/Entity.h"
#include "materialsystem/imaterialsystem.h"

void BuildRotationMatrixFromQAngle(const QAngle& angles, VMatrix& outMatrix);
void UpdateEntityBBox(CEntity& ent);
void RenderEntityModel(CEntity& ent, IMatRenderContext *pRenderContext);

#endif // SCENE_ENTITY_RENDER_H