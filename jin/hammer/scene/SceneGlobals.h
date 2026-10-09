#ifndef SCENE_GLOBALS_H
#define SCENE_GLOBALS_H

#include <vector>
#include "scene/Entity.h"
#include "scene/Brush.h"

extern std::vector<CEntity> g_entities;
extern std::vector<CBrush>  g_brushes;
extern int                  g_iSelectedEntity;

#endif // SCENE_GLOBALS_H