#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowFolderPane final
    {
    public:

         AssetBrowserEditorWindowFolderPane() = default;
        ~AssetBrowserEditorWindowFolderPane() = default;

        void Draw(const float a_paneWidth);

    private:

        static constexpr std::string_view k_childLabel      = "##AssetBrowserEditorWindowFolderPane";
        static constexpr std::string_view k_paneTitleLabel = "ファイル";
    };
}