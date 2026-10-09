#pragma once

namespace FWK
{
    class GameObject;
    class GameObjectComponentBase;
}

namespace FWK::Editor
{
    class RemoveGameObjectComponentCommand final : public ICommand
    {
    public:

         RemoveGameObjectComponentCommand(const std::vector<boost::uuids::uuid>& a_beforeComponentUUIDList, const std::shared_ptr<GameObjectComponentBase>& a_component, const boost::uuids::uuid& a_gameObjectUUID);
        ~RemoveGameObjectComponentCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        std::weak_ptr<GameObject> FindVALTargetGameObject() const;

        std::vector<boost::uuids::uuid> m_beforeComponentUUIDList;

        std::shared_ptr<GameObjectComponentBase> m_component;

        boost::uuids::uuid m_gameObjectUUID;
    };
}