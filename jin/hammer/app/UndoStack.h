#ifndef UNDO_STACK_H
#define UNDO_STACK_H

#include <vector>
#include <string>
#include "scene/Brush.h"
#include "scene/Entity.h"

struct EditorSnapshot
{
    std::vector<CBrush>  brushes;
    std::vector<CEntity> entities;
    int selectedBrush  = -1;
    int selectedEntity = -1;
    std::string label;
};

class UndoStack
{
public:
    static UndoStack& Instance();

    // 在操作"开始前"调用，把当前状态压入撤销栈
    void PushSnapshot(const char* label);

    bool Undo();
    bool Redo();

    bool CanUndo() const { return !m_undoStack.empty(); }
    bool CanRedo() const { return !m_redoStack.empty(); }

    int GetUndoCount() const { return (int)m_undoStack.size(); }
    int GetRedoCount() const { return (int)m_redoStack.size(); }

    // 获取上一个操作的名字（用于菜单显示）
    std::string GetUndoLabel() const;
    std::string GetRedoLabel() const;

    void Clear();

private:
    UndoStack() = default;
    ~UndoStack() = default;
    UndoStack(const UndoStack&) = delete;
    UndoStack& operator=(const UndoStack&) = delete;

    EditorSnapshot CaptureCurrent() const;
    void ApplySnapshot(const EditorSnapshot& snap);

    std::vector<EditorSnapshot> m_undoStack;
    std::vector<EditorSnapshot> m_redoStack;
    static const size_t MAX_DEPTH = 50;
};

#endif // UNDO_STACK_H