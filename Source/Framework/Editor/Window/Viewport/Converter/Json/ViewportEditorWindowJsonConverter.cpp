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
}

nlohmann::json FWK::Converter::ViewportEditorWindowJsonConverter::Serialize(const Editor::ViewportEditorWindow& a_viewportEditorWindow) const
{
          nlohmann::json l_rootJson     = {};
    const auto&          l_editorCamera = a_viewportEditorWindow.GetREFEditorCamera();

    if (!l_editorCamera) { return {}; }

    l_rootJson[k_editorCameraJsonKey] = l_editorCamera->Serialize();

    return l_rootJson;
}