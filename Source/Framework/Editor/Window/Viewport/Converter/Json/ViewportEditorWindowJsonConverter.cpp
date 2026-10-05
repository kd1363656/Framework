#include "ViewportEditorWindowJsonConverter.h"

void FWK::Converter::ViewportEditorWindowJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::ViewportEditorWindow& a_viewportEditorWindow) const
{
    if (a_rootJson.is_null()) { return; }

    // エディターカメラのデシリアライズ
    if (const auto& l_json = a_rootJson.value(k_editorCameraJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        const auto& l_editorCamera = a_viewportEditorWindow.GetREFEditorCamera();

        if (l_editorCamera)
        {
            l_editorCamera->Deserialize(l_json);
        }
    }

    // デバッグ描画の表示状態を復元
    a_viewportEditorWindow.SetIsDrawFrustum     (a_rootJson.value(k_isDrawFrustumJsonKey,      false));
    a_viewportEditorWindow.SetIsDrawCulledResult(a_rootJson.value(k_isDrawCulledResultJsonKey, false));
}

nlohmann::json FWK::Converter::ViewportEditorWindowJsonConverter::Serialize(const Editor::ViewportEditorWindow& a_viewportEditorWindow) const
{
          nlohmann::json l_rootJson     = {};
    const auto&          l_editorCamera = a_viewportEditorWindow.GetREFEditorCamera();

    l_rootJson[k_editorCameraJsonKey]       = l_editorCamera ? l_editorCamera->Serialize     () : nlohmann::json{};
    l_rootJson[k_isDrawFrustumJsonKey]      = a_viewportEditorWindow.GetVALIsDrawFrustum     ();
    l_rootJson[k_isDrawCulledResultJsonKey] = a_viewportEditorWindow.GetVALIsDrawCulledResult();

    return l_rootJson;
}