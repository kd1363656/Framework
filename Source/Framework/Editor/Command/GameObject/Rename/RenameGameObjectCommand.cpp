#include "RenameGameObjectCommand.h"

FWK::Editor::RenameGameObjectCommand::RenameGameObjectCommand(const boost::uuids::uuid& a_gameObjectUUID,
                                                              const std::string&        a_beforeName,
                                                              const std::string&        a_afterName) :
    m_gameObjectUUID(a_gameObjectUUID),

    m_beforeName(a_beforeName),
    m_afterName (a_afterName)
{}
FWK::Editor::RenameGameObjectCommand::~RenameGameObjectCommand() = default;

void FWK::Editor::RenameGameObjectCommand::Undo()
{
    // 変更前の名前へ戻す
    ApplyName(m_beforeName);
}

void FWK::Editor::RenameGameObjectCommand::Redo()
{
    // 変更後の名前へ進める
    ApplyName(m_afterName);
}

void FWK::Editor::RenameGameObjectCommand::ApplyName(const std::string& a_name) const
{
    // SceneManager経由で現在のSceneを取得する
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    // コマンドはGameObjectを直接保持しないため
    // SceneInstanceUUIDから現在のGameObjectを検索する
    const auto& l_gameObject = l_scene->FindVALGameObject(m_gameObjectUUID).lock();

    if (!l_gameObject ||
        l_gameObject->GetVALIsDestroyed())
    {
        return;
    }

    l_gameObject->SetName(a_name);
}