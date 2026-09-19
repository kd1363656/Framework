#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindow;
}

namespace FWK::Editor
{
    class AssetBrowserEditorWindowPopupDrawer final
    {
    public:

         AssetBrowserEditorWindowPopupDrawer() = default;
        ~AssetBrowserEditorWindowPopupDrawer() = default;

        void BeginPopup(const std::string_view& a_openPopupLabel) const;

        void Draw(const std::vector<std::filesystem::path>& a_selectedFilePathList, 
                  const std::filesystem::path&              a_targetFilePath,
                  const std::string_view&                   a_openPopupLabel,
                  const Enum::AssetBrowserPopupContextType  a_contextType,
                        AssetBrowserEditorWindow&           a_editorWindow) const;

    private:

        void DrawCreateFolderMenu(const std::filesystem::path&             a_targetFolderPath,
                                  const Enum::AssetBrowserPopupContextType a_contextPopup, 
                                  const bool                               a_canCreate,
                                        AssetBrowserEditorWindow&          a_assetBrowserEditorWindow) const;

        void DrawCreatePrefabMenu(const AssetBrowserEditorWindowAssetCreator&        a_assetCreator,
                                  const std::filesystem::path&                       a_targetFolderPath,
                                  const bool                                         a_canCreate,
                                        AssetFilePathRegistry&                       a_assetFilePathRegistry,
                                        Struct::AssetBrowserEditorWindowRenameState& a_renameState) const;

        void DrawCreateSceneMenu(const AssetBrowserEditorWindowAssetCreator&        a_assetCreator,
                                 const std::filesystem::path&                       a_targetFolderPath,
                                 const bool                                         a_canCreate,
                                       AssetFilePathRegistry&                       a_assetFilePathRegistry,
                                       Struct::AssetBrowserEditorWindowRenameState& a_renameState) const;

        void DrawRenameMenu(const std::filesystem::path& a_targetFilePath, const bool a_canRename, Struct::AssetBrowserEditorWindowRenameState& a_renameState) const;

        void DrawCopyMenu(const std::vector<std::filesystem::path>&    a_selectedFilePathList,
                          const bool                                   a_hasSelection, 
                                AssetBrowserEditorWindowFileOperation& a_fileOperation,
                                AssetBrowserEditorWindowClipboard&     a_clipboard) const;

        void DrawCutMenu(const std::vector<std::filesystem::path>&    a_selectedFilePathList,
                         const bool                                   a_hasSelection,
                               AssetBrowserEditorWindowFileOperation& a_fileOperation,
                               AssetBrowserEditorWindowClipboard&     a_clipboard) const;

        void DrawPasteMenu(const std::filesystem::path&                 a_targetFolderPath,
                           const bool                                   a_canPaste,
                                 AssetBrowserEditorWindowFileOperation& a_fileOperation,
                                 AssetBrowserEditorWindowClipboard&     a_clipboard) const;

        void DrawDuplicateMenu(const std::vector<std::filesystem::path>& a_selectedFilePathList, const bool a_hasSelection, AssetBrowserEditorWindowFileOperation& a_fileOperation) const;
        void DrawDeleteMenu   (const std::vector<std::filesystem::path>& a_selectedFilePathList, const bool a_hasSelection, AssetBrowserEditorWindow&              a_editorWindow) const;

        void StartRename(const std::filesystem::path& a_targetFilePath, Struct::AssetBrowserEditorWindowRenameState& a_renameState) const;

        static constexpr std::string_view k_imguiFontAwesomeFolderPlusIcon = "\xEF\x99\x9E";
        static constexpr std::string_view k_imguiFontAwesomeEditIcon       = "\xEF\x8B\x84";
        static constexpr std::string_view k_imguiFontAwesomeCopyIcon       = "\xEF\x83\x85";
        static constexpr std::string_view k_imguiFontAwesomeCutIcon        = "\xEF\x83\x84";
        static constexpr std::string_view k_imguiFontAwesomePasteIcon      = "\xEF\x83\xAA";
        static constexpr std::string_view k_imguiFontAwesomeCloneIcon      = "\xEF\x89\x8D";
        static constexpr std::string_view k_imguiFontAwesomeTrashIcon      = "\xEF\x87\xB8";
    
        static constexpr std::string_view k_createNewFolderLabel = "新規フォルダ";
        static constexpr std::string_view k_createNewPrefabLabel = "新規プレハブ";
        static constexpr std::string_view k_createNewSceneLabel  = "新規シーン";

        static constexpr std::string_view k_renameLabel    = "名前変更";
        static constexpr std::string_view k_copyLabel      = "コピー";
        static constexpr std::string_view k_cutLabel       = "切り取り";
        static constexpr std::string_view k_pasteLabel     = "貼り付け";
        static constexpr std::string_view k_duplicateLabel = "複製";
        static constexpr std::string_view k_deleteLabel    = "削除";

        static constexpr std::string_view k_createNewFolderShortcutLabel = "Ctrl + Shift + N";

        static constexpr std::string_view k_renameShortcutLabel    = "F2";
        static constexpr std::string_view k_copyShortcutLabel      = "Ctrl + C";
        static constexpr std::string_view k_cutShortcutLabel       = "Ctrl + X";
        static constexpr std::string_view k_pasteShortcutLabel     = "Ctrl + V";
        static constexpr std::string_view k_duplicateShortcutLabel = "Ctrl + D";
        static constexpr std::string_view k_deleteShortcutLabel    = "Del";
    };
}
