#include "GameObjectCameraComponent.h"

void FWK::GameObjectCameraComponent::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::GameObjectCameraComponent::PostDeserialize()
{
    // 射影行列を作るには縦横比が必要だが、jsonには保存していないため、今の出力先(Viewport又はウィンドウ)の縦横比を使う
    // シーンを読み込む時点ではまだ出力先の大きさが決まっていないことがあるため、その場合は仮の縦横比(16:9)で作り、
    // 最初のEarlyUpdateで本当の縦横比へ直す
    // (縦横比が0のままSetupすると射影行列を作れず、その後のEarlyUpdateでも縦横比を更新できなくなるため)
    const auto& l_editorManager = Editor::EditorManager::GetInstance             ();
    const float l_aspectRatio   = l_editorManager.FetchVALRenderOutputAspectRatio();

    m_camera.SetAspectRatio(l_aspectRatio > Constant::k_cameraInvalidAspectRatio ? l_aspectRatio : Constant::k_cameraDefaultAspectRatio);
    m_camera.Setup         ();
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
    // 持ち主のTransformの位置と向きをカメラへ反映する
    // (Transformはコンポーネント一覧の先頭に固定されていて、PostLateUpdateが他のコンポーネントより先に実行されるため、ここでは行列が確定している)
    const auto& l_owner = GetREFOwner().lock();

    if (!l_owner) { return; }

    const auto& l_transformComponent = l_owner->GetVALTransformComponent().lock();

    if (!l_transformComponent) { return; }

    // カメラ行列に拡大率が入るとビュー行列が歪むため、位置と回転だけを取り出して作り直す
    TypeAlias::Math::Vector3    l_scale    = TypeAlias::Math::Vector3::One;
    TypeAlias::Math::Quaternion l_rotation = TypeAlias::Math::Quaternion::Identity;
    TypeAlias::Math::Vector3    l_position = TypeAlias::Math::Vector3::Zero;

    // SimpleMathのDecomposeは自分自身を書き換える可能性がある関数(constでない)ため、コピーしてから分解する
    auto l_worldMatrix = l_transformComponent->GetREFMatrix();

    if (!l_worldMatrix.Decompose(l_scale, l_rotation, l_position)) { return; }

    const auto& l_cameraMatrix = TypeAlias::Math::Matrix::CreateFromQuaternion(l_rotation) * TypeAlias::Math::Matrix::CreateTranslation(l_position);

    // 変わっていなければ定数バッファを書き換えない
    if (l_cameraMatrix == m_camera.GetREFCameraMatrix()) { return; }

    m_camera.ApplyCameraMatrix(l_cameraMatrix);
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