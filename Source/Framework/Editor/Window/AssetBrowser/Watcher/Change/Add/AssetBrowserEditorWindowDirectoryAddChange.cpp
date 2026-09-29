#include "AssetBrowserEditorWindowDirectoryAddChange.h"

void FWK::Editor::AssetBrowserEditorWindowDirectoryAddChange::Apply(AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, SceneManager& a_sceneManager)
{
    // 前FrameでFile削除などに失敗してRetryになっていた場合でも、
    // 今Frameで処理できたときにChangeを完了できるよう最初に解除する
    SetIsRequiresRetry(false);

    // Folder追加はAssetRegistryへ登録するAssetそのものではない
    if (GetVALIsDirectory()) { return; }

    const auto& l_filePath = GetREFFilePath();


    // 現在AssetFilePathRegistryで管理しているPrefab/SceneはJsonなので
    // Json以外のFile追加はこのChangeでは同期対象にしない
    if (l_filePath.empty() ||
        l_filePath.extension() != Constant::k_lowerJsonExtension)
    {
        return;
    }

    if (const auto* l_assetUUID = a_assetBrowserAssetFilePathRegistry.FindPTRAssetUUID(l_filePath);
        l_assetUUID)
    {
        // FilePath          -> UUID
        // UUID              -> AssetFilePathData
        // AssetFilePathData -> 同じFilePath
        // の3つが成立して初めて正式登録済みAssetとして扱う
        if (const auto* l_assetFilePathData = a_assetBrowserAssetFilePathRegistry.FindPTRAssetFilePathData(*l_assetUUID);
           l_assetFilePathData &&
           l_assetFilePathData->m_assetFilePath == l_filePath)
        {
            switch (l_assetFilePathData->m_type)
            {
                case Enum::AssetFilePathRegistryType::Prefab:
                {
                    ApplyPrefabAdd(l_filePath, a_sceneManager, *l_assetUUID);

                    return;
                }
                break;

                case Enum::AssetFilePathRegistryType::Scene:
                {
                    ApplySceneAdd(l_filePath, *l_assetUUID, a_sceneManager);

                    return;
                }
                break;

                default:
                {
                    FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "追加されたJsonに対応するAssetFilePathRegistryTypeが無効です。\nFilePath : {}", l_filePath.string());
                }
                break;
            }
        }
        else if (!l_assetFilePathData)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "追加されたJsonのUUIDに対応するAssetFilePathDataがAssetBrowserのAssetFilePathRegistryに存在しません。\nFilePath : {}", l_filePath.string());
        }
        else
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
                        "追加されたJsonのFilePathとAssetFilePathRegistryに登録されているFilePathが一致しません。\nAddedFilePath : {}\nRegistryFilePath : {}",
                        l_filePath.string(),
                        l_assetFilePathData->m_assetFilePath.string());
        }

        // Path -> UUIDだけ存在するなど、
        // Registry内部の対応関係が壊れているDataを残さない
        a_assetBrowserAssetFilePathRegistry.Erase(l_filePath);
    }

    // AssetBrowser側でFilePath -> UUID -> AssetFilePathDataの正式な対応関係を確認できないJsonは
    // Editor管理外から追加されたFileとして物理削除する
    std::error_code l_errorCode = {};

    if (std::filesystem::remove(l_filePath, l_errorCode))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetBrowserのAssetFilePathRegistryへ正式登録されていないJsonが追加されたため削除しました。\nFilePath : {}", l_filePath.string());

        return;
    }

    // remove()がfalseでもErrorがなければ、
    // 他の処理などによって既にFileが消えているためRetry不要
    if (!l_errorCode) { return; }

    // Explorerなど別ProcessがFileをまだ使用している場合、
    // ADD通知を受けたFrameでは削除できない可能性がある
    // その場合はDirectoryNotificationProcessorによって
    // 次FrameでもこのChangeを再実行してもらう
    FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
                "AssetBrowserのAssetFilePathRegistryへ正式登録されていないJsonを削除できなかったため、次Frameで再試行します。\nFilePath : {}\nErrorCode : {}",
                l_filePath.string(),
                l_errorCode.value());

    SetIsRequiresRetry(true);
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryAddChange::ApplyPrefabAdd(const std::filesystem::path& a_filePath, const SceneManager& a_sceneManager, const boost::uuids::uuid& a_prefabUUID)
{
    if (a_filePath.empty() ||
        a_prefabUUID.is_nil())
    {
        return;
    }

    const auto& l_scene = a_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    auto& l_sceneAssetFilePathRegistry = l_scene->GetMutableREFAssetFilePathRegistry();
    bool  l_isSceneRegistryAdded       = false;

    // Scene側に同じFilePathが既に存在する場合は、
    // AssetBrowser側とUUID、Type、FilePathが同じか確認する
    if (const auto* l_sceneAssetUUID = l_sceneAssetFilePathRegistry.FindPTRAssetUUID(a_filePath))
    {
        const auto* l_assetFilePathData = l_sceneAssetFilePathRegistry.FindPTRAssetFilePathData(*l_sceneAssetUUID);

        if (*l_sceneAssetUUID != a_prefabUUID                               ||
            !l_assetFilePathData                                                   ||
            l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Prefab ||
            l_assetFilePathData->m_assetFilePath != a_filePath)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneのAssetFilePathRegistryに登録されているPrefab情報がAssetBrowser側と一致しません。\nFilePath : {}", a_filePath.string());

            return;
        }
    }
    else
    {
        // 同じUUIDが別FilePathで既に登録されている場合、
        // AddChange側でどちらのPathが正しいかを推測してはいけない
        if (l_sceneAssetFilePathRegistry.FindPTRAssetFilePathData(a_prefabUUID))
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "PrefabUUIDがSceneのAssetFilePathRegistryへ別FilePathで既に登録されています。\nFilePath : {}", a_filePath.string());

            return;
        }

        // 現在Scene側でもUUIDからPrefabFilePathを取得できるよう登録する
        if (!l_sceneAssetFilePathRegistry.Add(a_filePath,
                                                     a_prefabUUID,
                                                     Enum::AssetFilePathRegistryType::Prefab))
        {
            return;
        }

        l_isSceneRegistryAdded = true;
    }

    if (!l_scene)
    {
        // 今回AddChangeで追加したRegistry情報だけRollbackする
        if (l_isSceneRegistryAdded)
        {
            l_sceneAssetFilePathRegistry.Erase(a_filePath);
        }

        SetIsRequiresRetry(true);

        return;
    }

    auto& l_gameObjectPrefabSystem = l_scene->GetMutableREFGameObjectPrefabSystem();

    // すでにPrefabSystemへ同じPrefabUUIDが存在する場合は、
    // 同じPrefabを二重登録しない
    if (l_gameObjectPrefabSystem.FindPTRPrefab(a_prefabUUID)) { return; }

    GameObjectPrefab l_gameObjectPrefab = {};

    // Add通知はJson生成直後に届く可能性がある
    // Prefab::Load()内部でJsonを実際に読み込ませ、
    // 書き込み途中の不完全なPrefabをPrefabSystemへ登録しない
    l_gameObjectPrefab.Load(a_filePath);

    if (l_gameObjectPrefab.GetREFJson().is_null())
    {
        // SceneRegistryを今回追加した場合だけ元に戻す
        // 元から存在していたRegistry情報は削除しない
        if (l_isSceneRegistryAdded)
        {
            l_sceneAssetFilePathRegistry.Erase(a_filePath);
        }

        SetIsRequiresRetry(true);

        return;
    }

    l_gameObjectPrefabSystem.AddPrefab(a_prefabUUID, l_gameObjectPrefab);
}
void FWK::Editor::AssetBrowserEditorWindowDirectoryAddChange::ApplySceneAdd(const std::filesystem::path& a_filePath, const boost::uuids::uuid& a_sceneUUID, SceneManager& a_sceneManager)
{
    if (a_filePath.empty() ||
        a_sceneUUID.is_nil())
    {
        return;
    }
 
    // CurrentSceneの新規保存によるADD通知ならここでは何もしない
    if (a_sceneManager.GetREFCurrentSceneFilePath() == a_filePath) { return; }
 
    const auto& l_scene = a_sceneManager.GetVALScene().lock();
 
    if (!l_scene) { return; }
 
    const auto& l_sceneAssetFilePathRegistry = l_scene->GetREFAssetFilePathRegistry ();
          auto& l_sceneChanger               = l_scene->GetMutableREFSceneChanger   ();
    const auto& l_nextSceneDataMap           = l_sceneChanger.GetREFNextSceneDataMap();
 
    // Scene側Registryへすでに同じFilePathが存在する場合
    if (const auto* l_sceneSceneUUID = l_sceneAssetFilePathRegistry.FindPTRAssetUUID(a_filePath);
        l_sceneSceneUUID)
    {
        // AssetBrowser側とScene側で
        // UUID / Type / FilePathが全て一致している必要がある
        if (const auto* l_assetFilePathData = l_sceneAssetFilePathRegistry.FindPTRAssetFilePathData(*l_sceneSceneUUID);
            *l_sceneSceneUUID != a_sceneUUID                                      ||
            !l_assetFilePathData                                                  ||
            l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Scene ||
            l_assetFilePathData->m_assetFilePath != a_filePath)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneのAssetFilePathRegistryに登録されているScene情報がAssetBrowser側と一致しません。\nFilePath : {}", a_filePath.string());
 
            return;
        }
 
        // RegistryとNextSceneDataMapの両方へ既に登録されている場合は何もしない
        if (const auto& l_nextSceneDataITR = l_nextSceneDataMap.find(a_sceneUUID);
            l_nextSceneDataITR != l_nextSceneDataMap.end())
        {
            if (l_nextSceneDataITR->second.m_filePath != a_filePath)
            {
                FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneのAssetFilePathRegistryとSceneChangerのNextSceneDataMapでSceneFilePathが一致しません。\nFilePath : {}", a_filePath.string());
            }
 
            return;
        }
 
        // Registryだけ存在してNextSceneDataMapに存在しない
        // Registryから正式なPathを引く版で不足しているMap登録を完成させる
        if (!l_sceneChanger.AddNextSceneData(a_sceneUUID, l_sceneAssetFilePathRegistry) &&
            !Utility::CanLoadFilePath(a_filePath, Constant::k_lowerJsonExtension))
        {
            SetIsRequiresRetry(true);
        }
 
        return;
    }
 
 
    // UUIDが別FilePathとして既に登録されている場合は、
    // AddChange側で勝手にFilePathを書き換えない
    if (l_sceneAssetFilePathRegistry.FindPTRAssetFilePathData(a_sceneUUID) ||
        l_nextSceneDataMap.contains(a_sceneUUID))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneUUIDがScene側へ別の状態で既に登録されているため、追加されたSceneを同期できません。\nFilePath : {}", a_filePath.string());
 
        return;
    }
 
    auto& l_mutableSceneAssetFilePathRegistry = l_scene->GetMutableREFAssetFilePathRegistry();
    auto& l_mutableSceneChanger               = l_scene->GetMutableREFSceneChanger         ();
 
    // まずScene側RegistryへSceneとして登録する
    if (!l_mutableSceneAssetFilePathRegistry.Add(a_filePath, a_sceneUUID, Enum::AssetFilePathRegistryType::Scene))
    {
        if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerJsonExtension))
        {
            SetIsRequiresRetry(true);
        }
 
        return;
    }
 
    // Registry登録に成功した後、
    // Registryから正式なPathを引く版でNextSceneDataMapへ登録する
    if (!l_mutableSceneChanger.AddNextSceneData(a_sceneUUID, l_sceneAssetFilePathRegistry))
    {
        // NextSceneDataMapへの登録に失敗した場合
        // RegistryだけにSceneが残る中途半端な状態を防ぐ
        l_mutableSceneAssetFilePathRegistry.Erase(a_filePath);
 
        if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerJsonExtension))
        {
            SetIsRequiresRetry(true);
        }
    }
}