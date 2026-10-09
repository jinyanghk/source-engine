#ifndef BOX_RENDER_H
#define BOX_RENDER_H

#include "scene/Entity.h"
#include "scene/Brush.h"

void DrawBrushBBox(const CBrush& brush, bool bSelected);
void DrawEntityBBox(const CEntity& ent, bool bSelected);

#endif // BOX_RENDER_H