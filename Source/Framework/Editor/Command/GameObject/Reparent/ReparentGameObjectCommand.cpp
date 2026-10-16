#include "ReparentGameObjectCommand.h"

FWK::Editor::ReparentGameObjectCommand::ReparentGameObjectCommand(const boost::uuids::uuid& a_gameObjectUUID, const Struct::GameObjectPlacement& a_beforePlacement, const Struct::GameObjectPlacement& a_afterPlacement) :
    m_gameObjectUUID(a_gameObjectUUID),

    m_beforePlacement(a_beforePlacement),
    m_afterPlacement (a_afterPlacement)
{}
FWK::Editor::ReparentGameObjectCommand::~ReparentGameObjectCommand() = default;

void FWK::Editor::ReparentGameObjectCommand::Undo()
{
    // 変更前の親と兄弟の並び順へ戻す
    ApplyPlacement(m_beforePlacement);
}

void FWK::Editor::ReparentGameObjectCommand::Redo()
{
    // 変更後の親と兄弟の並び順へ進める
    ApplyPlacement(m_afterPlacement);
}

void FWK::Editor::ReparentGameObjectCommand::ApplyPlacement(const Struct::GameObjectPlacement& a_placement) const
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    // 対象GameObjectをUUIDから検索する
    const auto& l_gameObject = l_scene->FindVALGameObject(m_gameObjectUUID).lock();

    if (!l_gameObject ||
        l_gameObject->GetVALIsDestroyed())
    {
        return;
    }

    // 親の付け替えと、兄弟の並び順の組み直しをまとめて行う
    // 例 : 変更前{ 親 : なし, ルート[A, B, C] } へ戻す → ルートの子にしてから[A, B, C]の順に並べる
    Utility::ApplyGameObjectPlacement(l_gameObject, a_placement, *l_scene);
}