#include "AssetBrowserEditorWindowDirectoryAddChange.h"

void FWK::Editor::AssetBrowserEditorWindowDirectoryAddChange::Apply(AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, SceneManager& a_sceneManager)
{
	// Folder追加はAssetRegistryへ登録するAssetそのものではない
	if (GetVALIsDirectory()) { return; }

	const auto& l_filePath = GetREFFilePath();

	if (l_filePath.empty()) { return; }

	// .jsonといったプレハブやシーンなどを管理する形式でない場合は追加を許可
	if (l_filePath.extension() != Constant::k_lowerJsonExtension) { return; }

	// AssetBrowser側RegistryはProject全体のAsset情報を保持する正本
	// Editor内部でPrefab/SceneJsonを正式に作成した場合
	// WatcherがAdd通知をApplyする時点までに
	// このRegistryへ登録されている必要がある
	if (a_assetBrowserAssetFilePathRegistry.FindPTRAssetUUID(l_filePath)) { return; }

	const auto& l_sceneManagerAssetFilePathRegistry = a_sceneManager.GetREFAssetFilePathRegistry();

	// AssetBrowser側には存在しないのでSceneManager側だけに存在する場合正常な状態ではない
	// ただし、この状態dえFileを削除してしまうと
	// 内部Registryの同期不具合によって正規Assetそのものを消してしまう場合がある
	// そのためFileは残し、Registryの同期崩れとしてWarningを出す
	if (l_sceneManagerAssetFilePathRegistry.FindPTRAssetUUID(l_filePath))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "追加されたJsonファイルがSceneManagerのAssetFilePathRegistryには登録されていますが、AssetBrowserのAssetFilePathRegistryには登録されていません。\nAssetFilePathRegistryの同期状態を確認してください。\nFilePath : {}", l_filePath.string());

		return;
	}

	// CurrentScene自身はSceneManager側AssetFilePathRegistryには
	// 登録しない設計なのでCurrentSceneFilePathも確認する
	const auto& l_currentSceneFilePath = a_sceneManager.GetREFCurrentSceneFilePath();

	if (l_currentSceneFilePath == l_filePath)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "追加されたJsonファイルがCurrentSceneFilePathと一致していますが、AssetBrowserのAssetFilePathRegistryには登録されていません。\nAssetFilePathRegistryの同期状態を確認してください。\nFilePath : {}", l_filePath.string());

		return;
	}

	// Prefab/SceneJsonはEditor内部の専用処理から作成する設計なので
	// 外部から直接追加されたJsonはとして削除する
	FWK_ADD_LOG(Constant::k_debugWarningColor, "Prefab、Scene等のJsonファイルはEditor内部での作成を想定しています。\n外部から追加されたJsonファイルを削除します。\nFilePath : {}", l_filePath.string());

	std::error_code l_errorCode = {};
	
	// 実際にAssetRegistryに含まれないJsonファイルを削除する
	std::filesystem::remove(l_filePath, l_errorCode);

	if (!l_errorCode) { return; }

	FWK_ADD_LOG(Constant::k_debugWarningColor, "Prefab、Scene等のJsonファイルはEditor内部での作成を想定しています。\n外部から追加されたJsonファイルを削除します。\nFilePath : {}", l_filePath.string());
}