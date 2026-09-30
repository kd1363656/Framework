#include "AssetBrowserEditorWindowDirectoryAddChange.h"

void FWK::Editor::AssetBrowserEditorWindowDirectoryAddChange::Apply(AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager)
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

    if (const auto* l_assetUUID = a_assetFilePathRegistry.FindPTRAssetUUID(l_filePath);
        l_assetUUID)
    {
        // FilePath          -> UUID
        // UUID              -> AssetFilePathData
        // AssetFilePathData -> 同じFilePath
        // の3つが成立して初めて正式登録済みAssetとして扱う
        if (const auto* l_assetFilePathData = a_assetFilePathRegistry.FindPTRAssetFilePathData(*l_assetUUID);
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
                    ApplySceneAdd(l_filePath, 
                                  *l_assetUUID,
                                  a_assetFilePathRegistry,
                                  a_sceneManager);

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
        a_assetFilePathRegistry.Erase(l_filePath);
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
        SetIsRequiresRetry(true);
 
        return;
    }
 
    l_gameObjectPrefabSystem.AddPrefab(a_prefabUUID, l_gameObjectPrefab);
}
void FWK::Editor::AssetBrowserEditorWindowDirectoryAddChange::ApplySceneAdd(const std::filesystem::path& a_filePath, const boost::uuids::uuid& a_sceneUUID, const AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager)
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
 
    auto& l_sceneChanger = l_scene->GetMutableREFSceneChanger();
 
    // 既にNextSceneMapへ登録済みなら何もしない
    if (l_sceneChanger.FetchPTRNexScene(a_sceneUUID)) { return; }

    NextScene l_nextScene = {};

    // Add通知はJson生成直後に届く可能性がある
    // NextScene::Load()内部でJsonを実際に読み込ませ、
    // 書き込み途中の不完全なSceneをSceneChangerへ登録しない
    l_nextScene.Load(a_filePath);

    if (l_nextScene.GetREFJson().is_null())
    {
        SetIsRequiresRetry(true);

        return;
    }

    l_sceneChanger.AddNextScene(a_sceneUUID, l_nextScene);
}