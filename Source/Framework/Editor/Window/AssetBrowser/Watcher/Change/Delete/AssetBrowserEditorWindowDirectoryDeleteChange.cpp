#include "AssetBrowserEditorWindowDirectoryDeleteChange.h"

void FWK::Editor::AssetBrowserEditorWindowDirectoryDeleteChange::Apply(AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, SceneManager& a_sceneManager)
{
	const auto& l_deletedFilePath = GetREFFilePath();

	if (l_deletedFilePath.empty()) { return; }

	if (GetVALIsDirectory())
	{
		ApplyDirectoryDelete(l_deletedFilePath, a_assetBrowserAssetFilePathRegistry, a_sceneManager);

		return;
	}

	ApplyFileDelete(l_deletedFilePath, a_assetBrowserAssetFilePathRegistry, a_sceneManager);
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryDeleteChange::ApplyFileDelete(const std::filesystem::path& a_deleteFilePath, AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, SceneManager& a_sceneManager) const
{
	// AssetBrowser側RegistryからAsset情報を取得
	if(const auto* l_assetUUID = a_assetBrowserAssetFilePathRegistry.FindPTRAssetUUID(a_deleteFilePath);
		l_assetUUID)
	{
		// この後RegistryからEraseされる可能性があるため
		// Map内部を示すPointerのまま保持せずUUIDを値としてコピーする
		const auto  l_copiedAssetUUID   = *l_assetUUID;
		const auto* l_assetFilePathData = a_assetBrowserAssetFilePathRegistry.FindPTRAssetFilePathData(l_copiedAssetUUID);

		if (!l_assetFilePathData)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "AssetBrowserのAssetFilePathRegistry内部で、UUIDに対応するAssetFilePathDataを取得できませんでした\nFilePath : {}。", a_deleteFilePath.string());

			return;
		}

		switch (l_assetFilePathData->m_type)
		{
			case Enum::AssetFilePathRegistryType::Prefab:
			{
				ApplyPrefabDelete(a_deleteFilePath, 
					              l_copiedAssetUUID, 
					              a_assetBrowserAssetFilePathRegistry,
					              a_sceneManager);

				return;
			}
			break;

			case Enum::AssetFilePathRegistryType::Scene:
			{
				ApplySceneDelete(a_deleteFilePath, 
					             l_copiedAssetUUID, 
					             a_assetBrowserAssetFilePathRegistry,
					             a_sceneManager);

				return;
			}
			break;

			default:
			break;
		}
	}

	// AssetBrowserRegistryにない場合のCurrentScene
	// 本来CurrentSceneもAssetRegistryへ登録される設計だが
	// Registry情報が変えていた場合でも
	// 現在Scene自身のJsonが物理削除されたことはPathから判断できる
	if (a_sceneManager.GetREFCurrentSceneFilePath() == a_deleteFilePath)
	{
		// 空のファイルパスをセット
		a_sceneManager.SetCurrentSceneFilePath({});
	}
}
void FWK::Editor::AssetBrowserEditorWindowDirectoryDeleteChange::ApplyPrefabDelete(const std::filesystem::path& a_deleteFilePath, 
	                                                                               const boost::uuids::uuid&    a_prefabUUID,
	                                                                                     AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, 
	                                                                                     SceneManager&          a_sceneManager) const
{
	if (a_prefabUUID.is_nil()) { return; }

	if (const auto& l_scene = a_sceneManager.GetVALScene().lock();
		l_scene)
	{
		// PrefabSystemはPrefabUUIDをKeyとして管理している
		// PrefabJsonそのものが削除されたので、
		// 現在Sceneで保持しているPrefab情報もUUIDで削除する
		auto& l_prefabSystem = l_scene->GetMutableREFPrefabSystem();

		l_prefabSystem.RemovePrefab(a_prefabUUID);
	}

	auto& l_sceneManagerAssetFilePathRegistry = a_sceneManager.GetMutableREFAssetFilePathRegistry();

	// シーンマネージャーのアセットファイルパスレジストリーに要素が存在すれば削除
	if (l_sceneManagerAssetFilePathRegistry.FindPTRAssetUUID(a_deleteFilePath))
	{
		l_sceneManagerAssetFilePathRegistry.Erase(a_deleteFilePath);
	}

	// AssetBrowser側のAssetFilePathRegistryからも削除
	a_assetBrowserAssetFilePathRegistry.Erase(a_deleteFilePath);
}
void FWK::Editor::AssetBrowserEditorWindowDirectoryDeleteChange::ApplySceneDelete(const std::filesystem::path& a_deleteFilePath, 
	                                                                              const boost::uuids::uuid&    a_sceneUUID, 
	                                                                                    AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, 
	                                                                                    SceneManager&          a_sceneManager) const
{
	if (a_sceneUUID.is_nil()) { return; }

	auto& l_sceneManagerAssetFilePathRegistry = a_sceneManager.GetMutableREFAssetFilePathRegistry();
	
	if (const auto* l_sceneManagerSceneUUID = l_sceneManagerAssetFilePathRegistry.FindPTRAssetUUID(a_deleteFilePath);
		l_sceneManagerSceneUUID)
	{
		// SceneManager側AssetRegistryんいは
		// CurrentScene自身は登録せず、NextSceneだけを登録する
		const auto  l_nextSceneUUID     = *l_sceneManagerSceneUUID;
		const auto* l_assetFilePathData = l_sceneManagerAssetFilePathRegistry.FindPTRAssetFilePathData(l_nextSceneUUID);

		if (l_assetFilePathData &&
			l_assetFilePathData->m_type == Enum::AssetFilePathRegistryType::Scene)
		{
			// NextSceneLoadFilePathMapから削除する
			a_sceneManager.RemoveNextSceneLoadFilePath(l_nextSceneUUID);
		}
	}

	// CurrentSceneはNextSceneのMapに存在しない
	// そのためm_currentSceneFilePathの一致によって判定する
	if (a_sceneManager.GetREFCurrentSceneFilePath() == a_deleteFilePath)
	{
		// 元のSceneJsonが物理的に存在しなくなったため
		// SaveScene()等が古いPathへ再保存しないいように無効化する
		a_sceneManager.SetCurrentSceneFilePath({});
	}

	// AssetBrowser側Registry空も削除
	a_assetBrowserAssetFilePathRegistry.Erase(a_deleteFilePath);
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryDeleteChange::ApplyDirectoryDelete(const std::filesystem::path& a_deleteFilePath, AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, SceneManager& a_sceneManager) const
{
	// Folder配下に存在していたPrefabやSceneJsonを集め
	// File単体削除と同じ処理を流す
	std::unordered_set<std::filesystem::path> l_deleteAssetFilePathSet = {};

	const auto& l_assetBrowserAssetFilePathToUUIDMap = a_assetBrowserAssetFilePathRegistry.GetREFAssetFilePathToUUIDMap();

	for (const auto& l_assetFilePathToUUID : l_assetBrowserAssetFilePathToUUIDMap)
	{
		const auto& l_assetFilePath = l_assetFilePathToUUID.first;

		// ファイルパスの階層よりも下でないファイルパスなら処理を飛ばす
		if (!IsChildFilePath(l_assetFilePath, a_deleteFilePath)) { continue; }

		l_deleteAssetFilePathSet.emplace(l_assetFilePath);
	}

	const auto& l_sceneManagerAssetFilePathRegistry  = a_sceneManager.GetREFAssetFilePathRegistry                      ();
	const auto& l_sceneManagerAssetFilePathToUUIDMap = l_sceneManagerAssetFilePathRegistry.GetREFAssetFilePathToUUIDMap();

	for (const auto& [l_assetFilePath, l_uuid] : l_sceneManagerAssetFilePathToUUIDMap)
	{
		// ファイル階層がこの削除ファイルパス以下のファイルでない場合処理とを飛ばす
		if (!IsChildFilePath(l_assetFilePath, a_deleteFilePath)) { continue; }

		// unordered_setなので
		// AssetBrowserRegistry二も同じPathが存在していても
		// 一つだけ保持される
		l_deleteAssetFilePathSet.emplace(l_assetFilePath);
	}

	const auto& l_currentSceneFilePath = a_sceneManager.GetREFCurrentSceneFilePath();

	// CurrentSceneはSceneManagerRegistryに存在しない多恵
	// Directoryは以下なら別途Pah一覧へ追加する
	if (!l_currentSceneFilePath.empty() && 
		IsChildFilePath(l_currentSceneFilePath, a_deleteFilePath))
	{
		l_deleteAssetFilePathSet.emplace(l_currentSceneFilePath);
	}

	// 再帰的に処理を行う
	for (const auto& l_deleteAssetFilePath : l_deleteAssetFilePathSet)
	{
		ApplyFileDelete(l_deleteAssetFilePath, a_assetBrowserAssetFilePathRegistry, a_sceneManager);
	}
}