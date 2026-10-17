#include "EditGameObjectComponentSceneCommand.h"

FWK::Editor::EditGameObjectComponentSceneCommand::EditGameObjectComponentSceneCommand(const nlohmann::json&     a_beforeJson,
                                                                                      const nlohmann::json&     a_afterJson,
                                                                                      const boost::uuids::uuid& a_gameObjectUUID,
                                                                                      const boost::uuids::uuid& a_componentUUID) :
    m_beforeJson(a_beforeJson),
    m_afterJson (a_afterJson),

    m_gameObjectUUID(a_gameObjectUUID),
    m_componentUUID (a_componentUUID)
{}
FWK::Editor::EditGameObjectComponentSceneCommand::~EditGameObjectComponentSceneCommand() = default;

void FWK::Editor::EditGameObjectComponentSceneCommand::Undo()
{
    ApplyJson(m_beforeJson);
}

void FWK::Editor::EditGameObjectComponentSceneCommand::Redo()
{
    ApplyJson(m_afterJson);
}

void FWK::Editor::EditGameObjectComponentSceneCommand::ApplyJson(const nlohmann::json& a_json) const
{
    const auto& l_gameObject = Utility::FindVALSceneGameObject(m_gameObjectUUID).lock();

    if (!l_gameObject) { return; }

    // コンポーネントもUUIDで探す
    // 削除→Undoで戻したコンポーネントも同じUUIDなので、前の編集の記録をそのまま当てられる
    const auto& l_componentContainer = l_gameObject->GetREFComponentContainer     ();
    const auto& l_component          = l_componentContainer.FindVALComponentByUUID(m_componentUUID).lock();

    if (!l_component) { return; }

    // 値をJsonから丸ごと戻し、読み込み直後と同じ準備をし直す
    // 例 : カメラの視野角を60から90にした後のUndo → {"FovYDegree" : 60, ...}を読み直し、射影行列を作り直す
    l_component->Deserialize(a_json);

    if (l_component->GetVALIsDisable()) { return; }

    l_component->PostDeserialize();
}