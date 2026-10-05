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

void FWK::Editor::EditorCamera::Move(const TypeAlias::Math::Vector3& a_localDirection, const float a_deltaTime)
{
    // カメラの向き(ローカル座標系)で移動方向を変換する
    const auto& l_rotationMatrix = TypeAlias::Math::Matrix::CreateFromQuaternion(m_transform.m_rotation);
    const auto& l_worldDirection = TypeAlias::Math::Vector3::Transform          (a_localDirection, l_rotationMatrix);

    m_transform.m_position += l_worldDirection * (m_moveSpeed * a_deltaTime);

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

nlohmann::json FWK::Editor::EditorCamera::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

FWK::TypeAlias::Math::Matrix FWK::Editor::EditorCamera::CalculateCameraMatrix() const
{
    return TypeAlias::Math::Matrix::CreateScale(m_transform.m_scale)* TypeAlias::Math::Matrix::CreateFromQuaternion (m_transform.m_rotation) * TypeAlias::Math::Matrix::CreateTranslation(m_transform.m_position);
}

void FWK::Editor::EditorCamera::UpdateCameraTransform()
{
    m_camera.ApplyCameraMatrix(CalculateCameraMatrix());
}