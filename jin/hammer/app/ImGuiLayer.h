#ifndef IMGUI_LAYER_H
#define IMGUI_LAYER_H

#include <SDL2/SDL.h>

//-----------------------------------------------------------------------------
// ImGui 生命周期封装。
//
// 职责：
//   - Init / Shutdown
//   - 每帧 BeginFrame / EndFrameAndRender
//   - SDL 事件转发
//
// 不管 UI 长什么样，只管 ImGui 框架跑起来。
//-----------------------------------------------------------------------------
namespace ImGuiLayer
{
    // 在 GL 上下文就绪后调用一次。
    bool Init(SDL_Window* window, void* glContext);

    // 在 GL 上下文销毁前调用。
    void Shutdown();

    // 每帧调用。
    void BeginFrame();
    void EndFrameAndRender(int windowWidth, int windowHeight);

    // 把 SDL 事件转发给 ImGui（在 SDL_PollEvent 循环里调用）。
    void ProcessEvent(const SDL_Event& event);
}

#endif // IMGUI_LAYER_H