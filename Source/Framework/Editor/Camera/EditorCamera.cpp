#include "EditorCamera.h"

void FWK::Editor::EditorCamera::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Editor::EditorCamera::Setup(const float a_aspectRatio)
{
    m_camera.Setup(CalculateCameraMatrix(),
                   a_aspectRatio,
                   m_camera.GetVALFovYDegree(),
                   m_camera.GetVALFarClip(),
                   m_camera.GetVALNearClip());
}

void FWK::Editor::EditorCamera::HandleInput(const bool a_isHovered)
{
    // ビューポート画像の上にマウスがあり、右クリックしている間だけ、カメラを操作する
    // a_isHoveredは、ビューポートを描くウィンドウ側が「操作してよい状態か」を判断して渡す
    if (!a_isHovered ||
        !ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        return;
    }

    const auto& l_io = ImGui::GetIO();

    // マウスの移動量をYaw/Pitch回転へ変換する
    Rotate(l_io.MouseDelta.x * m_rotateSpeed, l_io.MouseDelta.y * m_rotateSpeed);

    // WASD/QEでカメラローカル方向の移動量を組み立てる
    auto l_localDirection = TypeAlias::Math::Vector3::Zero;

    // X軸Z軸に移動
    if (ImGui::IsKeyDown(ImGuiKey_W))
    {
        l_localDirection.z += TypeAlias::Math::Vector3::UnitZ.z;
    }
    if (ImGui::IsKeyDown(ImGuiKey_S))
    {
        l_localDirection.z -= TypeAlias::Math::Vector3::UnitZ.z;
    }
    if (ImGui::IsKeyDown(ImGuiKey_D))
    {
        l_localDirection.x += TypeAlias::Math::Vector3::UnitX.x;
    }
    if (ImGui::IsKeyDown(ImGuiKey_A))
    {
        l_localDirection.x -= TypeAlias::Math::Vector3::UnitX.x;
    }

    // Y軸に移動
    if (ImGui::IsKeyDown(ImGuiKey_E))
    {
        l_localDirection.y += TypeAlias::Math::Vector3::UnitY.y;
    }
    if (ImGui::IsKeyDown(ImGuiKey_Q))
    {
        l_localDirection.y -= TypeAlias::Math::Vector3::UnitY.y;
    }

    // 何も押されていない場合は、移動しない
    if (l_localDirection == TypeAlias::Math::Vector3::Zero) { return; }

    // 斜めに押したときだけ速くならないよう、方向の長さを1にしてから移動する
    l_localDirection.Normalize();

    Move(l_localDirection, l_io.DeltaTime);
}

nlohmann::json FWK::Editor::EditorCamera::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Editor::EditorCamera::ApplyTransformPosition(const TypeAlias::Math::Vector3& a_set)
{
    m_transform.m_position = a_set;

    UpdateCameraTransform();
}

void FWK::Editor::EditorCamera::ApplyTransformRotation(const TypeAlias::Math::Quaternion& a_set)
{
    m_transform.m_rotation = a_set;

    UpdateCameraTransform();
}

void FWK::Editor::EditorCamera::ApplyTransformScale(const TypeAlias::Math::Vector3& a_set)
{
    m_transform.m_scale = a_set;

    UpdateCameraTransform();
}

void FWK::Editor::EditorCamera::ApplyAspectRatio(const float a_set)
{
    m_camera.ApplyPerspectiveAspectRatio(a_set);
}

void FWK::Editor::EditorCamera::ApplyFovYDegree(const float a_set)
{
    m_camera.ApplyProjectionMatrix(m_camera.GetVALAspectRatio(),
                                   a_set,
                                   m_camera.GetVALFarClip(),
                                   m_camera.GetVALNearClip());
}

void FWK::Editor::EditorCamera::Move(const TypeAlias::Math::Vector3& a_localDirection, const float a_deltaTime)
{
    // カメラの向き(ローカル座標系)で移動方向を変換する
    const auto& l_rotationMatrix = TypeAlias::Math::Matrix::CreateFromQuaternion(m_transform.m_rotation);
    const auto& l_worldDirection = TypeAlias::Math::Vector3::Transform          (a_localDirection, l_rotationMatrix);

    m_transform.m_position += l_worldDirection * m_moveSpeed * a_deltaTime;

    UpdateCameraTransform();
}

void FWK::Editor::EditorCamera::Rotate(const float a_yawRadian, const float a_pitchRadian)
{
    // 現在のQuaternionをEuler角(度)へ分解し、Yaw/Pitchへ差分を加算してからQuaternionへ戻す
    auto l_euler = Utility::QuaternionToEuler(m_transform.m_rotation);

    l_euler.y += DirectX::XMConvertToDegrees(a_yawRadian);

    // 真上・真下を向いたときに視点が反転しないようPitch角度を制限する
    l_euler.x  = std::clamp(l_euler.x + DirectX::XMConvertToDegrees(a_pitchRadian), -Constant::k_editorCameraPitchLimitDegree, Constant::k_editorCameraPitchLimitDegree);

    m_transform.m_rotation = Utility::EulerToQuaternion(l_euler);

    UpdateCameraTransform();
}

FWK::TypeAlias::Math::Matrix FWK::Editor::EditorCamera::CalculateCameraMatrix() const
{
    return TypeAlias::Math::Matrix::CreateScale(m_transform.m_scale)* TypeAlias::Math::Matrix::CreateFromQuaternion (m_transform.m_rotation) * TypeAlias::Math::Matrix::CreateTranslation(m_transform.m_position);
}

void FWK::Editor::EditorCamera::UpdateCameraTransform()
{
    m_camera.ApplyCameraMatrix(CalculateCameraMatrix());
}