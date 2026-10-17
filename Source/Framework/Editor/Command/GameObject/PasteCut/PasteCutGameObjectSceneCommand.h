#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK::Editor
{
    class PasteCutGameObjectSceneCommand final : public SceneCommandBase
    {
    public:

         PasteCutGameObjectSceneCommand(const std::vector<std::weak_ptr<GameObject>>& a_pastedGameObjectList, const std::vector<std::weak_ptr<GameObject>>& a_cutGameObjectList);
        ~PasteCutGameObjectSceneCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        std::vector<GameObjectCommandRecord> m_pastedRecordList;
        std::vector<GameObjectCommandRecord> m_cutRecordList;
    };
}