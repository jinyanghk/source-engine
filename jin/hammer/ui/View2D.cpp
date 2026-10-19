#include "ui/View2D.h"

#include <cmath>
#include <vector>
#include <algorithm>
#include "imgui.h"
#include "scene/SceneGlobals.h"
#include "scene/Brush.h"
#include "scene/Entity.h"
#include "app/EditorState.h"
#include "app/UndoStack.h"
#include "mathlib/vector.h"

//-----------------------------------------------------------------------------
// 每个 2D 视图的相机状态 + 拖动状态
//-----------------------------------------------------------------------------
struct View2D
{
    float centerX = 0.0f;
    float centerY = 0.0f;
    float zoom    = 0.5f;

    int axisMapX  = 0;
    int axisMapY  = 1;
    int axisDepth = 2;

    // 拖动状态：brush
    bool   bDraggingBrush       = false;
    int    iDraggingBrush       = -1;
    float  fDragStartWorldX     = 0.0f;
    float  fDragStartWorldY     = 0.0f;
    Vector vecDragStartBrushPos;

    // 拖动状态：entity
    bool   bDraggingEntity      = false;
    int    iDraggingEntity      = -1;
    Vector vecDragStartEntityPos;

    // 手柄缩放状态
    bool   bResizingBrush       = false;
    int    iResizingBrush       = -1;
    int    iResizeHandle        = -1;   // 0-7：左上、右上、右下、左下、上中、右中、下中、左中
    // 固定边界的"世界坐标"（8 个手柄对应 4 条边界的组合）
    float  fResizeFixedMinX     = 0.0f;   // 固定的"左"边界
    float  fResizeFixedMaxX     = 0.0f;   // 固定的"右"边界
    float  fResizeFixedMinY     = 0.0f;   // 固定的"下"边界
    float  fResizeFixedMaxY     = 0.0f;   // 固定的"上"边界

    // 画框创建状态
    bool   bBoxCreating       = false;
    float  fBoxCreateStartX   = 0.0f;
    float  fBoxCreateStartY   = 0.0f;
    float  fBoxCreateEndX     = 0.0f;
    float  fBoxCreateEndY     = 0.0f;

    // 标记"本次拖动是否已经 push 过快照"
    bool   bPushedDragSnapshot  = false;

    View2D() = default;
    View2D(float cx, float cy, float z, int ax, int ay, int ad)
        : centerX(cx), centerY(cy), zoom(z)
        , axisMapX(ax), axisMapY(ay), axisDepth(ad)
    {}
};

static View2D s_topView(   0, 0, 0.5f, 0, 1, 2);   // Top:   X→X, Y→Y, 深度 Z
static View2D s_frontView( 0, 0, 0.5f, 0, 2, 1);   // Front: X→X, Y→Z, 深度 Y
static View2D s_sideView(  0, 0, 0.5f, 1, 2, 0);   // Side:  X→Y, Y→Z, 深度 X

//-----------------------------------------------------------------------------
// 世界坐标 → 屏幕坐标（ImGui 窗口内）
//-----------------------------------------------------------------------------
static ImVec2 WorldToScreen2D(float wx, float wy, const View2D& view,
                               const ImVec2& canvasPos, const ImVec2& canvasSize)
{
    float screenX = canvasPos.x + canvasSize.x * 0.5f + (wx - view.centerX) * view.zoom;
    float screenY = canvasPos.y + canvasSize.y * 0.5f - (wy - view.centerY) * view.zoom;
    return ImVec2(screenX, screenY);
}

//-----------------------------------------------------------------------------
// 屏幕坐标 → 世界坐标
//-----------------------------------------------------------------------------
static void ScreenToWorld2D(const ImVec2& screen, const View2D& view,
                            const ImVec2& canvasPos, const ImVec2& canvasSize,
                            float& outWX, float& outWY)
{
    outWX = view.centerX + (screen.x - canvasPos.x - canvasSize.x * 0.5f) / view.zoom;
    outWY = view.centerY - (screen.y - canvasPos.y - canvasSize.y * 0.5f) / view.zoom;
}

//-----------------------------------------------------------------------------
// 从 brush 提取 2D 投影的 X/Y 范围
//-----------------------------------------------------------------------------
static void GetBrush2DRange(const CBrush& b, const View2D& view,
                            float& outMinX, float& outMaxX,
                            float& outMinY, float& outMaxY)
{
    float mins[3] = { b.m_vecPos.x - b.m_vecSize.x,
                      b.m_vecPos.y - b.m_vecSize.y,
                      b.m_vecPos.z - b.m_vecSize.z };
    float maxs[3] = { b.m_vecPos.x + b.m_vecSize.x,
                      b.m_vecPos.y + b.m_vecSize.y,
                      b.m_vecPos.z + b.m_vecSize.z };

    outMinX = mins[view.axisMapX];
    outMaxX = maxs[view.axisMapX];
    outMinY = mins[view.axisMapY];
    outMaxY = maxs[view.axisMapY];
}

//-----------------------------------------------------------------------------
// 从 entity 的 bbox 提取 2D 投影的 X/Y 范围
//-----------------------------------------------------------------------------
static void GetEntity2DRange(const CEntity& e, const View2D& view,
                             float& outMinX, float& outMaxX,
                             float& outMinY, float& outMaxY)
{
    float mins[3] = { e.m_vecBBoxMins.x, e.m_vecBBoxMins.y, e.m_vecBBoxMins.z };
    float maxs[3] = { e.m_vecBBoxMaxs.x, e.m_vecBBoxMaxs.y, e.m_vecBBoxMaxs.z };

    outMinX = mins[view.axisMapX];
    outMaxX = maxs[view.axisMapX];
    outMinY = mins[view.axisMapY];
    outMaxY = maxs[view.axisMapY];
}

//-----------------------------------------------------------------------------
// 从 entity 提取 2D 位置（用于画圆圈 + 十字）
//-----------------------------------------------------------------------------
static void GetEntity2DPos(const CEntity& e, const View2D& view,
                           float& outX, float& outY)
{
    float pos[3] = { e.m_vecPos.x, e.m_vecPos.y, e.m_vecPos.z };
    outX = pos[view.axisMapX];
    outY = pos[view.axisMapY];
}

//-----------------------------------------------------------------------------
// 按 axisMap 设置/读取 Vector 的某个轴
//-----------------------------------------------------------------------------
static void SetVectorAxis(Vector& v, int axis, float value)
{
    if (axis == 0)      v.x = value;
    else if (axis == 1) v.y = value;
    else                v.z = value;
}

static float GetVectorAxis(const Vector& v, int axis)
{
    if (axis == 0) return v.x;
    if (axis == 1) return v.y;
    return v.z;
}

//-----------------------------------------------------------------------------
// 8 个手柄的"世界坐标"（顺序：左上、右上、右下、左下、上中、右中、下中、左中）
//-----------------------------------------------------------------------------
static void GetBrushHandles(const CBrush& b, const View2D& view,
                            float outHandles[8][2])
{
    float minX, maxX, minY, maxY;
    GetBrush2DRange(b, view, minX, maxX, minY, maxY);
    float midX = (minX + maxX) * 0.5f;
    float midY = (minY + maxY) * 0.5f;

    outHandles[0][0] = minX; outHandles[0][1] = maxY;   // 左上
    outHandles[1][0] = maxX; outHandles[1][1] = maxY;   // 右上
    outHandles[2][0] = maxX; outHandles[2][1] = minY;   // 右下
    outHandles[3][0] = minX; outHandles[3][1] = minY;   // 左下
    outHandles[4][0] = midX; outHandles[4][1] = maxY;   // 上中
    outHandles[5][0] = maxX; outHandles[5][1] = midY;   // 右中
    outHandles[6][0] = midX; outHandles[6][1] = minY;   // 下中
    outHandles[7][0] = minX; outHandles[7][1] = midY;   // 左中
}

//-----------------------------------------------------------------------------
// 画网格
//-----------------------------------------------------------------------------
static void DrawGrid2D(ImDrawList* draw, const View2D& view,
                       const ImVec2& canvasPos, const ImVec2& canvasSize)
{
    if (g_flGridSize <= 0.0f) return;

    float worldMinX, worldMaxY;
    ScreenToWorld2D(canvasPos, view, canvasPos, canvasSize, worldMinX, worldMaxY);

    float worldMaxX, worldMinY;
    ScreenToWorld2D(ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                    view, canvasPos, canvasSize, worldMaxX, worldMinY);

    float gridPixels = g_flGridSize * view.zoom;
    if (gridPixels < 4.0f) return;

    ImU32 gridColor = IM_COL32(60, 60, 60, 255);

    float startX = floorf(worldMinX / g_flGridSize) * g_flGridSize;
    for (float x = startX; x <= worldMaxX; x += g_flGridSize)
    {
        ImVec2 p1 = WorldToScreen2D(x, worldMinY, view, canvasPos, canvasSize);
        ImVec2 p2 = WorldToScreen2D(x, worldMaxY, view, canvasPos, canvasSize);
        draw->AddLine(p1, p2, gridColor);
    }

    float startY = floorf(worldMinY / g_flGridSize) * g_flGridSize;
    for (float y = startY; y <= worldMaxY; y += g_flGridSize)
    {
        ImVec2 p1 = WorldToScreen2D(worldMinX, y, view, canvasPos, canvasSize);
        ImVec2 p2 = WorldToScreen2D(worldMaxX, y, view, canvasPos, canvasSize);
        draw->AddLine(p1, p2, gridColor);
    }

    // 原点十字
    ImVec2 origin = WorldToScreen2D(0, 0, view, canvasPos, canvasSize);
    draw->AddLine(ImVec2(origin.x - 12, origin.y), ImVec2(origin.x + 12, origin.y),
                  IM_COL32(255, 255, 255, 128));
    draw->AddLine(ImVec2(origin.x, origin.y - 12), ImVec2(origin.x, origin.y + 12),
                  IM_COL32(255, 255, 255, 128));
}

//-----------------------------------------------------------------------------
// 画所有 brush 的 2D 投影 + 选中 brush 的 8 个手柄
//-----------------------------------------------------------------------------
static void DrawBrushes2D(ImDrawList* draw, const View2D& view,
                          const ImVec2& canvasPos, const ImVec2& canvasSize)
{
    for (size_t i = 0; i < g_brushes.size(); i++)
    {
        const CBrush& b = g_brushes[i];

        float minX, maxX, minY, maxY;
        GetBrush2DRange(b, view, minX, maxX, minY, maxY);

        ImVec2 p1 = WorldToScreen2D(minX, minY, view, canvasPos, canvasSize);
        ImVec2 p2 = WorldToScreen2D(maxX, maxY, view, canvasPos, canvasSize);

        bool bSelected = ((int)i == g_iSelectedBrush);
        ImU32 color = bSelected ? IM_COL32(255, 255, 0, 255)
                                : IM_COL32(200, 200, 200, 255);

        draw->AddRectFilled(p1, p2, IM_COL32(120, 120, 120, 80));
        draw->AddRect(p1, p2, color, 0.0f, 0, bSelected ? 2.0f : 1.0f);
    }

    // 画"选中 brush"的 8 个手柄
    if (g_iSelectedBrush >= 0 && g_iSelectedBrush < (int)g_brushes.size())
    {
        float handles[8][2];
        GetBrushHandles(g_brushes[g_iSelectedBrush], view, handles);

        const float HANDLE_HALF = 4.0f;   // 手柄半边长（像素）

        for (int h = 0; h < 8; h++)
        {
            ImVec2 p = WorldToScreen2D(handles[h][0], handles[h][1], view, canvasPos, canvasSize);

            // hover 或"正在拖"时高亮
            bool bHighlighted = (view.bResizingBrush && view.iResizeHandle == h);

            ImU32 fillColor   = bHighlighted ? IM_COL32(255, 200, 0, 255)
                                             : IM_COL32(255, 255, 0, 255);
            ImU32 borderColor = IM_COL32(0, 0, 0, 255);

            ImVec2 hMin(p.x - HANDLE_HALF, p.y - HANDLE_HALF);
            ImVec2 hMax(p.x + HANDLE_HALF, p.y + HANDLE_HALF);

            draw->AddRectFilled(hMin, hMax, fillColor);
            draw->AddRect(hMin, hMax, borderColor);
        }
    }
}

//-----------------------------------------------------------------------------
// 画所有 entity 的 2D 图标（bbox 矩形 + 圆圈 + 十字）
//-----------------------------------------------------------------------------
static void DrawEntities2D(ImDrawList* draw, const View2D& view,
                           const ImVec2& canvasPos, const ImVec2& canvasSize)
{
    for (size_t i = 0; i < g_entities.size(); i++)
    {
        const CEntity& e = g_entities[i];

        bool bSelected = ((int)i == g_iSelectedEntity);
        ImU32 color = bSelected ? IM_COL32(255, 255, 0, 255)
                                : IM_COL32(0, 200, 0, 255);

        float minX, maxX, minY, maxY;
        GetEntity2DRange(e, view, minX, maxX, minY, maxY);

        ImVec2 p1 = WorldToScreen2D(minX, minY, view, canvasPos, canvasSize);
        ImVec2 p2 = WorldToScreen2D(maxX, maxY, view, canvasPos, canvasSize);

        draw->AddRect(p1, p2, color, 0.0f, 0, bSelected ? 2.0f : 1.0f);

        // 圆圈 + 十字（位置中心）
        float wx, wy;
        GetEntity2DPos(e, view, wx, wy);
        ImVec2 p = WorldToScreen2D(wx, wy, view, canvasPos, canvasSize);

        draw->AddCircle(p, 5.0f, color, 0, 1.5f);
        draw->AddLine(ImVec2(p.x - 8, p.y), ImVec2(p.x + 8, p.y), color);
        draw->AddLine(ImVec2(p.x, p.y - 8), ImVec2(p.x, p.y + 8), color);
    }
}

//-----------------------------------------------------------------------------
// 检测"鼠标是否点中 8 个手柄之一"
// 返回 -1 表示没点中，否则返回手柄索引 0-7
//-----------------------------------------------------------------------------
static int HitTestHandles(const CBrush& b, const View2D& view,
                          const ImVec2& mousePos, const ImVec2& canvasPos, const ImVec2& canvasSize)
{
    float handles[8][2];
    GetBrushHandles(b, view, handles);

    const float HANDLE_HALF = 6.0f;   // 点击容差稍大（像素）

    for (int h = 0; h < 8; h++)
    {
        ImVec2 p = WorldToScreen2D(handles[h][0], handles[h][1], view, canvasPos, canvasSize);

        if (mousePos.x >= p.x - HANDLE_HALF && mousePos.x <= p.x + HANDLE_HALF &&
            mousePos.y >= p.y - HANDLE_HALF && mousePos.y <= p.y + HANDLE_HALF)
        {
            return h;
        }
    }

    return -1;
}

//-----------------------------------------------------------------------------
// 开始手柄缩放
//-----------------------------------------------------------------------------
static void StartResize(View2D& view, int brushIdx, int handle)
{
    view.bResizingBrush = true;
    view.iResizingBrush = brushIdx;
    view.iResizeHandle  = handle;
    view.bPushedDragSnapshot = false;

    float minX, maxX, minY, maxY;
    GetBrush2DRange(g_brushes[brushIdx], view, minX, maxX, minY, maxY);

    // 保存"固定边界"（"手柄"的"对侧"）
    view.fResizeFixedMinX = minX;
    view.fResizeFixedMaxX = maxX;
    view.fResizeFixedMinY = minY;
    view.fResizeFixedMaxY = maxY;
}

//-----------------------------------------------------------------------------
// 处理手柄缩放（"拖动中"）
//-----------------------------------------------------------------------------
static void UpdateResize(View2D& view, ImGuiIO& io,
                         const ImVec2& canvasPos, const ImVec2& canvasSize)
{
    if (!view.bResizingBrush || view.iResizingBrush < 0 || view.iResizeHandle < 0)
        return;

    float wx, wy;
    ScreenToWorld2D(io.MousePos, view, canvasPos, canvasSize, wx, wy);

    // 首次有实际位移时 push Undo
    if (!view.bPushedDragSnapshot)
    {
        UndoStack::Instance().PushSnapshot("2D Resize Brush");
        view.bPushedDragSnapshot = true;
    }

    CBrush& brush = g_brushes[view.iResizingBrush];

    float newMinX = view.fResizeFixedMinX;
    float newMaxX = view.fResizeFixedMaxX;
    float newMinY = view.fResizeFixedMinY;
    float newMaxY = view.fResizeFixedMaxY;

    int h = view.iResizeHandle;

    // 根据手柄更新对应边界
    switch (h)
    {
    case 0:   // 左上角：动 minX, maxY
        newMinX = wx; newMaxY = wy;
        break;
    case 1:   // 右上角：动 maxX, maxY
        newMaxX = wx; newMaxY = wy;
        break;
    case 2:   // 右下角：动 maxX, minY
        newMaxX = wx; newMinY = wy;
        break;
    case 3:   // 左下角：动 minX, minY
        newMinX = wx; newMinY = wy;
        break;
    case 4:   // 上中：动 maxY
        newMaxY = wy;
        break;
    case 5:   // 右中：动 maxX
        newMaxX = wx;
        break;
    case 6:   // 下中：动 minY
        newMinY = wy;
        break;
    case 7:   // 左中：动 minX
        newMinX = wx;
        break;
    }

    // Grid Snap
    if (g_flGridSize > 0.0f)
    {
        if (h == 0 || h == 3 || h == 7) newMinX = roundf(newMinX / g_flGridSize) * g_flGridSize;
        if (h == 1 || h == 2 || h == 5) newMaxX = roundf(newMaxX / g_flGridSize) * g_flGridSize;
        if (h == 0 || h == 1 || h == 4) newMaxY = roundf(newMaxY / g_flGridSize) * g_flGridSize;
        if (h == 2 || h == 3 || h == 6) newMinY = roundf(newMinY / g_flGridSize) * g_flGridSize;
    }

    // 检查尺寸
    if (newMaxX - newMinX < 1.0f || newMaxY - newMinY < 1.0f)
        return;

    // 计算新的中心/半尺寸（沿着两个轴）
    float newCenterX = (newMinX + newMaxX) * 0.5f;
    float newHalfX   = (newMaxX - newMinX) * 0.5f;
    float newCenterY = (newMinY + newMaxY) * 0.5f;
    float newHalfY   = (newMaxY - newMinY) * 0.5f;

    Vector oldPos = brush.m_vecPos;
    Vector oldSize = brush.m_vecSize;

    // 设置（axisMapX / axisMapY）
    SetVectorAxis(brush.m_vecPos, view.axisMapX, newCenterX);
    SetVectorAxis(brush.m_vecSize, view.axisMapX, newHalfX);
    SetVectorAxis(brush.m_vecPos, view.axisMapY, newCenterY);
    SetVectorAxis(brush.m_vecSize, view.axisMapY, newHalfY);

    // 同步 m_vertices
    Vector delta = brush.m_vecPos - oldPos;
    Vector scale;
    scale.x = (oldSize.x > 0.0001f) ? (brush.m_vecSize.x / oldSize.x) : 1.0f;
    scale.y = (oldSize.y > 0.0001f) ? (brush.m_vecSize.y / oldSize.y) : 1.0f;
    scale.z = (oldSize.z > 0.0001f) ? (brush.m_vecSize.z / oldSize.z) : 1.0f;

    for (auto& v : brush.m_vertices)
    {
        Vector local = v - oldPos;
        local.x *= scale.x;
        local.y *= scale.y;
        local.z *= scale.z;
        v = oldPos + local + delta;
    }

    brush.RecalcFaceNormals();
}

static void CreateBrushFromBox(View2D& view)
{
    float minX = std::min(view.fBoxCreateStartX, view.fBoxCreateEndX);
    float maxX = std::max(view.fBoxCreateStartX, view.fBoxCreateEndX);
    float minY = std::min(view.fBoxCreateStartY, view.fBoxCreateEndY);
    float maxY = std::max(view.fBoxCreateStartY, view.fBoxCreateEndY);

    // 太小，忽略（用户"点击"而不是"拖动"）
    if ((maxX - minX) < 1.0f || (maxY - minY) < 1.0f)
        return;

    // Grid Snap
    if (g_flGridSize > 0.0f)
    {
        minX = roundf(minX / g_flGridSize) * g_flGridSize;
        maxX = roundf(maxX / g_flGridSize) * g_flGridSize;
        minY = roundf(minY / g_flGridSize) * g_flGridSize;
        maxY = roundf(maxY / g_flGridSize) * g_flGridSize;
    }

    // "第三轴"的"深度"：用"当前选中的 brush"的深度；没有的话，默认 16 单位厚
    float depthMin = -8.0f;
    float depthMax =  8.0f;

    if (g_iSelectedBrush >= 0 && g_iSelectedBrush < (int)g_brushes.size())
    {
        const CBrush& sel = g_brushes[g_iSelectedBrush];
        float selMin[3] = { sel.m_vecPos.x - sel.m_vecSize.x,
                            sel.m_vecPos.y - sel.m_vecSize.y,
                            sel.m_vecPos.z - sel.m_vecSize.z };
        float selMax[3] = { sel.m_vecPos.x + sel.m_vecSize.x,
                            sel.m_vecPos.y + sel.m_vecSize.y,
                            sel.m_vecPos.z + sel.m_vecSize.z };
        depthMin = selMin[view.axisDepth];
        depthMax = selMax[view.axisDepth];
    }

    // 计算"三维"的 min/max
    float minCoords[3], maxCoords[3];
    minCoords[view.axisMapX] = minX;
    maxCoords[view.axisMapX] = maxX;
    minCoords[view.axisMapY] = minY;
    maxCoords[view.axisMapY] = maxY;
    minCoords[view.axisDepth] = depthMin;
    maxCoords[view.axisDepth] = depthMax;

    // 创建 brush
    CBrush newBrush;
    newBrush.m_vecPos  = Vector(
        (minCoords[0] + maxCoords[0]) * 0.5f,
        (minCoords[1] + maxCoords[1]) * 0.5f,
        (minCoords[2] + maxCoords[2]) * 0.5f
    );
    newBrush.m_vecSize = Vector(
        (maxCoords[0] - minCoords[0]) * 0.5f,
        (maxCoords[1] - minCoords[1]) * 0.5f,
        (maxCoords[2] - minCoords[2]) * 0.5f
    );
    newBrush.m_szMaterial = "DEV/DEV_MEASUREGENERIC01";
    newBrush.RebuildPrimitiveMesh();

    // Push Undo
    UndoStack::Instance().PushSnapshot("Create Brush (2D Block)");

    // 添加到 g_brushes
    g_brushes.push_back(newBrush);
    g_iSelectedBrush  = (int)g_brushes.size() - 1;
    g_iSelectedEntity = -1;
}

//-----------------------------------------------------------------------------
// 拖动处理
//-----------------------------------------------------------------------------
static bool HandleDrag(View2D& view, ImGuiIO& io,
                       const ImVec2& canvasPos, const ImVec2& canvasSize)
{
    // --- 1. 左键按下：拾取 + 开始拖动 / 开始缩放 ---
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        float wx, wy;
        ScreenToWorld2D(io.MousePos, view, canvasPos, canvasSize, wx, wy);

        // ★ Block Tool 模式：左键拖动 = 画框创建
        if (g_bBlockToolActive)
        {
            view.bBoxCreating     = true;
            view.fBoxCreateStartX = wx;
            view.fBoxCreateStartY = wy;
            view.fBoxCreateEndX   = wx;
            view.fBoxCreateEndY   = wy;
            return true;
        }

        // ★ 优先：如果"当前选中的 brush"的"手柄"被点中 → 边缘缩放
        if (g_iSelectedBrush >= 0 && g_iSelectedBrush < (int)g_brushes.size())
        {
            int handle = HitTestHandles(g_brushes[g_iSelectedBrush], view, io.MousePos, canvasPos, canvasSize);
            if (handle >= 0)
            {
                StartResize(view, g_iSelectedBrush, handle);
                return true;
            }
        }

        // 先查 entity
        int hitEntity = -1;
        for (int i = (int)g_entities.size() - 1; i >= 0; i--)
        {
            float minX, maxX, minY, maxY;
            GetEntity2DRange(g_entities[i], view, minX, maxX, minY, maxY);
            if (wx >= minX && wx <= maxX && wy >= minY && wy <= maxY)
            {
                hitEntity = i;
                break;
            }
        }

        if (hitEntity >= 0)
        {
            g_iSelectedEntity = hitEntity;
            g_iSelectedBrush  = -1;

            view.bDraggingEntity       = true;
            view.iDraggingEntity       = hitEntity;
            view.fDragStartWorldX      = wx;
            view.fDragStartWorldY      = wy;
            view.vecDragStartEntityPos = g_entities[hitEntity].m_vecPos;
            view.bPushedDragSnapshot   = false;
            return true;
        }

        // 再查 brush
        int hitBrush = -1;
        for (int i = (int)g_brushes.size() - 1; i >= 0; i--)
        {
            float minX, maxX, minY, maxY;
            GetBrush2DRange(g_brushes[i], view, minX, maxX, minY, maxY);
            if (wx >= minX && wx <= maxX && wy >= minY && wy <= maxY)
            {
                hitBrush = i;
                break;
            }
        }

        if (hitBrush >= 0)
        {
            g_iSelectedBrush  = hitBrush;
            g_iSelectedEntity = -1;

            view.bDraggingBrush       = true;
            view.iDraggingBrush       = hitBrush;
            view.fDragStartWorldX     = wx;
            view.fDragStartWorldY     = wy;
            view.vecDragStartBrushPos = g_brushes[hitBrush].m_vecPos;
            view.bPushedDragSnapshot  = false;
            return true;
        }

        // 都没点中
        g_iSelectedBrush  = -1;
        g_iSelectedEntity = -1;
    }

    // --- 2. 拖动中：手柄缩放 ---
    if (view.bResizingBrush && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        UpdateResize(view, io, canvasPos, canvasSize);
        return true;
    }

    // --- 3. 拖动中：brush / entity 移动 ---
    if (view.bBoxCreating && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        float wx, wy;
        ScreenToWorld2D(io.MousePos, view, canvasPos, canvasSize, wx, wy);
        view.fBoxCreateEndX = wx;
        view.fBoxCreateEndY = wy;
        return true;
    }

    bool bAnyDragging = view.bDraggingBrush || view.bDraggingEntity;
    if (bAnyDragging && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        float wx, wy;
        ScreenToWorld2D(io.MousePos, view, canvasPos, canvasSize, wx, wy);

        float deltaX = wx - view.fDragStartWorldX;
        float deltaY = wy - view.fDragStartWorldY;

        if (!view.bPushedDragSnapshot && (fabsf(deltaX) > 0.01f || fabsf(deltaY) > 0.01f))
        {
            UndoStack::Instance().PushSnapshot(
                view.bDraggingBrush ? "2D Drag Brush" : "2D Drag Entity");
            view.bPushedDragSnapshot = true;
        }

        if (view.bDraggingBrush)
        {
            CBrush& brush = g_brushes[view.iDraggingBrush];

            Vector newPos = view.vecDragStartBrushPos;
            SetVectorAxis(newPos, view.axisMapX,
                GetVectorAxis(view.vecDragStartBrushPos, view.axisMapX) + deltaX);
            SetVectorAxis(newPos, view.axisMapY,
                GetVectorAxis(view.vecDragStartBrushPos, view.axisMapY) + deltaY);

            if (g_flGridSize > 0.0f)
            {
                SetVectorAxis(newPos, view.axisMapX,
                    roundf(GetVectorAxis(newPos, view.axisMapX) / g_flGridSize) * g_flGridSize);
                SetVectorAxis(newPos, view.axisMapY,
                    roundf(GetVectorAxis(newPos, view.axisMapY) / g_flGridSize) * g_flGridSize);
            }

            Vector offset = newPos - brush.m_vecPos;
            for (auto& v : brush.m_vertices)
                v += offset;
            brush.m_vecPos = newPos;
        }
        else if (view.bDraggingEntity)
        {
            Vector newPos = view.vecDragStartEntityPos;
            SetVectorAxis(newPos, view.axisMapX,
                GetVectorAxis(view.vecDragStartEntityPos, view.axisMapX) + deltaX);
            SetVectorAxis(newPos, view.axisMapY,
                GetVectorAxis(view.vecDragStartEntityPos, view.axisMapY) + deltaY);

            if (g_flGridSize > 0.0f)
            {
                SetVectorAxis(newPos, view.axisMapX,
                    roundf(GetVectorAxis(newPos, view.axisMapX) / g_flGridSize) * g_flGridSize);
                SetVectorAxis(newPos, view.axisMapY,
                    roundf(GetVectorAxis(newPos, view.axisMapY) / g_flGridSize) * g_flGridSize);
            }

            g_entities[view.iDraggingEntity].m_vecPos = newPos;
        }
    }

    // --- 4. 左键松开：结束拖动 / 缩放 ---
    if (view.bBoxCreating && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        // 创建 brush
        CreateBrushFromBox(view);
        view.bBoxCreating = false;
        return true;
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        view.bDraggingBrush      = false;
        view.iDraggingBrush      = -1;
        view.bDraggingEntity     = false;
        view.iDraggingEntity     = -1;
        view.bResizingBrush      = false;
        view.iResizingBrush      = -1;
        view.iResizeHandle       = -1;
        view.bPushedDragSnapshot = false;
    }

    return bAnyDragging;
}

//-----------------------------------------------------------------------------
// 通用 2D 视图渲染
//-----------------------------------------------------------------------------
static void Draw2DViewCommon(const char* name, View2D& view)
{
    if (!ImGui::Begin(name))
    {
        ImGui::End();
        return;
    }

    ImVec2 canvasPos  = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();

    if (canvasSize.x < 1.0f || canvasSize.y < 1.0f)
    {
        ImGui::End();
        return;
    }

    ImDrawList* draw = ImGui::GetWindowDrawList();

    // 1. 背景
    draw->AddRectFilled(canvasPos,
                        ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                        IM_COL32(25, 25, 25, 255));

    // 2. 网格
    DrawGrid2D(draw, view, canvasPos, canvasSize);

    // 3. Brush 投影 + 手柄
    DrawBrushes2D(draw, view, canvasPos, canvasSize);

    // 4. Entity 图标
    DrawEntities2D(draw, view, canvasPos, canvasSize);

    if (view.bBoxCreating)
    {
        ImVec2 p1 = WorldToScreen2D(view.fBoxCreateStartX, view.fBoxCreateStartY, view, canvasPos, canvasSize);
        ImVec2 p2 = WorldToScreen2D(view.fBoxCreateEndX, view.fBoxCreateEndY, view, canvasPos, canvasSize);

        ImVec2 minP = ImVec2(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
        ImVec2 maxP = ImVec2(std::max(p1.x, p2.x), std::max(p1.y, p2.y));

        draw->AddRectFilled(minP, maxP, IM_COL32(100, 150, 255, 60));
        draw->AddRect(minP, maxP, IM_COL32(100, 150, 255, 255), 0.0f, 0, 2.0f);
    }

    // 5. 交互
    if (ImGui::IsWindowHovered())
    {
        ImGuiIO& io = ImGui::GetIO();

        // 5a. 缩放（滚轮，以鼠标为中心；拖动中禁用）
        if (io.MouseWheel != 0.0f && !view.bDraggingBrush && !view.bDraggingEntity && !view.bResizingBrush && !view.bBoxCreating)
        {
            float wxBefore, wyBefore;
            ScreenToWorld2D(io.MousePos, view, canvasPos, canvasSize, wxBefore, wyBefore);

            view.zoom *= (1.0f + io.MouseWheel * 0.15f);
            if (view.zoom < 0.01f) view.zoom = 0.01f;
            if (view.zoom > 20.0f) view.zoom = 20.0f;

            float wxAfter, wyAfter;
            ScreenToWorld2D(io.MousePos, view, canvasPos, canvasSize, wxAfter, wyAfter);

            view.centerX += wxBefore - wxAfter;
            view.centerY += wyBefore - wyAfter;
        }

        // 5b. 平移（中键拖拽）
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
        {
            view.centerX -= io.MouseDelta.x / view.zoom;
            view.centerY += io.MouseDelta.y / view.zoom;
        }

        // 5c. 拖动 / 缩放
        HandleDrag(view, io, canvasPos, canvasSize);
    }

    ImGui::End();
}

//-----------------------------------------------------------------------------
// 三个 2D 视图
//-----------------------------------------------------------------------------
void DrawTopView()   { Draw2DViewCommon("Top View",   s_topView);   }
void DrawFrontView() { Draw2DViewCommon("Front View", s_frontView); }
void DrawSideView()  { Draw2DViewCommon("Side View",  s_sideView);  }

//-----------------------------------------------------------------------------
void Draw2DViews()
{
    DrawTopView();
    DrawFrontView();
    DrawSideView();
}