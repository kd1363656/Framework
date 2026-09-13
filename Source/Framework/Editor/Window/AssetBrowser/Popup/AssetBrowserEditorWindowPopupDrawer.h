#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowPopupDrawer final
    {
    public:

         AssetBrowserEditorWindowPopupDrawer() = default;
        ~AssetBrowserEditorWindowPopupDrawer() = default;

        void Draw(const std::vector<std::filesystem::path>&          a_selectedFilePathList,
                  const std::filesystem::path&                       a_targetFilePath,
                  const std::filesystem::path&                       a_parentFolderPath,
                  const Enum::AssetBrowserPopupContextType           a_contextType,
                        AssetBrowserEditorWindowAssetCreator&        a_assetCreator,
                        AssetBrowserEditorWindowClipboard&           a_clipboard,
                        AssetFilePathRegistry&                       a_filePathRegistry,
                        Struct::AssetBrowserEditorWindowRenameState& a_renameState) const;

    private:

        void StartRename(const std::filesystem::path& a_targetFilePath, Struct::AssetBrowserEditorWindowRenameState& a_renameState) const;
    };
}
