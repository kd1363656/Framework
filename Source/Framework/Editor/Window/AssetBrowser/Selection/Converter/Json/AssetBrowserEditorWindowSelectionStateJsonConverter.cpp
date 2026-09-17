#include "AssetBrowserEditorWindowSelectionStateJsonConverter.h"

void FWK::Converter::AssetBrowserEditorWindowSelectionStateJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::AssetBrowserEditorWindowSelectionState& a_assetBrowserEditorWindowSelectionState) const
{
    if (a_rootJson.is_null()) { return; }

          auto  l_selectedFilePathList    = a_rootJson.value(k_selectedFilePathListJsonKey,    std::vector<std::filesystem::path>{});
    const auto& l_rangeSelectionStartPath = a_rootJson.value(k_rangeSelectionStartPathJsonKey, std::filesystem::path{});

    a_assetBrowserEditorWindowSelectionState.SetSelectedFilePathList   (std::move(l_selectedFilePathList));
    a_assetBrowserEditorWindowSelectionState.SetRangeSelectionStartPath(l_rangeSelectionStartPath);
}

nlohmann::json FWK::Converter::AssetBrowserEditorWindowSelectionStateJsonConverter::Serialize(const Editor::AssetBrowserEditorWindowSelectionState& a_assetBrowserEditorWindowSelectionState) const
{
    nlohmann::json l_rootJson = {};

    const auto& l_selectedFilePathList    = a_assetBrowserEditorWindowSelectionState.GetREFSelectedFilePathList   ();
    const auto& l_rangeSelectionStartPath = a_assetBrowserEditorWindowSelectionState.GetREFRangeSelectionStartPath();

    l_rootJson[k_selectedFilePathListJsonKey]    = l_selectedFilePathList;
    l_rootJson[k_rangeSelectionStartPathJsonKey] = l_rangeSelectionStartPath;

    return l_rootJson;
}