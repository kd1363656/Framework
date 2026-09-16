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

        static constexpr std::string_view k_childLabel     = "##AssetBrowserEditorWindowAssetPane";
        static constexpr std::string_view k_paneTitleLabel = "アセット";

        AssetBrowserEditorWindowAssetPaneBreadcrumb m_breadcrumb = {};
    };
}