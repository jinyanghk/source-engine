#ifndef ENTITY_PALETTE_H
#define ENTITY_PALETTE_H

#include <string>

//-----------------------------------------------------------------------------
// FGD 实体列表面板。列出所有 point classes。
// 用户点击某个 class 时，outSelectedClass 会被填入该 class 名；否则为空。
//-----------------------------------------------------------------------------
void DrawEntityPalette(std::string& outSelectedClass);

#endif