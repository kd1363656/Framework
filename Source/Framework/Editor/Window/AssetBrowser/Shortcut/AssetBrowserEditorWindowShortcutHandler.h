#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowShortcutHandler final
    {
    public:

         AssetBrowserEditorWindowShortcutHandler() = default;
        ~AssetBrowserEditorWindowShortcutHandler() = default;

        void Handle(const std::vector<std::filesystem::path>&          a_selectedFilePathList,
                    const std::filesystem::path&                       a_parentFolderPath,
                    const std::filesystem::path&                       a_targetFilePath,
                    const Enum::AssetBrowserActivePaneType             a_activePane,
                    const AssetBrowserEditorWindowAssetCreator&        a_assetCreator,
                          AssetBrowserEditorWindowFileOperation&       a_fileOperation,
                          AssetBrowserEditorWindowClipboard&           a_clipboard,
                          Struct::AssetBrowserEditorWindowRenameState& a_renameState) const;

    private:

        void HandleCreateFolder(const std::filesystem::path& a_parentFolderPath, const AssetBrowserEditorWindowAssetCreator& a_assetCreator, Struct::AssetBrowserEditorWindowRenameState& a_renameState) const;

        void HandleRename   (const std::filesystem::path&              a_targetFilePath,       Struct::AssetBrowserEditorWindowRenameState& a_renameState)                                                   const;
        void HandleCopy     (const std::vector<std::filesystem::path>& a_selectedFilePathList, AssetBrowserEditorWindowFileOperation&       a_fileOperation, AssetBrowserEditorWindowClipboard& a_clipboard) const;
        void HandleCut      (const std::vector<std::filesystem::path>& a_selectedFilePathList, AssetBrowserEditorWindowFileOperation&       a_fileOperation, AssetBrowserEditorWindowClipboard& a_clipboard) const;
        void HandlePaste    (const std::filesystem::path&              a_parentFolderPath,     AssetBrowserEditorWindowFileOperation&       a_fileOperation, AssetBrowserEditorWindowClipboard& a_clipboard) const;
        void HandleDuplicate(const std::vector<std::filesystem::path>& a_selectedFilePathList, AssetBrowserEditorWindowFileOperation&       a_fileOperation)                                                 const;
        void HandleDelete   (const std::vector<std::filesystem::path>& a_selectedFilePathList, AssetBrowserEditorWindowFileOperation&       a_fileOperation)                                                 const;

        static constexpr std::size_t k_singleSize = 1ULL;
    };
}
