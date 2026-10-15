#include "app/UndoStack.h"
#include "scene/SceneGlobals.h"

UndoStack& UndoStack::Instance()
{
    static UndoStack instance;
    return instance;
}

EditorSnapshot UndoStack::CaptureCurrent() const
{
    EditorSnapshot snap;
    snap.brushes       = g_brushes;      // vector 深拷贝
    snap.entities      = g_entities;     // vector 深拷贝
    snap.selectedBrush  = g_iSelectedBrush;
    snap.selectedEntity = g_iSelectedEntity;
    return snap;
}

void UndoStack::ApplySnapshot(const EditorSnapshot& snap)
{
    g_brushes         = snap.brushes;
    g_entities        = snap.entities;
    g_iSelectedBrush  = snap.selectedBrush;
    g_iSelectedEntity = snap.selectedEntity;
}

void UndoStack::PushSnapshot(const char* label)
{
    EditorSnapshot snap = CaptureCurrent();
    snap.label = label ? label : "";
    m_undoStack.push_back(snap);

    // 限制栈深度
    if (m_undoStack.size() > MAX_DEPTH)
        m_undoStack.erase(m_undoStack.begin());

    // 新的操作发生 → 清空 redo 栈
    m_redoStack.clear();
}

bool UndoStack::Undo()
{
    if (m_undoStack.empty()) return false;

    // 1. 保存当前状态到 redo 栈
    EditorSnapshot current = CaptureCurrent();
    current.label = m_undoStack.back().label;   // 保留 label（用于 redo 显示）
    m_redoStack.push_back(current);

    // 2. 从 undo 栈弹出，恢复
    EditorSnapshot snap = m_undoStack.back();
    m_undoStack.pop_back();
    ApplySnapshot(snap);

    return true;
}

bool UndoStack::Redo()
{
    if (m_redoStack.empty()) return false;

    // 1. 保存当前状态到 undo 栈
    EditorSnapshot current = CaptureCurrent();
    current.label = m_redoStack.back().label;
    m_undoStack.push_back(current);

    // 2. 从 redo 栈弹出，恢复
    EditorSnapshot snap = m_redoStack.back();
    m_redoStack.pop_back();
    ApplySnapshot(snap);

    return true;
}

std::string UndoStack::GetUndoLabel() const
{
    if (m_undoStack.empty()) return "";
    return m_undoStack.back().label;
}

std::string UndoStack::GetRedoLabel() const
{
    if (m_redoStack.empty()) return "";
    return m_redoStack.back().label;
}

void UndoStack::Clear()
{
    m_undoStack.clear();
    m_redoStack.clear();
}