#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowAssetPane final
    {
    public:

         AssetBrowserEditorWindowAssetPane() = default;
        ~AssetBrowserEditorWindowAssetPane() = default;

        void Draw(AssetBrowserEditorWindow& a_editorWindow);

    private:

        static constexpr std::string_view k_childLabel     = "##AssetBrowserEditorWindowRightPane";
        static constexpr std::string_view k_paneTitleLabel = "アセット";
    };
}