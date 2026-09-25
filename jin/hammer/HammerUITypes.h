#pragma once

#include <QString>
#include <QColor>
#include "mathlib/vector.h"

// Unified Edit Tool Enum matching traditional Valve Hammer operations
enum EditTool {
    TOOL_SELECT,
    TOOL_BLOCK,
    TOOL_ENTITY
};

struct MapBrush {
    int id;
    Vector mins;
    Vector maxs;
    QColor color;
};

// New structure tracking Point Entities positions and target classnames
struct MapEntity {
    int id;
    QString classname;
    Vector origin;
    QColor color;
};
