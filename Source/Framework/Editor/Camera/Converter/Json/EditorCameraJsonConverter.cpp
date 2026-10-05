#include "EditorCameraJsonConverter.h"

void FWK::Converter::EditorCameraJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::EditorCamera& a_editorCamera) const
{
    if (a_rootJson.is_null()) { return; }

    // ScaleとRotationはキーが存在しない場合にZero値が適用されると
    // カメラ行列が退化してしまうため、キー存在チェックを挟む
    if (a_rootJson.contains(k_scaleJsonKey.data()))
    {
        a_editorCamera.ApplyTransformScale(Utility::DeserializeVector3(a_rootJson, k_scaleJsonKey));
    }
    if (a_rootJson.contains(k_rotationJsonKey.data()))
    {
        a_editorCamera.ApplyTransformRotation(Utility::DeserializeQuaternion(a_rootJson, k_rotationJsonKey));
    }

    a_editorCamera.ApplyTransformPosition(Utility::DeserializeVector3(a_rootJson, k_positionJsonKey));

    auto& l_camera = a_editorCamera.GetMutableREFCamera();

    l_camera.SetFovYDegree(a_rootJson.value(k_fovYDegreeJsonKey, Constant::k_cameraDefaultFOVYDegree));
    l_camera.SetFarClip   (a_rootJson.value(k_farClipJsonKey,    Constant::k_cameraDefaultFarClip));
    l_camera.SetNearClip  (a_rootJson.value(k_nearClipJsonKey,   Constant::k_cameraDefaultNearClip));

    a_editorCamera.SetMoveSpeed  (a_rootJson.value(k_moveSpeedJsonKey,   Constant::k_editorCameraDefaultMoveSpeed));
    a_editorCamera.SetRotateSpeed(a_rootJson.value(k_rotateSpeedJsonKey, Constant::k_editorCameraDefaultRotateSpeed));
}

nlohmann::json FWK::Converter::EditorCameraJsonConverter::Serialize(const Editor::EditorCamera& a_editorCamera) const
{
    nlohmann::json l_rootJson = {};

    const auto& l_transform = a_editorCamera.GetREFTransform();
    const auto& l_camera    = a_editorCamera.GetREFCamera   ();

    Utility::UpdateJson(l_rootJson, Utility::SerializeVector3   (l_transform.m_scale,    k_scaleJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeQuaternion(l_transform.m_rotation, k_rotationJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeVector3   (l_transform.m_position, k_positionJsonKey));

    l_rootJson[k_fovYDegreeJsonKey] = l_camera.GetVALFovYDegree();
    l_rootJson[k_farClipJsonKey]    = l_camera.GetVALFarClip   ();
    l_rootJson[k_nearClipJsonKey]   = l_camera.GetVALNearClip  ();

    l_rootJson[k_moveSpeedJsonKey]   = a_editorCamera.GetVALMoveSpeed  ();
    l_rootJson[k_rotateSpeedJsonKey] = a_editorCamera.GetVALRotateSpeed();

    return l_rootJson;
}