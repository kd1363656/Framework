#pragma once

namespace FWK::Editor
{
    class EditorUndoRedoSystem final
    {
    public:
    
         EditorUndoRedoSystem() = default;
        ~EditorUndoRedoSystem() = default;
    
        void Deserialize(const nlohmann::json& a_rootJson);

        void HandleUndoRedoShortcut();

        template <Concept::IsDerivedICommandConcept CommandType, typename... Args>
        void PushUndoCommand(Args&&... a_args)
        {
            // 操作を終えた後に呼ぶ
            // 派生クラスをunique_ptrで生成してUndoスタックへ積む
            // std::make_unique + std::forwardで完全転送
            AddUndoCommand(std::make_unique<CommandType>(std::forward<Args>(a_args)...));
        }
        template <Concept::IsDerivedICommandConcept CommandType, typename... Args>
        void ExecuteUndoCommand(Args&&... a_args)
        {
            // 操作をする前に呼ぶ
            // コマンドを作るときに操作前の状態を記録させ、最初のRedoで操作そのものを行う
            // 例 : 削除は親と並び順を記録してから、Redoでシーンから外す
            auto l_command = std::make_unique<CommandType>(std::forward<Args>(a_args)...);

            l_command->Redo();

            AddUndoCommand(std::move(l_command));
        }

        void Undo();
        void Redo();

        void Clear();

        nlohmann::json Serialize() const;

        void SetCapacity(const std::size_t& a_set) { m_capacity = a_set; }

        const auto& GetREFCapacity() const { return m_capacity; }

    private:

        void AddUndoCommand(std::unique_ptr<ICommand>&& a_command);

        std::deque<std::unique_ptr<ICommand>> m_undoList = {};
        std::deque<std::unique_ptr<ICommand>> m_redoList = {};

        std::size_t m_capacity = Constant::k_initialEditorUndoResoSystemListCapacity;

        Converter::EditorUndoRedoSystemJsonConverter m_jsonConverter = {};
    };
}