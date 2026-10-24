#include "GameObjectCameraComponent.h"

void FWK::GameObjectCameraComponent::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::GameObjectCameraComponent::PostDeserialize()
{
    m_camera.Setup();
}

void FWK::GameObjectCameraComponent::EarlyUpdate()
{
    // Viewportのサイズ変更はウィンドウのリサイズ申請を伴わないため、
    // 出力先(Viewportまたはウィンドウ)のAspectRatioが変わったときのみに更新する
    const auto& l_editorManager = Editor::EditorManager::GetInstance          ();
    const float l_aspectRatio   = l_editorManager.FetchVALRenderOutputAspectRatio();

    if (l_aspectRatio <= Constant::k_cameraInvalidAspectRatio ||
        l_aspectRatio == m_camera.GetVALAspectRatio())
    {
        return;
    }

    m_camera.ApplyPerspectiveAspectRatio(l_aspectRatio);
}
void FWK::GameObjectCameraComponent::PostLateUpdate()
{

}

void FWK::GameObjectCameraComponent::EditInspector()
{
    m_inspector.EditInspector(*this);
}

nlohmann::json FWK::GameObjectCameraComponent::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

std::shared_ptr<FWK::GameObjectComponentBase> FWK::GameObjectCameraComponent::Clone() const
{
    auto l_clone = std::make_shared<GameObjectCameraComponent>();

    // INIT()はUUIDなどをリセットするため、値をコピーするDeserialize()より前に呼ぶ
    l_clone->INIT();

    // jsonに乗る部分はDeserialize/Serializeで一括生成
    l_clone->Deserialize(Serialize());

    return l_clone;
}