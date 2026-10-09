#include "app/EditorLayout.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace EditorLayout
{

static bool s_bLayoutInitialized = false;
static bool s_bResetRequested = false;

//-----------------------------------------------------------------------------
void Begin()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags host_flags = 0;
    host_flags |= ImGuiWindowFlags_NoTitleBar;
    host_flags |= ImGuiWindowFlags_NoCollapse;
    host_flags |= ImGuiWindowFlags_NoResize;
    host_flags |= ImGuiWindowFlags_NoMove;
    host_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    host_flags |= ImGuiWindowFlags_NoNavFocus;
    host_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("##DockHost", nullptr, host_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("EditorDockSpace");

    ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    // 首次运行或用户点了"恢复默认布局"时，用 DockBuilder 建立布局。
    // 注意：DockBuilder 建立布局时不调 ImGui::DockSpace，直接调 DockBuilder* 系列。
    // 建立完成后，下一帧的 ImGui::DockSpace 会自动用这个布局。
    if (!s_bLayoutInitialized || s_bResetRequested)
    {
        s_bLayoutInitialized = true;
        s_bResetRequested = false;

        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id,
                                  ImGuiDockNodeFlags_DockSpace | dock_flags);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

        ImGuiID dock_main = dockspace_id;
        ImGuiID dock_right = ImGui::DockBuilderSplitNode(
            dock_main, ImGuiDir_Right, 0.25f, nullptr, &dock_main);
        ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(
            dock_main, ImGuiDir_Down, 0.20f, nullptr, &dock_main);

        ImGui::DockBuilderDockWindow("Entities", dock_right);
        ImGui::DockBuilderDockWindow("Console", dock_bottom);

        ImGui::DockBuilderFinish(dockspace_id);
    }

    // 每帧都要提交 DockSpace。
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dock_flags);

    ImGui::End();
}

//-----------------------------------------------------------------------------
void End()
{
    // 目前不需要做任何事情。所有面板在 Begin/End 之间由调用者绘制。
}

//-----------------------------------------------------------------------------
void ResetDefaultLayout()
{
    s_bResetRequested = true;
}

} // namespace EditorLayout