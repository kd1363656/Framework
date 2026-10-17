#pragma once

namespace FWK::Editor
{
    class RenameSceneCommand final : public SceneCommandBase
    {
    public:

         RenameSceneCommand(const std::string& a_beforeName, const std::string& a_afterName);
        ~RenameSceneCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        void ApplyName(const std::string& a_name) const;

        std::string m_beforeName;
        std::string m_afterName;
    };
}