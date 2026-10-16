#include "RenameGameObjectCommand.h"

FWK::Editor::RenameGameObjectCommand::RenameGameObjectCommand(const boost::uuids::uuid& a_gameObjectUUID, const std::string& a_beforeName, const std::string& a_afterName) :
    m_beforeName(a_beforeName),
    m_afterName (a_afterName),

    m_gameObjectUUID(a_gameObjectUUID)
{}
FWK::Editor::RenameGameObjectCommand::~RenameGameObjectCommand() = default;

void FWK::Editor::RenameGameObjectCommand::Undo()
{
    ApplyName(m_beforeName);
}

void FWK::Editor::RenameGameObjectCommand::Redo()
{
    ApplyName(m_afterName);
}

void FWK::Editor::RenameGameObjectCommand::ApplyName(const std::string& a_name) const
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    // GameObjectはポインタではなくUUIDで探す
    // 削除のUndoなどで同じUUIDのGameObjectがシーンへ戻ってきても見つけられるため
    const auto& l_gameObject = l_scene->FindVALGameObject(m_gameObjectUUID).lock();

    if (!l_gameObject) { return; }

    l_gameObject->SetName(a_name);
}