#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindow;
}

namespace FWK::Editor
{
    class AssetBrowserEditorWindowDeleteConfirmPopup final
    {
    public:

         AssetBrowserEditorWindowDeleteConfirmPopup() = default;
        ~AssetBrowserEditorWindowDeleteConfirmPopup() = default;

        void Request(const std::vector<std::filesystem::path>& a_filePathList, AssetBrowserEditorWindow& a_editorWindow) const;

        void Draw(AssetBrowserEditorWindow& a_editorWindow) const;

    private:

        static constexpr std::string_view k_childLabel   = "##AssetBrowserEditorWindowDeleteConfirmFileList";
        static constexpr std::string_view k_titleLabel   = "削除確認";
        static constexpr std::string_view k_messageLabel = "以下のファイル・フォルダを削除しますか？";
        static constexpr std::string_view k_deleteLabel  = "削除";
        static constexpr std::string_view k_cancelLabel  = "キャンセル";

        static constexpr float k_fileListChildHeight      = 200.0F;
        static constexpr float k_framePaddingBothSidesNUM = 2.0F;
    };
}