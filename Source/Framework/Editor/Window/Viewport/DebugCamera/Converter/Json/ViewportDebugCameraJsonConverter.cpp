#include "ViewportDebugCameraJsonConverter.h"

void FWK::Converter::ViewportDebugCameraJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::ViewportDebugCamera& a_debugCamera) const
{
    if (a_rootJson.is_null()) { return; }

    // 保存されていない項目は、今の値(初期値)のまま使う
    if (a_rootJson.contains(k_positionJsonKey.data()))
    {
        a_debugCamera.SetPosition(Utility::DeserializeVector3(a_rootJson, k_positionJsonKey));
    }

    a_debugCamera.SetYawDegree          (a_rootJson.value(k_yawDegreeJsonKey,           a_debugCamera.GetVALYawDegree          ()));
    a_debugCamera.SetPitchDegree        (a_rootJson.value(k_pitchDegreeJsonKey,         a_debugCamera.GetVALPitchDegree        ()));
    a_debugCamera.SetMoveSpeed          (a_rootJson.value(k_moveSpeedJsonKey,           a_debugCamera.GetVALMoveSpeed          ()));
    a_debugCamera.SetRotationSpeedDegree(a_rootJson.value(k_rotationSpeedDegreeJsonKey, a_debugCamera.GetVALRotationSpeedDegree()));

    a_debugCamera.SetIsEnabled              (a_rootJson.value(k_isEnabledJsonKey,               a_debugCamera.GetVALIsEnabled              ()));
    a_debugCamera.SetIsCullingDebugVisualize(a_rootJson.value(k_isCullingDebugVisualizeJsonKey, a_debugCamera.GetVALIsCullingDebugVisualize()));
    a_debugCamera.SetIsDrawMainCameraFrustum(a_rootJson.value(k_isDrawMainCameraFrustumJsonKey, a_debugCamera.GetVALIsDrawMainCameraFrustum()));
}

nlohmann::json FWK::Converter::ViewportDebugCameraJsonConverter::Serialize(const Editor::ViewportDebugCamera& a_debugCamera) const
{
    nlohmann::json l_rootJson = {};

    Utility::UpdateJson(l_rootJson, Utility::SerializeVector3(a_debugCamera.GetREFPosition(), k_positionJsonKey));

    l_rootJson[k_yawDegreeJsonKey]           = a_debugCamera.GetVALYawDegree          ();
    l_rootJson[k_pitchDegreeJsonKey]         = a_debugCamera.GetVALPitchDegree        ();
    l_rootJson[k_moveSpeedJsonKey]           = a_debugCamera.GetVALMoveSpeed          ();
    l_rootJson[k_rotationSpeedDegreeJsonKey] = a_debugCamera.GetVALRotationSpeedDegree();

    l_rootJson[k_isEnabledJsonKey]               = a_debugCamera.GetVALIsEnabled              ();
    l_rootJson[k_isCullingDebugVisualizeJsonKey] = a_debugCamera.GetVALIsCullingDebugVisualize();
    l_rootJson[k_isDrawMainCameraFrustumJsonKey] = a_debugCamera.GetVALIsDrawMainCameraFrustum();

    return l_rootJson;
}
