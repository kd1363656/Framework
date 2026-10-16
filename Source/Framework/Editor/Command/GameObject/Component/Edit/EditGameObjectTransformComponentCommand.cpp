#include "EditGameObjectTransformComponentCommand.h"

FWK::Editor::EditGameObjectTransformComponentCommand::EditGameObjectTransformComponentCommand(const nlohmann::json& a_beforeJson, const nlohmann::json& a_afterJson, const boost::uuids::uuid& a_gameObjectUUID) :
    m_beforeJson(a_beforeJson),
    m_afterJson (a_afterJson),

    m_gameObjectUUID(a_gameObjectUUID)
{}
FWK::Editor::EditGameObjectTransformComponentCommand::~EditGameObjectTransformComponentCommand() = default;

void FWK::Editor::EditGameObjectTransformComponentCommand::Undo()
{
    ApplyJson(m_beforeJson);
}

void FWK::Editor::EditGameObjectTransformComponentCommand::Redo()
{
    ApplyJson(m_afterJson);
}

void FWK::Editor::EditGameObjectTransformComponentCommand::ApplyJson(const nlohmann::json& a_json) const
{
    const auto& l_sceneManager = SceneManager::GetInstance ();
    const auto& l_scene        = l_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    const auto& l_gameObject = l_scene->FindVALGameObject(m_gameObjectUUID).lock();

    if (!l_gameObject) { return; }

    const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();

    if (!l_transformComponent) { return; }

    // 位置・回転・拡大と行列の計算方法をJsonから丸ごと戻す
    // 例 : 位置を(0, 0, 0)から(3, 0, 0)へドラッグした後のUndo → {"Position" : [0, 0, 0], ...}を読み直す
    // 行列の計算方法はDeserializeで作り直されるため、PostDeserializeで親との繋がりを付け直して行列を確定する
    l_transformComponent->Deserialize    (a_json);
    l_transformComponent->PostDeserialize();

    // 子の行列も次のPostLateUpdateで計算し直されるようにする
    l_transformComponent->SetShouldUpdateMatrixDirty(true);
}