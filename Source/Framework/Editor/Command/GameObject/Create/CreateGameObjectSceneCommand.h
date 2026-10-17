#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK::Editor
{
    class CreateGameObjectSceneCommand final : public SceneCommandBase
    {
    public:

         CreateGameObjectSceneCommand(const std::vector<std::weak_ptr<GameObject>>& a_createdGameObjectList);
        ~CreateGameObjectSceneCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        std::vector<GameObjectCommandRecord> m_recordList;
    };
}