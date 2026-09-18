#include "AssetBrowserEditorWindowSelectionStateJsonConverter.h"

void FWK::Converter::AssetBrowserEditorWindowSelectionStateJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::AssetBrowserEditorWindowSelectionState& a_assetBrowserEditorWindowSelectionState) const
{
    if (a_rootJson.is_null()) { return; }

    const auto& l_rangeSelectionStartPath = a_rootJson.value(k_rangeSelectionStartPathJsonKey, std::filesystem::path{});

    if (const auto& l_json = a_rootJson.value(k_selectedFilePathListJsonKey, nlohmann::json{});
        !l_json.is_null() &&
        Utility::IsJsonArray(l_json))
    {
        DeserializeSelectedFilePathList(l_json, a_assetBrowserEditorWindowSelectionState);
    }

    a_assetBrowserEditorWindowSelectionState.SetRangeSelectionStartPath(l_rangeSelectionStartPath);
}

nlohmann::json FWK::Converter::AssetBrowserEditorWindowSelectionStateJsonConverter::Serialize(const Editor::AssetBrowserEditorWindowSelectionState& a_assetBrowserEditorWindowSelectionState) const
{
    nlohmann::json l_rootJson = {};

    const auto& l_rangeSelectionStartPath = a_assetBrowserEditorWindowSelectionState.GetREFRangeSelectionStartPath();

    l_rootJson[k_selectedFilePathListJsonKey]    = SerializeSelectedFilePathList(a_assetBrowserEditorWindowSelectionState);
    l_rootJson[k_rangeSelectionStartPathJsonKey] = l_rangeSelectionStartPath;

    return l_rootJson;
}

void FWK::Converter::AssetBrowserEditorWindowSelectionStateJsonConverter::DeserializeSelectedFilePathList(const nlohmann::json& a_rootJson, Editor::AssetBrowserEditorWindowSelectionState& a_assetBrowserEditorWindowSelectionState) const
{
    if (a_rootJson.is_null() ||
        !Utility::IsJsonArray(a_rootJson)) 
    {
        return; 
    }
    
    std::vector<std::filesystem::path> l_selectedFilePathList = {};

    for (const auto& l_json : a_rootJson)
    {
        const auto& l_selectedFilePath = l_json.value(k_selectedFilePathJsonKey, std::filesystem::path{});

        l_selectedFilePathList.emplace_back(l_selectedFilePath);
    }

    a_assetBrowserEditorWindowSelectionState.SetSelectedFilePathList(std::move(l_selectedFilePathList));
}

nlohmann::json FWK::Converter::AssetBrowserEditorWindowSelectionStateJsonConverter::SerializeSelectedFilePathList(const Editor::AssetBrowserEditorWindowSelectionState& a_assetBrowserEditorWindowSelectionState) const
{
          auto  l_rootJsonArray        = nlohmann::json::array                                              ();
    const auto& l_selectedFilePathList = a_assetBrowserEditorWindowSelectionState.GetREFSelectedFilePathList();

    for (const auto& l_selectedFilePath : l_selectedFilePathList)
    {
        nlohmann::json l_json = {};

        l_json[k_selectedFilePathJsonKey] = l_selectedFilePath;

        l_rootJsonArray.emplace_back(l_json);
    }

    return l_rootJsonArray;
}