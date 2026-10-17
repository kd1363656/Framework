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
    // 空のGameObjectを1つ持つPrefabファイルを作る
    // 一意なファイルパスの決定・Registryへの先登録・失敗時の取り消しはCreateRegisteredAssetFileが行う
    return CreateRegisteredAssetFile(a_parentFolderPath,
                                     k_defaultPrefabName,
                                     &AssetBrowserEditorWindowAssetCreator::WriteNewPrefabFile,
                                     Enum::AssetFilePathRegistryType::Prefab,
                                     a_assetFilePathRegistry);
}
FWK::Struct::AssetBrowserEditorWindowAssetCreationResult FWK::Editor::AssetBrowserEditorWindowAssetCreator::CreateScene(const std::filesystem::path& a_parentFolderPath, AssetFilePathRegistry& a_assetFilePathRegistry) const
{
    // GameObjectを持たない空のSceneファイルを作る
    return CreateRegisteredAssetFile(a_parentFolderPath,
                                     k_defaultSceneName,
                                     &AssetBrowserEditorWindowAssetCreator::WriteNewSceneFile,
                                     Enum::AssetFilePathRegistryType::Scene,
                                     a_assetFilePathRegistry);
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

    const auto& l_deserializedJson = Utility::LoadJsonFile(a_oldFilePath);

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

FWK::Struct::AssetBrowserEditorWindowAssetCreationResult FWK::Editor::AssetBrowserEditorWindowAssetCreator::CreateRegisteredAssetFile(const std::filesystem::path&          a_parentFolderPath,
                                                                                                                                      const std::string_view&               a_defaultName,
                                                                                                                                      const AssetFileWriteFunction          a_writeFunction,
                                                                                                                                      const Enum::AssetFilePathRegistryType a_registryType,
                                                                                                                                            AssetFilePathRegistry&          a_assetFilePathRegistry) const
{
    if (!a_writeFunction) { return {}; }

    // 一意なファイルパスを決定
    // 同名が存在する場合はNewPrefab1,NewPrefab2...と番号付与される
    const auto& l_filePath = ResolveDefaultFilePath(a_parentFolderPath, Constant::k_lowerJsonExtension, a_defaultName);

    // アセットのUUIDを生成
          auto& l_uuidManager = Utility::UUIDManager::GetInstance();
    const auto& l_uuid        = l_uuidManager.GenerateVALUUID    ();

    // AssetFilePathRegistryへ先に登録
    // WatcherはRegistryへ未登録のJsonがディスクに現れると物理削除するため
    // ファイルを書き込む前に必ずRegistryへ登録する
    if (!a_assetFilePathRegistry.Add(l_filePath, l_uuid, a_registryType))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetFilePathRegistryへの登録に失敗したため、ファイルを作成しませんでした。\nFilePath : {}", l_filePath.string());

        return {};
    }

    // 種類ごとの書き込みはメンバ関数ポインタで受け取り、ここから呼ぶ
    // std::invoke(関数ポインタ, オブジェクト, 引数...)で「このオブジェクトのメンバ関数として呼ぶ」
    if (!std::invoke(a_writeFunction,
                     *this,
                     l_filePath,
                     l_uuid))
    {
        // Registryだけにエントリが残るとWatcherが間違って
        // ファイルを削除しないようになるため、Registryから削除して登録前の状態へ戻す
        a_assetFilePathRegistry.Erase(l_filePath);

        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "ファイルの書き込みに失敗したため、Registry登録を取り消しました。\nFilePath : {}", l_filePath.string());

        return {};
    }

    Struct::AssetBrowserEditorWindowAssetCreationResult l_result = {};

    l_result.m_createdFilePath = l_filePath;
    l_result.m_isSuccess       = true;

    return l_result;
}

bool FWK::Editor::AssetBrowserEditorWindowAssetCreator::WriteNewPrefabFile(const std::filesystem::path& a_filePath, const boost::uuids::uuid& a_prefabUUID) const
{
    // 空のGameObjectを保持したPrefabファイルを作成する
    // GameObjectはINIT内でweak_from_thisを使うため
    // shared_ptr管理でなければbad_weak_ptrが投げられる
    const auto& l_gameObject = std::make_shared<GameObject>();

    l_gameObject->INIT   ();
    l_gameObject->SetName(a_filePath.stem().string());

    // 新規の空GameObjectにはPrefabインスタンスの子が存在しないため
    // シリアライズ用の一時的なPrefabSystemで十分
    SceneGameObjectPrefabSystem l_prefabSystem = {};

    l_prefabSystem.INIT();

    // Save内部でConvertToPrefabが呼ばれ
    // GameObjectと子孫へPrefabUUID/IsPrefabOriginが設定されてから
    // GameObjectJsonConverter::Serializeでフル形式のPrefabJsonが作られる
    GameObjectPrefab l_gameObjectPrefab = {};

    return l_gameObjectPrefab.Save(a_filePath,
                                   a_prefabUUID,
                                   l_prefabSystem,
                                   *l_gameObject);
}
bool FWK::Editor::AssetBrowserEditorWindowAssetCreator::WriteNewSceneFile(const std::filesystem::path& a_filePath, const boost::uuids::uuid&) const
{
    // SceneのUUIDはRegistryだけが持ち、Sceneファイルには書かないため引数は使わない
    // (Prefabと同じ形の関数にすることで、CreateRegisteredAssetFileから同じ呼び方ができる)
    Scene l_scene = {};

    l_scene.INIT   ();
    l_scene.SetName(a_filePath.stem().string());

    // 新規からシーンとして必要な情報のみを書き出す
    const auto& l_rootJson = l_scene.Serialize();

    if (l_rootJson.is_null()) { return false; }

    // SaveJsonFileは拡張子が.jsonかを検査し
    // 親フォルダが存在しない場合はcreate_directoriesで作成してから書き込む
    return Utility::SaveJsonFile(l_rootJson, a_filePath);
}

std::filesystem::path FWK::Editor::AssetBrowserEditorWindowAssetCreator::ResolveDefaultFilePath(const std::filesystem::path& a_parentFolderPath, const std::filesystem::path& a_extension, const std::string_view& a_defaultName)
{
    // デフォルト名 + 拡張子を統合した希望パスを作る
    const auto& l_desiredFilePath = a_parentFolderPath / (std::string{ a_defaultName } + a_extension.string());

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

    // 親から切り離して結び直すときにローカルのTransformを控えて戻すため、Transformが無ければPrefab化しない
    const auto& l_transform = l_gameObject->GetVALTransformComponent().lock();

    if (!l_transform) { return {}; }
 
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

    // 親を持つGameObjectは、親子関係を一度切り離した状態でPrefabに保存し、保存が終わったら元の親へ結び直す
    // 切り離すとワールドの位置を保つようにローカルのTransformが計算し直されるため
    // Prefabのルートにはシーンで見えている位置(ワールドの値)が保存される
    // 例 : 親(10, 0, 0)の子(1, 0, 0)をPrefab化
    //      → 切り離すと子のローカルは(11, 0, 0)になり、Prefabのルートの位置は(11, 0, 0)
    //      → 保存後に親(10, 0, 0)へ結び直し、ローカルを元の(1, 0, 0)へ戻す
    // 結び直すために、親と兄弟の並び・ローカルのTransform・親の子としてのUUID・Prefab由来かどうかを切り離す前に控える
    // ローカルのTransform・UUID・Prefab由来かどうかはこの後書き換えられるため、参照ではなくコピーで受け取る
          auto& l_hierarchy               = l_gameObject->GetMutableREFHierarchy       ();
    const bool  l_hasParent               = !l_hierarchy.GetREFParent                  ().expired();
    const auto& l_placement               = Utility::FetchVALGameObjectPlacement       (l_gameObject, a_scene);
    const auto  l_localTransform          = l_transform->GetREFTransform               ();
    const auto  l_prefabHierarchyNodeUUID = l_gameObject->GetREFPrefabHierarchyNodeUUID();
    const bool  l_wasPrefabOrigin         = l_gameObject->GetVALIsPrefabOrigin         ();

    if (l_hasParent)
    {
        l_hierarchy.ClearParent();
    }

    GameObjectPrefab l_gameObjectPrefab = {};

    // Save内部でConvertToPrefabが呼ばれ
    // GameObjectと子孫へPrefabUUID/IsPrefabOriginが設定されてから
    // GameObjectJsonConverter::Serializeでフル形式のPrefabJsonが作られる
    // シーンのPrefabSystemを渡して別Prefabのネストインスタンスを正しく差分保存する
    auto& l_prefabSystem = a_scene.GetMutableREFGameObjectPrefabSystem();

    const bool l_isSaved = l_gameObjectPrefab.Save(l_prefabFilePath,
                                                   l_prefabUUID,
                                                   l_prefabSystem,
                                                   *l_gameObject);

    // 保存が終わったので元の親へ結び直す
    // 親を持たないGameObjectはPrefabのルートと同じnilのままでよい
    if (l_hasParent)
    {
        // 親のPrefabの一部(Prefab由来)だったGameObjectは、親から見ると
        // 「Prefabの元の子を消して、新しいPrefabのインスタンスを足した」ことになる
        // UUIDはnilのまま結び直し、AddChildで新しいUUIDを作らせる
        // 元の子のUUIDは切り離したとき(RemoveChild)に親の「Prefabから消した子」へ記録されている
        // 例 : 親PのインスタンスにあるXのインスタンスW(UUID = A)をYにする
        //      → 親は「Aを消して、Yのインスタンス(UUID = B)を足した」として保存される
        //      元のAへ戻すと、シーン保存時にPの子A(Xのインスタンス)との差分として保存され、読み込むとWの子が消える
        // 親のPrefabと関係の無い子と保存に失敗したときだけ、元のUUIDへ戻して親側の登録と食い違わないようにする
        if (!l_wasPrefabOrigin ||
            !l_isSaved)
        {
            l_gameObject->SetPrefabHierarchyNodeUUID(l_prefabHierarchyNodeUUID);
        }

        // 親と兄弟の並びを元へ戻す
        // 切り離したときに親へ付いた「Prefabから消した子」の記録もここで消える
        if (Utility::ApplyGameObjectPlacement(l_gameObject, l_placement, a_scene))
        {
            // 結び直すとワールドの位置を保つようにローカルが計算し直され、小数の誤差が出ることがあるため
            // 控えておいたローカルの値をそのまま戻す
            l_transform->ApplyTransformScale   (l_localTransform.m_scale);
            l_transform->ApplyTransformRotation(l_localTransform.m_rotation);
            l_transform->ApplyTransformPosition(l_localTransform.m_position);
        }
        else
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Prefab化した後、元の親へ結び直せませんでした。\nFilePath : {}", l_prefabFilePath.string());
        }
    }

    if (!l_isSaved)
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
    l_prefabSystem.AddPrefab(l_gameObjectPrefab, l_prefabUUID);

    // GameObjectがPrefabと紐付いたのでシーンの内容も変わった
    // この操作はUndoできないため、保存するまで未保存の状態にする
    auto& l_editorManager  = EditorManager::GetInstance                 ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.MarkSceneModified();

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