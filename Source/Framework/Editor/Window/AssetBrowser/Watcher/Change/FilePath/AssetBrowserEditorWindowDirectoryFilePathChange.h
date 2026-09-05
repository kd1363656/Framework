#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindowDirectoryFilePathChange final : public AssetBrowserEditorWindowDirectoryChangeBase
	{
	public:

		 AssetBrowserEditorWindowDirectoryFilePathChange()          = default;
		~AssetBrowserEditorWindowDirectoryFilePathChange() override = default;

		void Apply(AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager) override;

		void SetNewFilePath(const std::filesystem::path& a_set) { m_newFilePath = a_set; }

	private:

		std::filesystem::path m_newFilePath = {};
	};
}