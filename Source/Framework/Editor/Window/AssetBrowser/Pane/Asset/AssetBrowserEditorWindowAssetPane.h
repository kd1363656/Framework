#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowAssetPane final
    {
    public:

         AssetBrowserEditorWindowAssetPane() = default;
        ~AssetBrowserEditorWindowAssetPane() = default;

        void Draw(const AssetBrowserEditorWindowFolderPane&          a_folderPane,
                  const AssetBrowserEditorWindowAssetCreator&        a_assetCreator,
                  const std::filesystem::path&                       a_assetRootFolderPath,
                        Enum::AssetBrowserActivePaneType&            a_activePane,
                        AssetBrowserEditorWindowPopupDrawer&         a_popupDrawer,
                        AssetBrowserEditorWindowFileOperation&       a_fileOperation,
                        AssetBrowserEditorWindowClipboard&           a_clipboard,
                        AssetFilePathRegistry&                       a_assetFilePathRegistry,
                        Struct::AssetBrowserEditorWindowRenameState& a_renameState);

    private:

        bool IsMultiSelection(const std::vector<std::filesystem::path>& a_selectedList) const;

        static constexpr std::string_view k_childLabel     = "##AssetBrowserEditorWindowRightPane";
        static constexpr std::string_view k_paneTitleLabel = "アセット";
    };
}