#pragma once

namespace FWK::Editor
{
    class EditorUndoRedoSystem final
    {
    public:
    
         EditorUndoRedoSystem() = default;
        ~EditorUndoRedoSystem() = default;
    
        template <Concept::IsDerivedICommandConcept CommandType, typename... Args>
        void PushUndoCommand(Args&&... a_args)
        {
            // 新規操作が入った時点でRedo履歴は破棄する
            m_redoList.clear();

            // 派生クラスをunique_ptrで生成してUndoスタックへ積む
            // std::make_unique + std::forwardで完全転送
            m_undoList.emplace_back(std::make_unique<CommandType>(std::forward<Args>(a_args)...));
        }

        void Undo();
        void Redo();

        void Clear();

    private:
    
        std::vector<std::unique_ptr<ICommand>> m_undoList = {};
        std::vector<std::unique_ptr<ICommand>> m_redoList = {};
    };
}