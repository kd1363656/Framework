#pragma once

namespace FWK
{
    class GameObjectComponentBase;
}

namespace FWK::Editor
{
    class AddGameObjectComponentSceneCommand final : public SceneCommandBase
    {
    public:

         AddGameObjectComponentSceneCommand(const std::shared_ptr<GameObjectComponentBase>& a_component, const boost::uuids::uuid& a_gameObjectUUID);
        ~AddGameObjectComponentSceneCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        std::shared_ptr<GameObjectComponentBase> m_component;

        boost::uuids::uuid m_gameObjectUUID;
    };
}