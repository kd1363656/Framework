#include "AssetBrowserEditorWindowDirectoryAddChange.h"

void FWK::Editor::AssetBrowserEditorWindowDirectoryAddChange::Apply(AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, SceneManager& a_sceneManager)
{
	// Folder追加はAssetRegistryへ登録するAssetそのものではない
	if (GetVALIsDirectory()) { return; }

	const auto& l_filePath = GetREFFilePath();

	if (l_filePath.empty()) { return; }

	// 現在AssetFilePathRegistryで管理しているPrefab/SceneはJsonなので
	// Json以外のFile追加はこのChangeでは同期対象にしない
	if (l_filePath.extension() != Constant::k_lowerJsonExtension) { return; }

	// AssetBrowser側RegistryはProject全体のPrefab / Scene情報を保持する正本
	// Editor内部からPrefab / Sceneを作成する場合は
	// Fileを物理作成する前にUUIDとTypeをAssetBrowser側Registryへ正式登録しておく
	const auto& l_assetUUID = a_assetBrowserAssetFilePathRegistry.FindPTRAssetUUID(l_filePath);

	if (!l_assetUUID)
	{
		const auto& l_sceneManagerAssetFilePathRegistry = a_sceneManager.GetREFAssetFilePathRegistry();
	}
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryAddChange::ApplyPrefabAdd(const std::filesystem::path& a_filePath, const boost::uuids::uuid& a_prefabUUID, SceneManager& a_sceneManager) const
{
}
void FWK::Editor::AssetBrowserEditorWindowDirectoryAddChange::ApplySceneAdd(const std::filesystem::path& a_filePath, const boost::uuids::uuid& a_sceneUUID, SceneManager& a_sceneManager) const
{
}