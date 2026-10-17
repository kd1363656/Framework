#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK::Editor
{
    class DestroyGameObjectSceneCommand final : public SceneCommandBase
    {
    public:

         DestroyGameObjectSceneCommand(const std::vector<std::weak_ptr<GameObject>>& a_destroyGameObjectList);
        ~DestroyGameObjectSceneCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        std::vector<GameObjectCommandRecord> m_recordList;
    };
}