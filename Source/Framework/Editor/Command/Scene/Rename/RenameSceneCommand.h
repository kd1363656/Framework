#pragma once

namespace FWK
{
    class Scene;
}

namespace FWK::Editor
{
    class RenameSceneCommand final : public ICommand
    {
    public:

         RenameSceneCommand(const std::weak_ptr<Scene>& a_scene, const std::string& a_beforeName, const std::string& a_afterName);

        ~RenameSceneCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        void ApplyName(const std::string& a_name) const;

        std::weak_ptr<Scene> m_scene = {};

        std::string m_beforeName = {};
        std::string m_afterName  = {};
    };
}