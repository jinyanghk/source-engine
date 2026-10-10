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

    if (!s_bLayoutInitialized || s_bResetRequested)
    {
        s_bLayoutInitialized = true;
        s_bResetRequested = false;

        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id,
            ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

        ImGuiID dock_main = dockspace_id;
        ImGuiID dock_right = ImGui::DockBuilderSplitNode(
            dock_main, ImGuiDir_Right, 0.28f, nullptr, &dock_main);
        ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(
            dock_main, ImGuiDir_Down, 0.22f, nullptr, &dock_main);

        ImGui::DockBuilderDockWindow("Entity Palette", dock_right);
        ImGui::DockBuilderDockWindow("Selection",      dock_right);
        ImGui::DockBuilderDockWindow("Console",        dock_bottom);

        ImGui::DockBuilderFinish(dockspace_id);
        printf("dock_main=%u dock_right=%u dock_bottom=%u\n",
            dock_main, dock_right, dock_bottom);
        fflush(stdout);
    }

    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f),
        ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::End();   // ★★★ 这行必须加回来 ★★★
}

//-----------------------------------------------------------------------------
void End()
{
}

//-----------------------------------------------------------------------------
void ResetDefaultLayout()
{
    s_bResetRequested = true;
}

} // namespace EditorLayout