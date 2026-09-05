#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindowDirectoryAddChange final : public AssetBrowserEditorWindowDirectoryChangeBase
	{
	public:

		 AssetBrowserEditorWindowDirectoryAddChange()          = default;
		~AssetBrowserEditorWindowDirectoryAddChange() override = default;

		void Apply(AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, SceneManager& a_sceneManager) override;
	};
}