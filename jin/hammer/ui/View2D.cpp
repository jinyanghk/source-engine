#include "ui/View2D.h"

#include <cmath>
#include "imgui.h"
#include "scene/SceneGlobals.h"
#include "scene/Brush.h"
#include "scene/Entity.h"
#include "app/EditorState.h"
#include "mathlib/vector.h"

//-----------------------------------------------------------------------------
// 每个 2D 视图的相机状态
//-----------------------------------------------------------------------------
struct View2D
{
    float centerX = 0.0f;
    float centerY = 0.0f;
    float zoom    = 0.5f;

    int axisMapX  = 0;
    int axisMapY  = 1;
    int axisDepth = 2;

    // ★ 加构造函数
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
    float screenY = canvasPos.y + canvasSize.y * 0.5f - (wy - view.centerY) * view.zoom;   // Y 翻转
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
// 从 entity 提取 2D 位置
//-----------------------------------------------------------------------------
static void GetEntity2DPos(const CEntity& e, const View2D& view,
                           float& outX, float& outY)
{
    float pos[3] = { e.m_vecPos.x, e.m_vecPos.y, e.m_vecPos.z };
    outX = pos[view.axisMapX];
    outY = pos[view.axisMapY];
}

//-----------------------------------------------------------------------------
// 画网格
//-----------------------------------------------------------------------------
static void DrawGrid2D(ImDrawList* draw, const View2D& view,
                       const ImVec2& canvasPos, const ImVec2& canvasSize)
{
    if (g_flGridSize <= 0.0f) return;

    // 计算可见世界范围
    float worldMinX, worldMaxY;
    ScreenToWorld2D(canvasPos, view, canvasPos, canvasSize, worldMinX, worldMaxY);

    float worldMaxX, worldMinY;
    ScreenToWorld2D(ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                    view, canvasPos, canvasSize, worldMaxX, worldMinY);

    // 网格太密就不画
    float gridPixels = g_flGridSize * view.zoom;
    if (gridPixels < 4.0f) return;

    ImU32 gridColor = IM_COL32(60, 60, 60, 255);

    // 竖线
    float startX = floorf(worldMinX / g_flGridSize) * g_flGridSize;
    for (float x = startX; x <= worldMaxX; x += g_flGridSize)
    {
        ImVec2 p1 = WorldToScreen2D(x, worldMinY, view, canvasPos, canvasSize);
        ImVec2 p2 = WorldToScreen2D(x, worldMaxY, view, canvasPos, canvasSize);
        draw->AddLine(p1, p2, gridColor);
    }

    // 横线
    float startY = floorf(worldMinY / g_flGridSize) * g_flGridSize;
    for (float y = startY; y <= worldMaxY; y += g_flGridSize)
    {
        ImVec2 p1 = WorldToScreen2D(worldMinX, y, view, canvasPos, canvasSize);
        ImVec2 p2 = WorldToScreen2D(worldMaxX, y, view, canvasPos, canvasSize);
        draw->AddLine(p1, p2, gridColor);
    }

    // 原点十字（白色）
    ImVec2 origin = WorldToScreen2D(0, 0, view, canvasPos, canvasSize);
    draw->AddLine(ImVec2(origin.x - 12, origin.y), ImVec2(origin.x + 12, origin.y), IM_COL32(255, 255, 255, 128));
    draw->AddLine(ImVec2(origin.x, origin.y - 12), ImVec2(origin.x, origin.y + 12), IM_COL32(255, 255, 255, 128));
}

//-----------------------------------------------------------------------------
// 画所有 brush 的 2D 投影
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

        // 半透明填充 + 描边
        draw->AddRectFilled(p1, p2, IM_COL32(120, 120, 120, 80));
        draw->AddRect(p1, p2, color, 0.0f, 0, bSelected ? 2.0f : 1.0f);
    }
}

//-----------------------------------------------------------------------------
// 画所有 entity 的 2D 图标
//-----------------------------------------------------------------------------
static void DrawEntities2D(ImDrawList* draw, const View2D& view,
                           const ImVec2& canvasPos, const ImVec2& canvasSize)
{
    for (size_t i = 0; i < g_entities.size(); i++)
    {
        const CEntity& e = g_entities[i];

        float wx, wy;
        GetEntity2DPos(e, view, wx, wy);

        ImVec2 p = WorldToScreen2D(wx, wy, view, canvasPos, canvasSize);

        bool bSelected = ((int)i == g_iSelectedEntity);
        ImU32 color = bSelected ? IM_COL32(255, 255, 0, 255)
                                : IM_COL32(0, 200, 0, 255);

        // 圆 + 十字
        draw->AddCircle(p, 5.0f, color, 0, 1.5f);
        draw->AddLine(ImVec2(p.x - 8, p.y), ImVec2(p.x + 8, p.y), color);
        draw->AddLine(ImVec2(p.x, p.y - 8), ImVec2(p.x, p.y + 8), color);
    }
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

    // 3. Brush 投影
    DrawBrushes2D(draw, view, canvasPos, canvasSize);

    // 4. Entity 图标
    DrawEntities2D(draw, view, canvasPos, canvasSize);

    // 5. 交互
    if (ImGui::IsWindowHovered())
    {
        ImGuiIO& io = ImGui::GetIO();

        // 5a. 缩放（滚轮，以鼠标为中心）
        if (io.MouseWheel != 0.0f)
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

        // 5c. 点击拾取
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            float wx, wy;
            ScreenToWorld2D(io.MousePos, view, canvasPos, canvasSize, wx, wy);

            // 找包含 (wx, wy) 的 brush（倒序，选最上层）
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

            // 找 entity（只在没选中 brush 时才查）
            int hitEntity = -1;
            if (hitBrush < 0)
            {
                float pickRadius = 8.0f / view.zoom;
                for (int i = (int)g_entities.size() - 1; i >= 0; i--)
                {
                    float ex, ey;
                    GetEntity2DPos(g_entities[i], view, ex, ey);
                    float dx = wx - ex;
                    float dy = wy - ey;
                    if (dx * dx + dy * dy < pickRadius * pickRadius)
                    {
                        hitEntity = i;
                        break;
                    }
                }
            }

            if (hitBrush >= 0)
            {
                g_iSelectedBrush = hitBrush;
                g_iSelectedEntity = -1;
            }
            else if (hitEntity >= 0)
            {
                g_iSelectedEntity = hitEntity;
                g_iSelectedBrush = -1;
            }
            else
            {
                g_iSelectedBrush = -1;
                g_iSelectedEntity = -1;
            }
        }
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