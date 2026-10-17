#pragma once

namespace FWK::Editor
{
    class RenameGameObjectSceneCommand final : public SceneCommandBase
    {
    public:

         RenameGameObjectSceneCommand(const std::string& a_beforeName, const std::string& a_afterName, const boost::uuids::uuid& a_gameObjectUUID);
        ~RenameGameObjectSceneCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        void ApplyName(const std::string& a_name) const;

        std::string m_beforeName;
        std::string m_afterName;

        boost::uuids::uuid m_gameObjectUUID;
    };
}