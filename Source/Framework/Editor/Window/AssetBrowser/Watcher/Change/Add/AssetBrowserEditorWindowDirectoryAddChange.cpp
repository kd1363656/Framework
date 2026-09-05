#include "AssetBrowserEditorWindowDirectoryAddChange.h"

void FWK::Editor::AssetBrowserEditorWindowDirectoryAddChange::Apply(AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager)
{
	// Folder追加はAssetRegistryへ登録するAssetそのものではない
	if (GetVALISDirectory()) { return; }

	const auto& l_filePath = GetREFFilePath();

	// .jsonといったプレハブやシーンなどを管理する形式でない場合は追加を許可
	if (l_filePath.extension() != Constant::k_lowerJsonExtension) { return; }

	// Editor内部でPrefabやSceneJsonを正式に作成した場合は、
	// Watcherが通知を処理するまでにAssetFilePathRegistryへ登録されていることを前提とする
	// 既にRegistry登録されているJsonなら
	// Editor内部で正式に作られたものなので削除しない
	if (a_assetFilePathRegistry.FindPTRAssetUUID(l_filePath)) { return; }

	FWK_ADD_LOG(Constant::k_debugWarningColor, "Prefab、Scene等のJsonファイルはEditor内部での作成を想定しています。\n外部から追加されたJsonファイルを削除します。\nFilePath : {}", l_filePath.string());

	std::error_code l_errorCode = {};

	// ファイルを削除する、使えないJsonファイルがあったらエディター使用者が使えなかった場合
	// バグだと勘違いするため
	std::filesystem::remove(l_filePath, l_errorCode);

	if (!l_errorCode) { return; }

	FWK_ADD_LOG(Constant::k_debugWarningColor, "外部から追加されたJsonファイルの削除に失敗しました。\nFilePath : {}", l_filePath.string());
}