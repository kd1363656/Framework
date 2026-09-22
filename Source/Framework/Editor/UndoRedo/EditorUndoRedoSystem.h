#pragma once

namespace FWK::Editor
{
    class EditorUndoRedoSystem final
    {
    public:
    
         EditorUndoRedoSystem() = default;
        ~EditorUndoRedoSystem() = default;
    
        void Deserialize(const nlohmann::json& a_rootJson);

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

        nlohmann::json Serialize() const;

        void SetCapacity(const std::size_t& a_set) { m_capacity = a_set; }

        const auto& GetREFCapacity() const { return m_capacity; }

    private:

        std::deque<std::unique_ptr<ICommand>> m_undoList = {};
        std::deque<std::unique_ptr<ICommand>> m_redoList = {};

        std::size_t m_capacity = Constant::k_initialEditorUndoResoSystemListCapacity;

        Converter::EditorUndoRedoSystemJsonConverter m_jsonConverter = {};
    };
}