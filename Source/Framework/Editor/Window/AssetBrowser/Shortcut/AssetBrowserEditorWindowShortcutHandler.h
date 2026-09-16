#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindow;
    class AssetBrowserEditorWindowFolderPane;
}

namespace FWK::Editor
{
    class AssetBrowserEditorWindowShortcutHandler final
    {
    public:

         AssetBrowserEditorWindowShortcutHandler() = default;
        ~AssetBrowserEditorWindowShortcutHandler() = default;

        void Handle(const std::vector<std::filesystem::path>& a_selectedFilePathList, const std::filesystem::path& a_targetFilePath, AssetBrowserEditorWindow& a_editorWindow) const;

        void HandleFolderPane(AssetBrowserEditorWindow& a_editorWindow);

    private:

        void HandleCreateFolder(const AssetBrowserEditorWindowAssetCreator&        a_assetCreator,
                                const std::filesystem::path&                       a_parentFolderPath, 
                                      AssetBrowserEditorWindowFolderPane&          a_folderPane,
                                      Struct::AssetBrowserEditorWindowRenameState& a_renameState) const;

        void HandleRename   (const std::filesystem::path&              a_targetFilePath,       Struct::AssetBrowserEditorWindowRenameState& a_renameState)                                                   const;
        void HandleCopy     (const std::vector<std::filesystem::path>& a_selectedFilePathList, AssetBrowserEditorWindowFileOperation&       a_fileOperation, AssetBrowserEditorWindowClipboard& a_clipboard) const;
        void HandleCut      (const std::vector<std::filesystem::path>& a_selectedFilePathList, AssetBrowserEditorWindowFileOperation&       a_fileOperation, AssetBrowserEditorWindowClipboard& a_clipboard) const;
        void HandlePaste    (const std::filesystem::path&              a_targetFolderPath,     AssetBrowserEditorWindowFileOperation&       a_fileOperation, AssetBrowserEditorWindowClipboard& a_clipboard) const;
        void HandleDuplicate(const std::vector<std::filesystem::path>& a_selectedFilePathList, AssetBrowserEditorWindowFileOperation&       a_fileOperation)                                                 const;
        void HandleDelete   (const std::vector<std::filesystem::path>& a_selectedFilePathList, AssetBrowserEditorWindowFileOperation&       a_fileOperation)                                                 const;
    };
}
