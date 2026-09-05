#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindowDirectoryDeleteChange final : public AssetBrowserEditorWindowDirectoryChangeBase
	{
	public:

		 AssetBrowserEditorWindowDirectoryDeleteChange()          = default;
		~AssetBrowserEditorWindowDirectoryDeleteChange() override = default;

		void Apply(AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager) override;
	};
}