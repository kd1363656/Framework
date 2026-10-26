#include "ViewportEditorWindowJsonConverter.h"

void FWK::Converter::ViewportEditorWindowJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::ViewportEditorWindow& a_viewportEditorWindow) const
{
    if (a_rootJson.is_null()) { return; }

    a_viewportEditorWindow.SetGizmoOperation(a_rootJson.value(k_gizmoOperationJsonKey, a_viewportEditorWindow.GetVALGizmoOperation()));

    if (a_rootJson.contains(k_cameraPreviewPanelSizeJsonKey.data()))
    {
        auto& l_cameraPreview = a_viewportEditorWindow.GetMutableREFCameraPreview();

        l_cameraPreview.SetPanelSize(Utility::DeserializeIMVEC2(a_rootJson, k_cameraPreviewPanelSizeJsonKey));
    }

    if (const auto& l_json = a_rootJson.value(k_debugCameraJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_debugCamera = a_viewportEditorWindow.GetMutableREFDebugCamera();

        l_debugCamera.Deserialize(l_json);
    }
}

nlohmann::json FWK::Converter::ViewportEditorWindowJsonConverter::Serialize(const Editor::ViewportEditorWindow& a_viewportEditorWindow) const
{
    nlohmann::json l_rootJson = {};

    const auto& l_cameraPreview = a_viewportEditorWindow.GetREFCameraPreview();
    const auto& l_debugCamera   = a_viewportEditorWindow.GetREFDebugCamera  ();

    l_rootJson[k_gizmoOperationJsonKey] = a_viewportEditorWindow.GetVALGizmoOperation();

    Utility::UpdateJson(l_rootJson, Utility::SerializeIMVEC2(l_cameraPreview.GetREFPanelSize(), k_cameraPreviewPanelSizeJsonKey));

    l_rootJson[k_debugCameraJsonKey] = l_debugCamera.Serialize();

    return l_rootJson;
}
