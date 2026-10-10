#include "app/ImGuiLayer.h"

#include <cstring>
#include <GL/gl.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

namespace ImGuiLayer
{

//-----------------------------------------------------------------------------
bool Init(SDL_Window* window, void* glContext)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    // ★ 开启 docking 支持。没有这一句，DockSpace 直接 return 0，
    //   后续所有 DockBuilder* 调用都会因为找不到节点而崩溃。
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    if (!ImGui_ImplSDL2_InitForOpenGL(window, glContext))
        return false;
    if (!ImGui_ImplOpenGL3_Init("#version 130"))
        return false;

    return true;
}

//-----------------------------------------------------------------------------
void Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

//-----------------------------------------------------------------------------
void BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

//-----------------------------------------------------------------------------
void EndFrameAndRender(int windowWidth, int windowHeight)
{
GLint currentFbo = 0;
glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFbo);
static GLint s_lastFbo2 = -2;
if (currentFbo != s_lastFbo2) {
    printf("[IMGUI] RenderDrawData FBO -> %d\n", currentFbo);
    fflush(stdout);
    s_lastFbo2 = currentFbo;
}
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (!draw_data)
        return;

    // ImGui 的顶点坐标是从左上角开始的 Y 向下。
    // 我们的 GL 渲染管线里 Y 是向上（ImGui 后端默认假设 Y 向上），
    // 所以这里手工翻转 Y，让 ImGui 的 UI 和我们的手绘内容对齐。
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int v_idx = 0; v_idx < cmd_list->VtxBuffer.Size; v_idx++)
        {
            ImDrawVert& vertex = cmd_list->VtxBuffer.Data[v_idx];
            vertex.pos.y = (float)windowHeight - vertex.pos.y;
        }
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            float clip_rect_h = pcmd->ClipRect.w - pcmd->ClipRect.y;
            pcmd->ClipRect.y = (float)windowHeight - pcmd->ClipRect.w;
            pcmd->ClipRect.w = pcmd->ClipRect.y + clip_rect_h;
        }
    }

    ImGui_ImplOpenGL3_RenderDrawData(draw_data);
}

//-----------------------------------------------------------------------------
void ProcessEvent(const SDL_Event& event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
}

} // namespace ImGuiLayer