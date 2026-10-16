#include "RenameSceneCommand.h"

FWK::Editor::RenameSceneCommand::RenameSceneCommand(const std::string& a_beforeName, const std::string& a_afterName) :
    m_beforeName(a_beforeName),
    m_afterName (a_afterName)
{}
FWK::Editor::RenameSceneCommand::~RenameSceneCommand() = default;

void FWK::Editor::RenameSceneCommand::Undo()
{
    ApplyName(m_beforeName);
}

void FWK::Editor::RenameSceneCommand::Redo()
{
    ApplyName(m_afterName);
}

void FWK::Editor::RenameSceneCommand::ApplyName(const std::string& a_name) const
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    // シーンを読み込み直すと履歴は消えるため、今のシーンが名前を変えたシーンになる
    l_scene->SetName(a_name);
}