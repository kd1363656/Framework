#include "RenameSceneCommand.h"

FWK::Editor::RenameSceneCommand::RenameSceneCommand(const std::weak_ptr<Scene>& a_scene, const std::string& a_beforeName, const std::string& a_afterName) :
    m_scene(a_scene),

    m_beforeName(a_beforeName),
    m_afterName (a_afterName)
{}
FWK::Editor::RenameSceneCommand::~RenameSceneCommand() = default;

void FWK::Editor::RenameSceneCommand::Undo()
{
    // 変更前の名前へ戻す
    ApplyName(m_beforeName);
}

void FWK::Editor::RenameSceneCommand::Redo()
{
    // 変更後の名前へ進める
    ApplyName(m_afterName);
}

void FWK::Editor::RenameSceneCommand::ApplyName(const std::string& a_name) const
{
    // 名前を変更したSceneが既に破棄されている(別のSceneへ切り替わった)場合は何もしない
    // 現在のSceneを取得し直すと、別のSceneの名前を書き換えてしまうため
    const auto& l_scene = m_scene.lock();

    if (!l_scene) { return; }

    l_scene->SetName(a_name);
}