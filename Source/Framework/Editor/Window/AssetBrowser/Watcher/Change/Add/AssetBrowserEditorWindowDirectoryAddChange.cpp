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
                    ApplyPrefabAdd(l_filePath, *l_assetUUID, a_sceneManager);

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

void FWK::Editor::AssetBrowserEditorWindowDirectoryAddChange::ApplyPrefabAdd(const std::filesystem::path& a_filePath, const boost::uuids::uuid& a_prefabUUID, SceneManager& a_sceneManager)
{
    if (a_filePath.empty() ||
        a_prefabUUID.is_nil())
    {
        return;
    }

    auto& l_sceneManagerAssetFilePathRegistry = a_sceneManager.GetMutableREFAssetFilePathRegistry();
    bool  l_isSceneManagerRegistryAdded       = false;

    // SceneManager側に同じFilePathが既に存在する場合は、
    // AssetBrowser側とUUID、Type、FilePathが同じか確認する
    if (const auto* l_sceneManagerAssetUUID = l_sceneManagerAssetFilePathRegistry.FindPTRAssetUUID(a_filePath))
    {
        const auto* l_assetFilePathData = l_sceneManagerAssetFilePathRegistry.FindPTRAssetFilePathData(*l_sceneManagerAssetUUID);

        if (*l_sceneManagerAssetUUID != a_prefabUUID                               ||
            !l_assetFilePathData                                                   ||
            l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Prefab ||
            l_assetFilePathData->m_assetFilePath != a_filePath)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneManagerのAssetFilePathRegistryに登録されているPrefab情報がAssetBrowser側と一致しません。\nFilePath : {}", a_filePath.string());

            return;
        }
    }
    else
    {
        // 同じUUIDが別FilePathで既に登録されている場合、
        // AddChange側でどちらのPathが正しいかを推測してはいけない
        if (l_sceneManagerAssetFilePathRegistry.FindPTRAssetFilePathData(a_prefabUUID))
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "PrefabUUIDがSceneManagerのAssetFilePathRegistryへ別FilePathで既に登録されています。\nFilePath : {}", a_filePath.string());

            return;
        }

        // 現在Scene側でもUUIDからPrefabFilePathを取得できるよう登録する
        if (!l_sceneManagerAssetFilePathRegistry.Add(a_filePath,
                                                     a_prefabUUID,
                                                     Enum::AssetFilePathRegistryType::Prefab))
        {
            return;
        }

        l_isSceneManagerRegistryAdded = true;
    }

    const auto& l_scene = a_sceneManager.GetVALScene().lock();

    if (!l_scene)
    {
        // 今回AddChangeで追加したRegistry情報だけRollbackする
        if (l_isSceneManagerRegistryAdded)
        {
            l_sceneManagerAssetFilePathRegistry.Erase(a_filePath);
        }

        SetIsRequiresRetry(true);

        return;
    }

    auto& l_prefabSystem = l_scene->GetMutableREFPrefabSystem();

    // すでにPrefabSystemへ同じPrefabUUIDが存在する場合は、
    // 同じPrefabを二重登録しない
    if (l_prefabSystem.FindPTRPrefab(a_prefabUUID)) { return; }

    // シーンのプレハブシステムにないということは追加するということなので
    // アロケータ用基本サイズを設定してprefabSystemに設定する
    Struct::PrefabData l_prefabData = {};

    auto& l_prefabInstanceNUMAllocator       = l_prefabData.m_prefabInstanceNUMAllocator;
    auto& l_prefabInstanceNUMIsAllocatedList = l_prefabInstanceNUMAllocator.GetMutableREFIsAllocatedList();

    // あらかじめオーバーフローしないように要素数を確保しておく
    l_prefabInstanceNUMIsAllocatedList.resize(Constant::k_storageIDAllocatorDefaultCreateStorageIDCapacity, false);

    // Add通知はJson生成直後に届く可能性がある
    // Prefab::Load()内部でJsonを実際に読み込ませ、
    // 書き込み途中の不完全なPrefabをPrefabSystemへ登録しない
    l_prefabData.m_prefab.Load(a_filePath);

    if (l_prefabData.m_prefab.GetREFJson().is_null())
    {
        // SceneManagerRegistryを今回追加した場合だけ元に戻す
        // 元から存在していたRegistry情報は削除しない
        if (l_isSceneManagerRegistryAdded)
        {
            l_sceneManagerAssetFilePathRegistry.Erase(a_filePath);
        }

        SetIsRequiresRetry(true);

        return;
    }

    l_prefabSystem.AddPrefab(a_prefabUUID, l_prefabData);

    // Prefab作成元のGameObjectなどが既にSceneに存在する場合
    // PrefabSystemへPrefabを追加した後に代表GameObject候補として再確認する
    for (const auto& l_gameObject : l_scene->GetREFGameObjectList())
    {
        if (!l_gameObject                     ||
            l_gameObject->GetVALIsDestroyed() ||
            l_gameObject->GetREFPrefabUUID() != a_prefabUUID)
        {
            continue;
        }

        l_prefabSystem.CachePrefabGameObjectIfNeeded(l_gameObject);
    }

}
void FWK::Editor::AssetBrowserEditorWindowDirectoryAddChange::ApplySceneAdd(const std::filesystem::path& a_filePath, const boost::uuids::uuid& a_sceneUUID, SceneManager& a_sceneManager)
{
    if (a_filePath.empty() ||
        a_sceneUUID.is_nil())
    {
        return;
    }

    // 現在の設計ではCurrentScene自身を
    // SceneManager側AssetFilePathRegistryへ登録していないため、
    // CurrentSceneの新規保存によるADD通知ならここでは何もしない
    if (a_sceneManager.GetREFCurrentSceneFilePath() == a_filePath) { return; }

          auto& l_sceneManagerAssetFilePathRegistry = a_sceneManager.GetMutableREFAssetFilePathRegistry();
    const auto& l_nextSceneLoadFilePathMap          = a_sceneManager.GetREFNextSceneLoadFilePathMap    ();

    // SceneManager側Registryへすでに同じFilePathが存在する場合
    if (const auto* l_sceneManagerSceneUUID = l_sceneManagerAssetFilePathRegistry.FindPTRAssetUUID(a_filePath);
        l_sceneManagerSceneUUID)
    {
        // AssetBrowser側とSceneManagerで
        // UUID / Type / FilePathが全て一致している必要がある
        if (const auto* l_assetFilePathData = l_sceneManagerAssetFilePathRegistry.FindPTRAssetFilePathData(*l_sceneManagerSceneUUID);
            *l_sceneManagerSceneUUID != a_sceneUUID                               ||
            !l_assetFilePathData                                                  ||
            l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Scene ||
            l_assetFilePathData->m_assetFilePath != a_filePath)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneManagerのAssetFilePathRegistryに登録されているScene情報がAssetBrowser側と一致しません。\nFilePath : {}", a_filePath.string());

            return;
        }

        // RegistryとNextSceneMapの両方へ既に登録されている場合は何もしない
        if (const auto& l_nextSceneLoadFilePathITR = l_nextSceneLoadFilePathMap.find(a_sceneUUID);
            l_nextSceneLoadFilePathITR != l_nextSceneLoadFilePathMap.end())
        {
            if (l_nextSceneLoadFilePathITR->second != a_filePath)
            {
                FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneManagerのAssetFilePathRegistryとNextSceneLoadFilePathMapでSceneFilePathが一致しません。\nFilePath : {}", a_filePath.string());
            }

            return;
        }

        // Registryだけ存在してNextSceneLoadFilePathMapに存在しない
        // UUID版APIで不足しているMAP登録を完成させる
        if (!a_sceneManager.AddNextSceneLoadFilePath(a_sceneUUID) &&
            !Utility::CanLoadFilePath(a_filePath, Constant::k_lowerJsonExtension))
        {
            SetIsRequiresRetry(true);
        }

        return;
    }


    // UUIDが別FilePathとして既に登録されている場合は、
    // AddChange側で勝手にFilePathを書き換えない
    if (l_sceneManagerAssetFilePathRegistry.FindPTRAssetFilePathData(a_sceneUUID) ||
        l_nextSceneLoadFilePathMap.contains(a_sceneUUID))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneUUIDがSceneManager側へ別の状態で既に登録されているため、追加されたSceneを同期できません。\nFilePath : {}", a_filePath.string());

        return;
    }

    if (!a_sceneManager.AddNextSceneLoadFilePath(a_filePath, a_sceneUUID) &&
        !Utility::CanLoadFilePath(a_filePath, Constant::k_lowerJsonExtension))
    {
        SetIsRequiresRetry(true);
    }
}