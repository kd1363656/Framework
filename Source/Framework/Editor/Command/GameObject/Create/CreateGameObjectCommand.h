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

         CreateGameObjectCommand(const std::vector<std::weak_ptr<GameObject>>& a_createdGameObjectList);
        ~CreateGameObjectCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        std::vector<std::shared_ptr<GameObject>> m_createdGameObjectList    = {};
        std::vector<std::shared_ptr<GameObject>> m_descendantGameObjectList = {};
        std::vector<boost::uuids::uuid>          m_parentUUIDList           = {};
    };
}