#include "GameObjectCameraComponent.h"
#include "../../../../Application/Application.h"

void FWK::GameObjectCameraComponent::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::GameObjectCameraComponent::PostDeserialize()
{
    // 自身のGameObjectが持つTransformComponentをキャッシュする
    // PostLateUpdateで毎フレームGameObjectから取り直さなくて済むようにするため
    // ※注意 : Ownerは、コンポーネントをGameObjectへ追加した時点で設定されているため、PostDeserializeで使える
    m_fetchSelfGameObjectTransformComponentHelper.PostDeserialize(GetREFOwner());

    const auto& l_application = Application::GetInstance    ();
    const auto& l_window      = l_application.GetREFWindow  ();

    m_camera.SetAspectRatio(l_window.GetVALAspectRatio());
    m_camera.Setup         ();
}

void FWK::GameObjectCameraComponent::EarlyUpdate()
{
    const auto& l_application = Application::GetInstance    ();
    const auto& l_window      = l_application.GetREFWindow  ();

    // リサイズ申請がされたときのみにアスペクト比率を更新する
    if (const auto& l_resizeRequest = l_window.GetREFResizeRequest();
        !l_resizeRequest.m_isRequested)
    {
        return;
    }

    m_camera.SetAspectRatio(l_window.GetVALAspectRatio());
}
void FWK::GameObjectCameraComponent::PostLateUpdate()
{
    // PostDeserializeでキャッシュしたTransformComponentを使う
    const auto& l_transformComponent = m_fetchSelfGameObjectTransformComponentHelper.GetREFTransformComponent().lock();

    if (!l_transformComponent) { return; }

    // エディター表示中もシーンカメラの定数バッファ内容は最新へ保つ
    // (カリング可視化などがシーンカメラの値を参照するため)
    m_camera.ApplyCameraMatrix(l_transformComponent->GetREFMatrix());

    // シーンカメラとしてCameraContextへ登録する
    // 描画にシーンカメラとデバッグカメラのどちらを使うかは、CameraContextがモードに合わせて決める
    auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance     ();
    auto& l_renderer        = l_graphicsManager.GetMutableREFRenderer    ();
    auto& l_mainRenderView  = l_renderer.GetMutableREFMainRenderView     ();
    auto& l_cameraContext   = l_mainRenderView.GetMutableREFCameraContext();

    l_cameraContext.SetSceneCamera(m_camera.GetREFCBCameraPass());
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

    // jsonに乗る部分はDeserialize/Serializeで一括生成
    l_clone->Deserialize(Serialize());

    return l_clone;
}