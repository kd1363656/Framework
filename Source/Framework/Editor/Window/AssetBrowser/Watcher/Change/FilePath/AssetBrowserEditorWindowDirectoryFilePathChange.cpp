#include "AssetBrowserEditorWindowDirectoryFilePathChange.h"

void FWK::Editor::AssetBrowserEditorWindowDirectoryFilePathChange::Apply(AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, SceneManager& a_sceneManager)
{
	const auto& l_oldFilePath = GetREFFilePath();

	if (l_oldFilePath.empty() ||
		m_newFilePath.empty())
	{
		return;
	}

	// OldPathとNewPathが同じなら内部情報を書き換える必要はない
	if (l_oldFilePath == m_newFilePath) { return; }

	if (GetVALIsDirectory())
	{
		ApplyDirectoryFilePathChange(l_oldFilePath,
			                         m_newFilePath,
			                         a_assetBrowserAssetFilePathRegistry,
			                         a_sceneManager);

		return;
	}

	ApplyFilePathChange(l_oldFilePath, 
		                m_newFilePath,
		                a_assetBrowserAssetFilePathRegistry, 
		                a_sceneManager);
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryFilePathChange::ApplyFilePathChange(const std::filesystem::path& a_oldFilePath, 
	                                                                                   const std::filesystem::path& a_newFilePath, 
	                                                                                         AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, 
	                                                                                         SceneManager&          a_sceneManager) const
{
	// AssetBrowser側RegistryはProject全体のAsset情報を持つため、
	// ここからUUIDとAssetTypeを取得して処理を分ける
	const auto* l_assetUUID = a_assetBrowserAssetFilePathRegistry.FindPTRAssetUUID(a_oldFilePath);

	if (!l_assetUUID)
	{
		// CurrentSceneはSceneManager側AssetRegistryには登録しないため
		// AssetBrowser側Registryの情報が何らかの理由で失われても
		// CurrentScene自身のPath変更だけは検出できるようにする
		if (a_sceneManager.GetREFCurrentSceneFilePath() == a_oldFilePath)
		{
			a_sceneManager.SetCurrentSceneFilePath(a_newFilePath);
		}

		return;
	}

	// ReplaceFilePath()によってRegistry内部の要素が移動するため
	// Map内部を示すPointerではなくUUIDを値として保持しておく
	const auto  l_copiedAssetUUID   = *l_assetUUID;
	const auto* l_assetFilePathData = a_assetBrowserAssetFilePathRegistry.FindPTRAssetFilePathData(l_copiedAssetUUID);

	if (!l_assetFilePathData)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor,
			        "AssetBrowserのAssetFilePathRegistry内部で、UUIDに対応するAssetFilePathDataを取得できませんでした。\nOldFilePath : {}\nNewFilePath : {}",
			        a_oldFilePath.string(),
			        a_newFilePath.string());

		return;
	}

	switch (l_assetFilePathData->m_type)
	{
		case Enum::AssetFilePathRegistryType::Prefab:
		{
			ApplyPrefabFilePathChange(
				a_oldFilePath,
				a_newFilePath,
				l_copiedAssetUUID,
				a_assetBrowserAssetFilePathRegistry,
				a_sceneManager);

			return;
		}
		break;

		case Enum::AssetFilePathRegistryType::Scene:
		{
			ApplySceneFilePathChange(
				a_oldFilePath,
				a_newFilePath,
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
void FWK::Editor::AssetBrowserEditorWindowDirectoryFilePathChange::ApplyPrefabFilePathChange(const std::filesystem::path& a_oldFilePath, 
	                                                                                         const std::filesystem::path& a_newFilePath,
	                                                                                         const boost::uuids::uuid&    a_prefabUUID,
	                                                                                               AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, 
	                                                                                               SceneManager&          a_sceneManager) const
{
	if (a_prefabUUID.is_nil()) { return; }

	auto& l_sceneManagerAssetFilePathRegistry = a_sceneManager.GetMutableREFAssetFilePathRegistry();

	// SceneManager側RegistryにOldPathが存在する場合
	// 現在Sceneで使用中のPrefabとして登録されている
	const auto* l_prefabUUID = l_sceneManagerAssetFilePathRegistry.FindPTRAssetUUID(a_oldFilePath);

	if (l_prefabUUID)
	{
		// Registryに書き換える前にUUIDをコピーする
		const auto  l_copiedPrefabUUID  = *l_prefabUUID;
		const auto* l_assetFilePathData = l_sceneManagerAssetFilePathRegistry.FindPTRAssetFilePathData(l_copiedPrefabUUID);

		if (!l_assetFilePathData)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor,"SceneManagerのAssetFilePathRegistry内部で、PrefabUUIDに対応するAssetFilePathDataを取得できませんでした。\nFilePath : {}", a_oldFilePath.string());

			return;
		}

		if (l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Prefab)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "SceneManagerのAssetFilePathRegistryに登録されているAssetTypeがPrefabではありません。\nFilePath : {}", a_oldFilePath.string());

			return;
		}

		// AssetBrowserとSceneManagerで同じPrefabPathを示しているなら
		// UUIDも一致していなければならない
		if (l_copiedPrefabUUID != a_prefabUUID)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "AssetBrowserとSceneManagerのAssetFilePathRegistryでPrefabUUIDが一致していません。\nFilePath : {}", a_oldFilePath.string());

			return;
		}

		// SceneManager側を先に変更する
		if (!l_sceneManagerAssetFilePathRegistry.ReplaceFilePath(a_oldFilePath, a_newFilePath)) { return; }

		// AssetBrowser側も同じPathへ変更する
		if (!a_assetBrowserAssetFilePathRegistry.ReplaceFilePath(a_oldFilePath, a_newFilePath))
		{
			// AssetBrowser側の変更だけ失敗した場合は、
			// SceneMaanger側だけnewPathになる状態を防ぐためOldPathへ戻す
			l_sceneManagerAssetFilePathRegistry.ReplaceFilePath(a_newFilePath, a_oldFilePath);

			return;
		}

		return;
	}

	// SceneManagerに存在しないPrefabは現在Sceneで使用していないPrefab
	// Project全体を管理するAssetBrowser側Registryだけ変更する
	a_assetBrowserAssetFilePathRegistry.ReplaceFilePath(a_oldFilePath, a_newFilePath);
}
void FWK::Editor::AssetBrowserEditorWindowDirectoryFilePathChange::ApplySceneFilePathChange(const std::filesystem::path& a_oldFilePath, 
	                                                                                        const std::filesystem::path& a_newFilePath,
	                                                                                        const boost::uuids::uuid&    a_sceneUUID, 
	                                                                                              AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, 
	                                                                                              SceneManager&          a_sceneManager) const
{
	if (a_sceneUUID.is_nil()) { return; }

	const auto& l_sceneManagerAssetFilePathRegistry = a_sceneManager.GetMutableREFAssetFilePathRegistry();

	// SceneManager側AssetRegistryにはCurrentScene自身を登録せず
	// NextSceneだけを問う臆する
	const auto*              l_sceneManagerSceneUUID      = l_sceneManagerAssetFilePathRegistry.FindPTRAssetUUID(a_oldFilePath);
	      bool               l_isNextSceneFilePathChanged = false;
	      boost::uuids::uuid l_nextSceneUUID              = {};

	if (l_sceneManagerSceneUUID)
	{
		l_nextSceneUUID = *l_sceneManagerSceneUUID;

		const auto* l_sceneManagerAssetFilePathData = l_sceneManagerAssetFilePathRegistry.FindPTRAssetFilePathData(l_nextSceneUUID);

		if (!l_sceneManagerAssetFilePathData)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "SceneManagerのAssetFilePathRegistry内部で、SceneUUIDに対応するAssetFilePathDataを取得できませんでした。\nFilePath : {}", a_oldFilePath.string());

			return;
		}

		if (l_sceneManagerAssetFilePathData->m_type != Enum::AssetFilePathRegistryType::Scene)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "SceneManagerのAssetFilePathRegistryに登録されているAssetTypeがSceneではありません。\nFilePath : {}", a_oldFilePath.string());

			return;
		}

		if (l_nextSceneUUID != a_sceneUUID)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "AssetBrowserとSceneManagerのAssetFilePathRegistryでSceneUUIDが一致していません。\nFilePath : {}", a_oldFilePath.string());

			return;
		}

		// ReplaceSceneFilePath()内部で
		// SceneManagerAssetFilePathRegistry
		// NextSceneLoadFilePathMapの両方を同時に変更する
		if (!a_sceneManager.ReplaceSceneFilePath(a_oldFilePath, a_newFilePath, l_nextSceneUUID)) { return; }

		l_isNextSceneFilePathChanged = true;
	}

	if (!a_assetBrowserAssetFilePathRegistry.ReplaceFilePath(a_oldFilePath, a_newFilePath))
	{
		// NextScene側だけ変更済みならOldPathへRollbackする
		if (l_isNextSceneFilePathChanged)
		{
			a_sceneManager.ReplaceSceneFilePath(a_newFilePath, a_oldFilePath, l_nextSceneUUID);
		}

		return;
	}

	// CurrentSceneはSceneManager側AssetRegistry二は存在しないため
	// FilePathそのものと一致によって判定する
	if (a_sceneManager.GetREFCurrentSceneFilePath() == a_oldFilePath)
	{
		a_sceneManager.SetCurrentSceneFilePath(a_newFilePath);
	}

}
void FWK::Editor::AssetBrowserEditorWindowDirectoryFilePathChange::ApplyDirectoryFilePathChange(const std::filesystem::path& a_oldFilePath, 
	                                                                                            const std::filesystem::path& a_newFilePath, 
	                                                                                                  AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, 
	                                                                                                  SceneManager&          a_sceneManager) const
{
	// Registryを走査中にReplaceFilePath()すると
	// unordered_map内部の要素が変更されるため、
	// まず変更対象となるOldPathだけを別Containerへコピーする
	      std::unordered_set<std::filesystem::path> l_oldeAssetFilePathSet               = {};
	const auto&                                     l_assetBrowserAssetFilePathToUUIDMap = a_assetBrowserAssetFilePathRegistry.GetREFAssetFilePathToUUIDMap();

	for(const auto& [l_assetFilePath, l_uuid] : l_assetBrowserAssetFilePathToUUIDMap)
	{
		if (!IsChildFilePath(l_assetFilePath, a_oldFilePath)) { continue; }

		l_oldeAssetFilePathSet.emplace(l_assetFilePath);
	}

	const auto& l_sceneManagerAssetFilePathRegistry  = a_sceneManager.GetREFAssetFilePathRegistry                      ();
	const auto& l_sceneManagerAssetFilePathToUUIDMap = l_sceneManagerAssetFilePathRegistry.GetREFAssetFilePathToUUIDMap();

	for (const auto& [l_assetFilePath, l_uuid] : l_sceneManagerAssetFilePathToUUIDMap)
	{
		if (!IsChildFilePath(l_assetFilePath, a_oldFilePath)) { continue; }

		// AssetBrowser側にも同じPathが存在する場合は
		// unordered_setなので重複登録されない
		l_oldeAssetFilePathSet.emplace(l_assetFilePath);
	}

	const auto& l_currentSceneFilePath = a_sceneManager.GetREFCurrentSceneFilePath();

	// CurrentSceneはSceneManager側Registryには入らないので
	// Folderは以下なら独立して追加しておく
	if (!l_currentSceneFilePath.empty() &&
		IsChildFilePath(l_currentSceneFilePath, a_oldFilePath))
	{
		l_oldeAssetFilePathSet.emplace(l_currentSceneFilePath);
	}

	for (const auto& l_oldAssetFilePath : l_oldeAssetFilePathSet)
	{
		// OldDirectoryから見た相対Pathを取得する
		// 例 : OldeDirectory : Asset/Scene
		//      Asset         : Asset/Scene/game/Main.json
		//      relative      : Game/Main.json
		const auto& l_relativeAssetFilePath = l_oldAssetFilePath.lexically_relative(a_oldFilePath);

		if (l_relativeAssetFilePath.empty()) { continue; }

		// NewDirectoryへ同じ相対階層をつけなおす
		// Asset/Scene
		// Asset/NewScene
		// Game/Main.json
		// Asset/NewScene/Game/Main.jsonという形にする
		const auto& l_newAssetFilePath = a_newFilePath / l_relativeAssetFilePath;

		ApplyFilePathChange(l_oldAssetFilePath,
			                l_newAssetFilePath,
			                a_assetBrowserAssetFilePathRegistry,
			                a_sceneManager);
	}
}