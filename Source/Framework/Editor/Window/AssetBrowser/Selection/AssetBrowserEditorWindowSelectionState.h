#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindow;
}

namespace FWK::Editor
{
    class AssetBrowserEditorWindowSelectionState final
    {
    public:
    
         AssetBrowserEditorWindowSelectionState() = default;
        ~AssetBrowserEditorWindowSelectionState() = default;
        
        void Deserialize(const nlohmann::json& a_rootJson);

        void ClearSelection           ();
        void ClearSelectedFilePathList();

        void SelectSingleFolder(const std::filesystem::path&              a_folderPath, AssetBrowserEditorWindow& a_editorWindow);
        void SelectAll         (const std::vector<std::filesystem::path>& a_displayedFilePathList);

        nlohmann::json Serialize() const;

        void AddSelectedFilePath(const std::filesystem::path& a_set);

        void EraseSelectedFilePath(const std::vector<std::filesystem::path>::const_iterator& a_itr);

        void SetSelectedFilePathList(std::vector<std::filesystem::path>&& a_set) { m_selectedFilePathList = std::move(a_set); }

        void SetRangeSelectionStartPath(const std::filesystem::path& a_set) { m_rangeSelectionStartPath = a_set; }
        
        const auto& GetREFSelectedFilePathList() const { return m_selectedFilePathList; }
        
        const auto& GetREFRangeSelectionStartPath() const { return m_rangeSelectionStartPath; }
        
        auto& GetMutableREFSelectedFilePathList() { return m_selectedFilePathList; }

    private:
    
        std::vector<std::filesystem::path> m_selectedFilePathList = {};

        Converter::AssetBrowserEditorWindowSelectionStateJsonConverter m_jsonConverter = {};

        std::filesystem::path m_rangeSelectionStartPath = {};
    };
}