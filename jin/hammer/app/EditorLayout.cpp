#include "app/EditorLayout.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace EditorLayout
{

// 首次运行 / 用户点"恢复默认布局"后，需要重新建立默认布局。
static bool s_bLayoutInitialized = false;
static bool s_bResetRequested = false;

//-----------------------------------------------------------------------------
void Begin()
{
    // 全屏宿住窗口，覆盖主视口。所有可停靠面板都活在这个窗口的 DockSpace 里。
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

    // PassthruCentralNode: 中央节点不画背景，让底层 GL 内容（3D 视口）透出来。
    // 注意：这个 flag 只传 ImGui::DockSpace，不传 DockBuilderAddNode
    // （DockBuilderAddNode 里带 PassthruCentralNode 在某些 ImGui 版本会引发问题）。
    ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    // 每帧都要提交 DockSpace。
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dock_flags);

    if (!s_bLayoutInitialized || s_bResetRequested)
    {
        s_bLayoutInitialized = true;
        s_bResetRequested = false;

        // Warm-up: 让 ImGui 内部先注册这些窗口。
        // 没有这一步，DockBuilderDockWindow 可能找不到窗口，
        // 导致后续 DockBuilderAddNode 内部状态不一致而崩溃。
        ImGui::Begin("Selection"); ImGui::End();
        ImGui::Begin("Console");  ImGui::End();

        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

        // 分割：中央（留给 3D 视口）+ 右侧（Entities）+ 底部（Console）
        ImGuiID dock_main = dockspace_id;
        ImGuiID dock_right = ImGui::DockBuilderSplitNode(
            dock_main, ImGuiDir_Right, 0.25f, nullptr, &dock_main);
        ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(
            dock_main, ImGuiDir_Down, 0.22f, nullptr, &dock_main);

        ImGui::DockBuilderDockWindow("Selection", dock_right);
        ImGui::DockBuilderDockWindow("Console", dock_bottom);

        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::End();
}

//-----------------------------------------------------------------------------
void End()
{
    // 面板在 Begin/End 之间由调用者绘制。这里无事可做。
}

//-----------------------------------------------------------------------------
void ResetDefaultLayout()
{
    s_bResetRequested = true;
}

} // namespace EditorLayout