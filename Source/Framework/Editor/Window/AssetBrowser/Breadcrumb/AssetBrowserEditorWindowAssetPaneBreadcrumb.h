#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindowAssetPaneBreadcrumb final
	{
	public:

		 AssetBrowserEditorWindowAssetPaneBreadcrumb() = default;
		~AssetBrowserEditorWindowAssetPaneBreadcrumb() = default;

		void Draw(AssetBrowserEditorWindow& a_editorWindow);

	private:

		static constexpr std::string_view k_childLabel     = "##AssetBrowserEditorWindowAssetPaneBreadcrumb";
		static constexpr std::string_view k_separatorLabel = " / ";
	};
}