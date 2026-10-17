#include "AddGameObjectComponentSceneCommand.h"

FWK::Editor::AddGameObjectComponentSceneCommand::AddGameObjectComponentSceneCommand(const std::shared_ptr<GameObjectComponentBase>& a_component, const boost::uuids::uuid& a_gameObjectUUID) :
    m_component(a_component),

    m_gameObjectUUID(a_gameObjectUUID)
{}
FWK::Editor::AddGameObjectComponentSceneCommand::~AddGameObjectComponentSceneCommand() = default;

void FWK::Editor::AddGameObjectComponentSceneCommand::Undo()
{
    const auto& l_gameObject = Utility::FindVALSceneGameObject(m_gameObjectUUID).lock();

    if (!l_gameObject) { return; }

    // 追加したコンポーネントを外す
    // 実体はこのコマンドが持ち続けるので、Redoで同じコンポーネント(同じUUID・同じ値)を戻せる
    auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();

    l_componentContainer.RemoveComponent(m_component);
}

void FWK::Editor::AddGameObjectComponentSceneCommand::Redo()
{
    if (!m_component) { return; }

    const auto& l_gameObject = Utility::FindVALSceneGameObject(m_gameObjectUUID).lock();

    if (!l_gameObject) { return; }

    if (auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();
        !l_componentContainer.AddComponent(m_component))
    {
        return;
    }

    // シーンを読み込んだ直後と同じ準備をする(カメラの行列計算など)
    // 無効なコンポーネントは読み込み時と同じく準備しない
    if (m_component->GetVALIsDisable()) { return; }

    m_component->PostDeserialize();
}