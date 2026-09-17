#include "AssetBrowserEditorWindowAssetPaneJsonConverter.h"

void FWK::Converter::AssetBrowserEditorWindowAssetPaneJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::AssetBrowserEditorWindowAssetPane& a_assetBrowserEditorWindowAssetPane) const
{
    if (a_rootJson.is_null()) { return; }

    if (const auto& l_json = a_rootJson.value(k_selectedFilePathListJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_selectionState = a_assetBrowserEditorWindowAssetPane.GetMutableREFSelectionState();

        l_selectionState.Deserialize(a_rootJson);
    }
}

nlohmann::json FWK::Converter::AssetBrowserEditorWindowAssetPaneJsonConverter::Serialize(const Editor::AssetBrowserEditorWindowAssetPane& a_assetBrowserEditorWindowAssetPane) const
{
          nlohmann::json l_rootJson       = {};
    const auto&          l_selectionState = a_assetBrowserEditorWindowAssetPane.GetREFSelectionState();

    l_rootJson[k_selectedFilePathListJsonKey] = l_selectionState.Serialize();

    return l_rootJson;
}