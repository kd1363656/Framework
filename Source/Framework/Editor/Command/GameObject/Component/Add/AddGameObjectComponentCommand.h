#pragma once

namespace FWK
{
    class GameObject;
    class GameObjectComponentBase;
}

namespace FWK::Editor
{
    class AddGameObjectComponentCommand final : public ICommand
    {
    public:

         AddGameObjectComponentCommand(const std::shared_ptr<GameObjectComponentBase>& a_component, const boost::uuids::uuid& a_gameObjectUUID);
        ~AddGameObjectComponentCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        std::weak_ptr<GameObject> FindVALTargetGameObject() const;

        std::shared_ptr<GameObjectComponentBase> m_component;

        boost::uuids::uuid m_gameObjectUUID;
    };
}