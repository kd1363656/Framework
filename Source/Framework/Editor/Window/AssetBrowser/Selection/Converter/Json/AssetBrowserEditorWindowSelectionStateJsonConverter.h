#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowSelectionState;
}

namespace FWK::Converter
{
    class AssetBrowserEditorWindowSelectionStateJsonConverter final
    {
    public:
    
         AssetBrowserEditorWindowSelectionStateJsonConverter() = default;
        ~AssetBrowserEditorWindowSelectionStateJsonConverter() = default;
        
        void Deserialize(const nlohmann::json& a_rootJson, Editor::AssetBrowserEditorWindowSelectionState& a_assetBrowserEditorWindowSelectionState) const;
        
        nlohmann::json Serialize(const Editor::AssetBrowserEditorWindowSelectionState& a_assetBrowserEditorWindowSelectionState) const;
    
    private:
    
        void DeserializeSelectedFilePathList(const nlohmann::json& a_rootJson, Editor::AssetBrowserEditorWindowSelectionState& a_assetBrowserEditorWindowSelectionState) const;

        nlohmann::json SerializeSelectedFilePathList(const Editor::AssetBrowserEditorWindowSelectionState& a_assetBrowserEditorWindowSelectionState) const;

        static constexpr std::string_view k_selectedFilePathListJsonKey    = "SelectedFilePathList";
        static constexpr std::string_view k_selectedFilePathJsonKey        = "SelectedFilePath";
        static constexpr std::string_view k_rangeSelectionStartPathJsonKey = "RangeSelectionStartPath";
    };
}