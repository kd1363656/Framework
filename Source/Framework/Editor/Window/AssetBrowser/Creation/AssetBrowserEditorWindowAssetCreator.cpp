#include "AssetBrowserEditorWindowAssetCreator.h"

FWK::Struct::AssetBrowserEditorWindowAssetCreationResult FWK::Editor::AssetBrowserEditorWindowAssetCreator::CreateFolder(const std::filesystem::path& a_parentFolderPath) const
{
    // フォルダはAssetではないためRegistryやWatcherは関与しない
    // 単純にディスク上へフォルダを作成するだけ
    // ResolveFilePathConflictByNumberSuffixで一意なファイルパスを作成する
    // 同名が存在する場合はNewFolder1,NewFolder2...と番号付与される
    const auto& l_folderPath = ResolveDefaultFilePath(a_parentFolderPath, {}, k_defaultFolderName);

    std::error_code l_errorCode = {};

    // create_directory()はシチエパスにフォルダを一つ作成する
    // 親フォルダが存在しない場合は失敗するが
    // AssetBrowserで操作される親フォルダはたんに存在する前提
    std::filesystem::create_directory(l_folderPath, l_errorCode);

    if (l_errorCode)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
                    "フォルダの作成に失敗しました。\nFolderPath : {}\nErrorCode : {}",
                    l_folderPath.string(),
                    l_errorCode.value());

        return {};
    }

    Struct::AssetBrowserEditorWindowAssetCreationResult l_result = {};

    l_result.m_createdFilePath = l_folderPath;
    l_result.m_isSuccess       = true;

    return l_result;
}
FWK::Struct::AssetBrowserEditorWindowAssetCreationResult FWK::Editor::AssetBrowserEditorWindowAssetCreator::CreatePrefab(const std::filesystem::path& a_parentFolderPath, AssetFilePathRegistry& a_assetFilePathRegistry) const
{
    // 一意なファイルパスを決定
    const auto& l_prefabFilePath = ResolveDefaultFilePath(a_parentFolderPath, Constant::k_lowerJsonExtension, k_defaultPrefabName);

    // PrefabUUIDを作成
    const auto& l_prefabUUID = UUIDManager::GetInstance().GenerateVALUUID();

    // Editor側のAssetFilePathRegistryへ先に登録
    // WatcherはRegistryへ未登録のJSOnがディスクに現れると物理削除するため
    // ファイルを書き込み前に必ずRegistryへ登録する
    // ファイルパスに対応するUUIDを生成数r
    if (!a_assetFilePathRegistry.Add(l_prefabFilePath, l_prefabUUID, Enum::AssetFilePathRegistryType::Prefab))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetFilePathRegistryへのPrefab登録に失敗したため、Prefabファイルを作成しませんでした。\nFilePath : {}", l_prefabFilePath.string());

        return {};
    }

    // シリアライズ用のダミーゲーむオブジェクトを作成
    auto l_gameObject = std::make_shared<GameObject>();

    l_gameObject->INIT         ();
    l_gameObject->SetPrefabUUID(l_prefabUUID);
    
    // Prefabを作成し、ファイルへ保存
    const auto&  l_prefabName = l_prefabFilePath.stem().string();
          Prefab l_prefab     = {};

    l_prefab.SetPrefabName(l_prefabName);
    l_prefab.SetGameObject(l_gameObject);

    // Prefab()::Save内部で以下の検査を行い保存する
    // GameObjectが有効か
    // PrefabUUIDが非nilか
    // SerializePrefab()が非nullか
    // 全て通ればファイルへ書き込む
    if (!l_prefab.Save(l_prefabFilePath))
    {
        // 保存に失敗したらRegistryから取り消す
        a_assetFilePathRegistry.Erase(l_prefabFilePath);

        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Prefabファイルの保存に失敗したため、Registry登録を取り消しました。\nFilePath : {}", l_prefabFilePath.string());

        return {};
    }

    // 作成パスを返却
    // 呼び出し側がこのパスを受け取り
    // m_renameStateへセットしてInputTextによる名前変更モードへ移行する
    // WatcherクラスがAdd通知意を検知 -> Registry命中 -> ApplyPrefabAdd
    // -> SceneManager側Registryへ登録 -> PrefabSystemへPrefabData追加
    Struct::AssetBrowserEditorWindowAssetCreationResult l_result = {};

    l_result.m_createdFilePath = l_prefabFilePath;
    l_result.m_isSuccess       = true;

    return l_result;
}
FWK::Struct::AssetBrowserEditorWindowAssetCreationResult FWK::Editor::AssetBrowserEditorWindowAssetCreator::CreateScene(const std::filesystem::path& a_parentFolderPath, AssetFilePathRegistry& a_assetFilePathRegistry) const
{
    // 一意なファイルパスを決定
    const auto& l_sceneFilePath = ResolveDefaultFilePath(a_parentFolderPath, Constant::k_lowerJsonExtension, k_defaultSceneName);

    // SceneUUIDを生成
    const auto& l_sceneUUID = UUIDManager::GetInstance().GenerateVALUUID();

    // AssetFilePathRegistryへ先登録
    // WatcherはRegistryへ未登録のJSONがディスクに現れると物理削除するため
    // ファイル書き込みの前に必ずRegistryへ登録する
    if (!a_assetFilePathRegistry.Add(l_sceneFilePath, l_sceneUUID, Enum::AssetFilePathRegistryType::Scene))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetFilePathRegistryへのScene登録に失敗したため、Sceneファイルを作成しませんでした。\nFilePath : {}", l_sceneFilePath.string());

        return {};
    }

    // ローカル空Sceneを作成
    const auto& l_sceneName = l_sceneFilePath.stem   ().string();
          auto  l_scene     = std::make_shared<Scene>();

    l_scene->INIT        ();
    l_scene->SetSceneName(l_sceneName);

    AssetFilePathRegistry l_emptyRegistry = {};

    // SceneManagerJsonConverter::SerializeSceneでJSONを生成
    // SerializeSceneはstaticメソッドなのでインスタンス不要
    // 空のAssetFilePathRegistryを渡す(新規Scene用のAssetがまだないため)
    // 新規からシーンとして必要な情報の身をロードできるようにする
    const auto& l_rootJson = Converter::SceneManagerJsonConverter::SerializeScene(l_scene, l_emptyRegistry);

    if (l_rootJson.is_null())
    {
        a_assetFilePathRegistry.Erase(l_sceneFilePath);


        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Sceneのシリアライズに失敗したため、Registry登録を取り消しました。\nFilePath : {}", l_sceneFilePath.string());

        return {};
    }

    // Utility::SaveJsonFileでファイルへ書き込み
    // SaveJsonFileは拡張子が.jsonかを検査し
    // 親フォルダが存在しない場合はcreate_directoriesで作成してから書き込む
    if (!Utility::SaveJsonFile(l_rootJson, l_sceneFilePath))
    {
        // ファイル書き込みに失敗した場合、
        // Registryだけにエントリが残るとWatcherが間違って
        // ファイルを削除しないようになるため、Registryから削除して登録前の状態へ戻す
        a_assetFilePathRegistry.Erase(l_sceneFilePath);

        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,"Sceneファイルの保存に失敗したため、Registry登録を取り消しました。\nFilePath : {}", l_sceneFilePath.string());

        return {};
    }

    Struct::AssetBrowserEditorWindowAssetCreationResult l_result = {};

    l_result.m_createdFilePath = l_sceneFilePath;
    l_result.m_isSuccess       = true;

    return l_result;
}

void FWK::Editor::AssetBrowserEditorWindowAssetCreator::RenamePrefab(const std::filesystem::path& a_oldFilePath, const std::filesystem::path& a_newFilePath) const
{
    // 既存のPrefabファイルを読み込む
    // Prefab::Loadは内部でPrefabJsonConverter::Loadを呼び
    // JSONからPrefabNameとGameObject情報を復元する
    Prefab l_prefab = {};

    // そのプレハブの編集内容を名前だけ変えて他の
    // パラメータを上書きしないように読みこむ
    l_prefab.Load(a_oldFilePath);

    // 新しいファイル名(stem)をPrefabNameとして設定
    // 例 : "Prefab.json" -> "Prefab"
    // これがJson内の"PrefabName"フィールドに保存される
    // PrefabJsonConverter::Save内部でk_prefabNameJsonKeyを使って書き込む
    const auto& l_newPrefabName = a_newFilePath.stem().string();

    // 名前だけ現在のPrefab.jsoのPrefab(stem)部分に変える
    l_prefab.SetPrefabName(l_newPrefabName);

    // 新しいパスへ保存
    // Prefab::Save -> PrefabJsonConverter::Save -> SaveJsonFile
    // 内部でPrefabNameとGameObjectJsonをJSONへ書き込む
    // UUIDはGameObjectが保持しているため変わらない
    l_prefab.Save(a_newFilePath);
}
void FWK::Editor::AssetBrowserEditorWindowAssetCreator::RenameScene(const std::filesystem::path& a_oldFilePath, const std::filesystem::path& a_newFilePath, const AssetFilePathRegistry& a_assetFilePathRegistry) const
{
    // 既存のSceneファイルを読み込む
    // Scene::Deserializeは内部でSceneJsonConverter::Deserializeを呼び
    // JSONからSceneNameとGameObjectListを復元する
    // AssetFilePathRegistryは空のものを渡す
    // (リネーム時はRegistryの内容は不要、名前変更だけが目的のため)
    auto l_scene = std::make_shared<Scene>();

    l_scene->INIT();

    const auto& l_rootJson = Utility::LoadJsonFile(a_oldFilePath);

    if (l_rootJson.is_null()) { return; }

    // シーンクラスのデシリアライズ処理
    // a_assetFilePathRegistryのファイルパスを参照してプレハブを読み込むため
    // ここには必ず全てのプレハブを保存しているAssetBrowser側のFilePathRegistryを使用すること
    Converter::SceneManagerJsonConverter::DeserializeScene(l_scene, l_rootJson, a_assetFilePathRegistry);

    const auto& l_newSceneName = a_newFilePath.stem().string();

    const auto& l_newRootJson = Converter::SceneManagerJsonConverter::SerializeScene(l_scene, a_assetFilePathRegistry);

    Utility::SaveJsonFile(l_newRootJson);
}

std::filesystem::path FWK::Editor::AssetBrowserEditorWindowAssetCreator::ResolveDefaultFilePath(const std::filesystem::path& a_parentFolderPath, const std::filesystem::path& a_extension, const std::string_view& a_defaultName)
{
    // デフォルト名 + 拡張子を統合した希望パスを作る
    const auto l_desiredFilePath = a_parentFolderPath / (std::string{ a_defaultName } + a_extension.string());

    return Utility::ResolveFilePathConflictByNumberSuffix(l_desiredFilePath);
}