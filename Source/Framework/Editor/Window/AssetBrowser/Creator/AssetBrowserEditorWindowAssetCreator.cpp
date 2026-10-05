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
          auto& l_uuidManager = Utility::UUIDManager::GetInstance();
    const auto& l_prefabUUID  = l_uuidManager.GenerateVALUUID    ();

    // Editor側のAssetFilePathRegistryへ先に登録
    // WatcherはRegistryへ未登録のJSOnがディスクに現れると物理削除するため
    // ファイルを書き込み前に必ずRegistryへ登録する
    // ファイルパスに対応するUUIDを生成数r
    if (!a_assetFilePathRegistry.Add(l_prefabFilePath, l_prefabUUID, Enum::AssetFilePathRegistryType::Prefab))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetFilePathRegistryへのPrefab登録に失敗したため、Prefabファイルを作成しませんでした。\nFilePath : {}", l_prefabFilePath.string());

        return {};
    }

    // 空のGameObjectを保持したPrefabファイルを作成する
    // GameObjectはINIT内でweak_from_thisを使うため
    // shared_ptr管理でなければbad_weak_ptrが投げられる
    const auto& l_gameObject = std::make_shared<GameObject>();
     
    l_gameObject->INIT   ();
    l_gameObject->SetName(l_prefabFilePath.stem().string());
     
    // 新規の空GameObjectにはPrefabインスタンスの子が存在しないため
    // シリアライズ用の一時的なPrefabSystemで十分
    SceneGameObjectPrefabSystem l_prefabSystem = {};
     
    l_prefabSystem.INIT();
     
    // Save内部でConvertToPrefabが呼ばれ
    // GameObjectと子孫へPrefabUUID/IsPrefabOriginが設定されてから
    // GameObjectJsonConverter::Serializeでフル形式のPrefabJsonが作られる
    if (GameObjectPrefab l_gameObjectPrefab = {};
        !l_gameObjectPrefab.Save(l_prefabFilePath,
                                 l_prefabUUID,
                                 l_prefabSystem,
                                 *l_gameObject))
    {
        // Registryだけにエントリが残るとWatcherが間違って
        // ファイルを削除しないようになるため、Registryから削除して登録前の状態へ戻す
        a_assetFilePathRegistry.Erase(l_prefabFilePath);
     
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Prefabファイルの保存に失敗したため、Registry登録を取り消しました。\nFilePath : {}", l_prefabFilePath.string());
     
        return {};
    }
     
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
    auto& l_uuidManager = Utility::UUIDManager::GetInstance();
    
    // AssetFilePathRegistryへ先登録
    // WatcherはRegistryへ未登録のJSONがディスクに現れると物理削除するため
    // ファイル書き込みの前に必ずRegistryへ登録する
    if (const auto& l_sceneUUID = l_uuidManager.GenerateVALUUID    ();
        !a_assetFilePathRegistry.Add(l_sceneFilePath, l_sceneUUID, Enum::AssetFilePathRegistryType::Scene))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetFilePathRegistryへのScene登録に失敗したため、Sceneファイルを作成しませんでした。\nFilePath : {}", l_sceneFilePath.string());

        return {};
    }

    // ローカル空Sceneを作成
    const auto&  l_sceneName = l_sceneFilePath.stem   ().string();
          Scene  l_scene = {};

    l_scene.INIT   ();
    l_scene.SetName(l_sceneName);

    AssetFilePathRegistry l_emptyRegistry = {};

    // SceneManagerJsonConverter::SerializeSceneでJSONを生成
    // SerializeSceneはstaticメソッドなのでインスタンス不要
    // 空のAssetFilePathRegistryを渡す(新規Scene用のAssetがまだないため)
    // 新規からシーンとして必要な情報の身をロードできるようにする
    const auto& l_rootJson = l_scene.Serialize();

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

std::vector<FWK::Struct::AssetBrowserEditorWindowAssetCreationResult> FWK::Editor::AssetBrowserEditorWindowAssetCreator::CreatePrefabFromGameObjectDrop(const std::weak_ptr<GameObject>&         a_droppedGameObject, 
                                                                                                                                                        const std::filesystem::path&             a_parentFolderPath, 
                                                                                                                                                                          Scene&                 a_scene,
                                                                                                                                                                          AssetFilePathRegistry& a_assetFilePathRegistry) const
{
    // ドロップ対象が選択リストに含まれていれば選択中全てをPrefab化対象にする
    // 親子を同時選択していた場合は子を除外する
    // (親のPrefab内部ノードとして既に含まれるため)
    const auto& l_targetList = CollectPrefabTargetGameObjectList(a_droppedGameObject);
 
    std::vector<Struct::AssetBrowserEditorWindowAssetCreationResult> l_resultList = {};
 
    l_resultList.reserve(l_targetList.size());
 
    for (const auto& l_gameObject : l_targetList)
    {
        const auto& l_prefab = CreatePrefabFromGameObject(l_gameObject,
                                                          a_parentFolderPath,
                                                          a_scene,
                                                          a_assetFilePathRegistry);

        l_resultList.emplace_back(l_prefab);
    }
 
    return l_resultList;
}

void FWK::Editor::AssetBrowserEditorWindowAssetCreator::RenamePrefab(const std::filesystem::path& a_oldFilePath, const std::filesystem::path& a_newFilePath) const
{
    // 新しいファイル名(stem)をPrefabNameとして設定
    // 例 : "Prefab.json" -> "Prefab"
    // これがJson内の"PrefabName"フィールドに保存される
    // PrefabJsonConverter::Save内部でk_prefabNameJsonKeyを使って書き込む
    const auto& l_newPrefabName = a_newFilePath.stem().string();

    // 古いファイルパスから新しいファイルパスに変更(NewPrefab.jsonがPrefab.jsonといった具合でファイル名が変わればパスも変わるから)
    // 新しいプレハブ名をJSONファイルに反映する
    Converter::GameObjectPrefabJsonConverter::Rename(a_oldFilePath, a_newFilePath, l_newPrefabName);
}
void FWK::Editor::AssetBrowserEditorWindowAssetCreator::RenameScene(const std::filesystem::path& a_oldFilePath, const std::filesystem::path& a_newFilePath) const
{
    Scene l_scene = {};

    l_scene.INIT();

    auto l_deserializedJson = Utility::LoadJsonFile(a_oldFilePath);

    if (l_deserializedJson.is_null()) { return; }

    l_scene.Deserialize(l_deserializedJson);

    const auto& l_sceneName = a_newFilePath.stem().string();

    l_scene.SetName(l_sceneName);

    nlohmann::json l_serializedJson = {};

    l_serializedJson = l_scene.Serialize();

    if (l_serializedJson.is_null())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Sceneのシリアライズに失敗しました。\nFilePath : {}", a_newFilePath.string());

        return;
    }

    if (!Utility::SaveJsonFile(l_serializedJson, a_newFilePath))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Sceneファイルの保存に失敗しました。\nFilePath : {}", a_newFilePath.string());
    }
}

bool FWK::Editor::AssetBrowserEditorWindowAssetCreator::RegisterCopiedAsset(const std::filesystem::path& a_sourceFilePath, const std::filesystem::path& a_copiedFilePath, AssetFilePathRegistry& a_assetFilePathRegistry) const
{
    // コピー元がRegistry未登録ならアセットではないため何もしない
    const auto* l_sourceUUIDPTR = a_assetFilePathRegistry.FindPTRAssetUUID(a_sourceFilePath);

    if (!l_sourceUUIDPTR) { return false; }

    const auto* l_sourceAssetFilePathDataPTR = a_assetFilePathRegistry.FindPTRAssetFilePathData(*l_sourceUUIDPTR);

    if (!l_sourceAssetFilePathDataPTR) { return false; }

    // Add()でRegistryへ要素を追加する前に値としてコピーしておく
    // (unordered_mapの要素へのポインタはAdd後も有効だが
    //  Registryの内部実装へ依存しないよう値で保持する)
    const auto l_sourceUUID = *l_sourceUUIDPTR;
    const auto l_assetType  = l_sourceAssetFilePathDataPTR->m_type;

    // コピー先は別アセットなので新しいUUIDを発行する
          auto& l_uuidManager = Utility::UUIDManager::GetInstance();
    const auto  l_copiedUUID = l_uuidManager.GenerateVALUUID     ();

    // Watcherが未登録Jsonとして削除する前にRegistryへ登録する
    if (!a_assetFilePathRegistry.Add(a_copiedFilePath, l_copiedUUID, l_assetType))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "コピーしたアセットのRegistry登録に失敗しました。\nFilePath : {}", a_copiedFilePath.string());

        return false;
    }

    switch (l_assetType)
    {
        case Enum::AssetFilePathRegistryType::Prefab:
        {
            if (const auto& l_prefabName = a_copiedFilePath.stem().string(); 
                !Converter::GameObjectPrefabJsonConverter::RebindPrefabUUID(a_copiedFilePath,
                                                                            l_prefabName,
                                                                            l_sourceUUID,
                                                                            l_copiedUUID))
            {
               // PrefabUUIDを付け替えられなかったファイルは
               // コピー元と同じPrefabUUIDを持つ不正なPrefabになるため
               // Registryから外し、Watcherに未登録Jsonとして削除させる
               a_assetFilePathRegistry.Erase(a_copiedFilePath);

               return false;
            }
        }
        break;

        case Enum::AssetFilePathRegistryType::Scene:
        {
            // SceneのUUIDはRegistryのみが保持しJson内には存在しないため
            // Json側は名前だけ更新すればよい
            RenameScene(a_copiedFilePath, a_copiedFilePath);
        }
        break;

        default:
        break;
    }
    
    return true;
}

std::filesystem::path FWK::Editor::AssetBrowserEditorWindowAssetCreator::ResolveDefaultFilePath(const std::filesystem::path& a_parentFolderPath, const std::filesystem::path& a_extension, const std::string_view& a_defaultName)
{
    // デフォルト名 + 拡張子を統合した希望パスを作る
    const auto l_desiredFilePath = a_parentFolderPath / (std::string{ a_defaultName } + a_extension.string());

    return Utility::ResolveFilePathConflictByNumberSuffix(l_desiredFilePath);
}

std::string FWK::Editor::AssetBrowserEditorWindowAssetCreator::FetchVALPrefabFileName(const std::weak_ptr<GameObject>& a_gameObject)
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return std::string{ k_defaultPrefabName }; }

    const auto& l_name = l_gameObject->GetREFName();

    if (l_name.empty()) { return std::string{ k_defaultPrefabName }; }

    auto l_fileName = l_name;

    for (auto& l_char : l_fileName)
    {
        // ファイル名に使用できない文字をアンダースコアに変換する
        if (k_invalidFileNameCharacters.find(l_char) == std::string_view::npos) { continue; }
        
        l_char = k_underScoreChar;
    }

    // 末尾のドットと空白はWindowsのファイル名として使えないため取り除く
    while (!l_fileName.empty()             &&
           (l_fileName.back() == k_dotChar ||
            l_fileName.back() == k_spaceChar))
    {
        l_fileName.pop_back();
    }
 
    if (l_fileName.empty()) { return std::string{ k_defaultPrefabName }; }
 
    return l_fileName;
}

FWK::Struct::AssetBrowserEditorWindowAssetCreationResult FWK::Editor::AssetBrowserEditorWindowAssetCreator::CreatePrefabFromGameObject(const std::weak_ptr<GameObject>& a_gameObject, 
                                                                                                                                       const std::filesystem::path&     a_parentFolderPath, 
                                                                                                                                             Scene&                     a_scene,
                                                                                                                                             AssetFilePathRegistry&     a_assetFilePathRegistry) const
{
    const auto& l_gameObject = a_gameObject.lock();

    // 無効・破棄済みのGameObjectはPrefab化しない
    if (!l_gameObject ||
        l_gameObject->GetVALIsDestroyed())
    {
        return {};
    }
 
    // ファイル名はGameObject名ベースで一意化する
    // 空名やファイル名に使えない文字は代替名・置換で回避する
    const auto& l_prefabFilePath = ResolveDefaultFilePath(a_parentFolderPath, Constant::k_lowerJsonExtension, FetchVALPrefabFileName(a_gameObject));
 
          auto& l_uuidManager = Utility::UUIDManager::GetInstance();
    const auto& l_prefabUUID  = l_uuidManager.GenerateVALUUID    ();
 
    // Editor側のAssetFilePathRegistryへ先に登録
    // WatcherはRegistryへ未登録のJsonがディスクに現れると物理削除するため
    // ファイルを書き込む前に必ずRegistryへ登録する
    if (!a_assetFilePathRegistry.Add(l_prefabFilePath, l_prefabUUID, Enum::AssetFilePathRegistryType::Prefab))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetFilePathRegistryへのPrefab登録に失敗したため、Prefabファイルを作成しませんでした。\nFilePath : {}", l_prefabFilePath.string());
 
        return {};
    }

    // 親を持つGameObjectをPrefab化する場合はPrefabのルートとして扱うため親子関係を解除する
    // ClearParent内でTransformがStandaloneへ切り替わりワールド行列は維持される
    // PrefabHierarchyNodeUUIDはGameObjectPrefab::Save内でnilへ戻される
    if (auto& l_hierarchy = l_gameObject->GetMutableREFHierarchy();
        !l_hierarchy.GetREFParent().expired())
    {
        l_hierarchy.ClearParent();
 
        // 階層の深さが変わったため実行レベルを再構築する
        a_scene.RebuildGameObjectExecutionLevelList();
    }

    GameObjectPrefab l_gameObjectPrefab = {};

    // Save内部でConvertToPrefabが呼ばれ
    // GameObjectと子孫へPrefabUUID/IsPrefabOriginが設定されてから
    // GameObjectJsonConverter::Serializeでフル形式のPrefabJsonが作られる
    // シーンのPrefabSystemを渡して別Prefabのネストインスタンスを正しく差分保存する
    auto& l_prefabSystem = a_scene.GetMutableREFGameObjectPrefabSystem();
 
    if (!l_gameObjectPrefab.Save(l_prefabFilePath,
                                 l_prefabUUID,
                                 l_prefabSystem,
                                 *l_gameObject))
    {
        // Registryだけにエントリが残るとWatcherが間違って
        // ファイルを削除しないようになるため、Registryから削除して登録前の状態へ戻す
        a_assetFilePathRegistry.Erase(l_prefabFilePath);
 
        // ConvertToPrefabでPrefabUUIDが設定済みのため
        // 失敗時はPrefabとの紐付けを剥がして元の状態へ戻す
        l_gameObject->DetachFromPrefab();
 
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Prefabファイルの保存に失敗したため、Registry登録を取り消しました。\nFilePath : {}", l_prefabFilePath.string());
 
        return {};
    }

    // シーン側のPrefabSystemへも登録する
    // 未登録のままだとScene保存時にPrefabとの差分が作れずフル形式になってしまう
    l_prefabSystem.AddPrefab(l_prefabUUID, l_gameObjectPrefab);
 
    Struct::AssetBrowserEditorWindowAssetCreationResult l_result = {};
 
    l_result.m_createdFilePath = l_prefabFilePath;
    l_result.m_isSuccess       = true;
 
    return l_result;
}

std::vector<std::shared_ptr<FWK::GameObject>> FWK::Editor::AssetBrowserEditorWindowAssetCreator::CollectPrefabTargetGameObjectList(const std::weak_ptr<GameObject>& a_droppedGameObject) const
{
    const auto& l_dropped = a_droppedGameObject.lock();

    if (!l_dropped ||
        l_dropped->GetVALIsDestroyed())
    {
        return {};
    }

    const auto& l_editorManager            = EditorManager::GetInstance                             ();
    const auto& l_gameObjectSelectionState = l_editorManager.GetREFGameObjectSelectionState         ();
    const auto& l_selectedList             = l_gameObjectSelectionState.GetREFSelectedGameObjectList();

    // ドロップしたGameObjectが選択リストに含まれていなければ
    // ドロップしたGameObjectのみをPrefab化対象にする
    if (!std::ranges::any_of(l_selectedList,
                             [&l_dropped](const auto& a_selectedWeak)
                             {
                                 return a_selectedWeak.lock() == l_dropped;
                             }))
    {
        return { l_dropped }; 
    }

    std::vector<std::shared_ptr<GameObject>> l_targetList = {};

    // 選択されたリスト数分要素数を予約
    l_targetList.reserve(l_selectedList.size());

    for (const auto& l_selectedWeak : l_selectedList)
    {
        const auto& l_selected = l_selectedWeak.lock();
 
        if (!l_selected ||
            l_selected->GetVALIsDestroyed())
        {
            continue;
        }
 
        // 選択リスト内に自身の祖先がいる場合は除外する
        // 祖先側のPrefabへ内部ノードとして既に含まれるため二重にPrefab化しない
        if (Utility::HasAncestorInList(l_selectedList, l_selected)) { continue; }
 
        l_targetList.emplace_back(l_selected);
    }
 
    return l_targetList;
}