#ifndef EDITOR_LAYOUT_H
#define EDITOR_LAYOUT_H

//-----------------------------------------------------------------------------
// DockSpace 和默认布局。
//
// 每帧调用 Begin/End 包裹所有面板。
// ResetDefaultLayout 用来恢复 Hammer 风格的初始布局。
//-----------------------------------------------------------------------------
namespace EditorLayout
{
    void Begin();
    void End();
    void ResetDefaultLayout();
}

#endif // EDITOR_LAYOUT_H