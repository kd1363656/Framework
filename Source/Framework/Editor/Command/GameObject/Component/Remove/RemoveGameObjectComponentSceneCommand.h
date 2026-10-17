#pragma once

namespace FWK
{
    class GameObjectComponentBase;
}

namespace FWK::Editor
{
    class RemoveGameObjectComponentSceneCommand final : public SceneCommandBase
    {
    public:

         RemoveGameObjectComponentSceneCommand(const std::weak_ptr<GameObjectComponentBase>& a_component, const boost::uuids::uuid& a_gameObjectUUID);
        ~RemoveGameObjectComponentSceneCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        std::shared_ptr<GameObjectComponentBase> m_component;

        boost::uuids::uuid m_gameObjectUUID;
    };
}