#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowAssetPane;
}

namespace FWK::Converter
{
    class AssetBrowserEditorWindowAssetPaneJsonConverter final
    {
    public:
    
         AssetBrowserEditorWindowAssetPaneJsonConverter() = default;
        ~AssetBrowserEditorWindowAssetPaneJsonConverter() = default;
        
        void Deserialize(const nlohmann::json& a_rootJson, Editor::AssetBrowserEditorWindowAssetPane& a_assetBrowserEditorWindowAssetPane) const;
        
        nlohmann::json Serialize(const Editor::AssetBrowserEditorWindowAssetPane& a_assetBrowserEditorWindowAssetPane) const;
    
    private:
    
        static constexpr std::string_view k_selectedFilePathListJsonKey  = "SelectedFilePathList";
    };
}