#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK::Editor
{
    class PasteCutGameObjectCommand final : public ICommand
    {
    public:

         PasteCutGameObjectCommand(const std::vector<std::weak_ptr<GameObject>>& a_pastedGameObjectList, const std::vector<std::weak_ptr<GameObject>>& a_cutGameObjectList);
        ~PasteCutGameObjectCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        std::vector<GameObjectCommandRecord> m_pastedRecordList;
        std::vector<GameObjectCommandRecord> m_cutRecordList;
    };
}