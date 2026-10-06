#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK::Editor
{
    class CreateGameObjectCommand final : public ICommand
    {
    public:

         CreateGameObjectCommand(std::vector<std::weak_ptr<GameObject>>&& a_createdGameObjectList, const boost::uuids::uuid& a_parentUUID);
        ~CreateGameObjectCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        std::vector<std::shared_ptr<GameObject>> m_createdGameObjectList = {};

        boost::uuids::uuid m_parentUUID = {};
    };
}