#include "AssetBrowserEditorWindowDirectoryDeleteChange.h"

void FWK::Editor::AssetBrowserEditorWindowDirectoryDeleteChange::Apply(AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager)
{
    const auto& l_deletedFilePath = GetREFFilePath();

    if (l_deletedFilePath.empty()) { return; }

    if (GetVALIsDirectory())
    {
        ApplyDirectoryDelete(l_deletedFilePath, a_assetFilePathRegistry, a_sceneManager);

        return;
    }

    ApplyFileDelete(l_deletedFilePath, a_assetFilePathRegistry, a_sceneManager);
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryDeleteChange::ApplyFileDelete(const std::filesystem::path& a_deleteFilePath, AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager) const
{
    // AssetBrowser側RegistryからAsset情報を取得
    if (const auto* l_assetUUID = a_assetFilePathRegistry.FindPTRAssetUUID(a_deleteFilePath);
        l_assetUUID)
    {
        // この後RegistryからEraseされる可能性があるため
        // Map内部を示すPointerのまま保持せずUUIDを値としてコピーする
        const auto  l_copiedAssetUUID   = *l_assetUUID;
        const auto* l_assetFilePathData = a_assetFilePathRegistry.FindPTRAssetFilePathData(l_copiedAssetUUID);

        if (!l_assetFilePathData)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetBrowserのAssetFilePathRegistry内部で、UUIDに対応するAssetFilePathDataを取得できませんでした\nFilePath : {}。", a_deleteFilePath.string());

            return;
        }

        switch (l_assetFilePathData->m_type)
        {
            case Enum::AssetFilePathRegistryType::Prefab:
            {
                ApplyPrefabDelete(a_deleteFilePath,
                                  l_copiedAssetUUID,
                                  a_sceneManager,
                                  a_assetFilePathRegistry);

                return;
            }
            break;

            case Enum::AssetFilePathRegistryType::Scene:
            {
                ApplySceneDelete(a_deleteFilePath,
                                 l_copiedAssetUUID,
                                 a_assetFilePathRegistry,
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
                                                                                   const SceneManager&          a_sceneManager,
                                                                                         AssetFilePathRegistry& a_assetFilePathRegistry) const
{
    if (a_prefabUUID.is_nil()) { return; }

    if (const auto& l_scene = a_sceneManager.GetVALScene().lock();
        l_scene)
    {
        // GameObjectPrefabSystemはPrefabUUIDをKeyとして管理している
        // GameObjectPrefabJsonそのものが削除されたので、
        // 現在Sceneで保持しているPrefab情報もUUIDで削除する
        auto& l_gameObjectPrefabSystem = l_scene->GetMutableREFGameObjectPrefabSystem();

        l_gameObjectPrefabSystem.RemovePrefab(a_prefabUUID);
    }

    // Registryから削除
    a_assetFilePathRegistry.Erase(a_deleteFilePath);
}
void FWK::Editor::AssetBrowserEditorWindowDirectoryDeleteChange::ApplySceneDelete(const std::filesystem::path& a_deleteFilePath,
                                                                                  const boost::uuids::uuid&    a_sceneUUID,
                                                                                        AssetFilePathRegistry& a_assetFilePathRegistry,
                                                                                        SceneManager&          a_sceneManager) const
{
    if (a_sceneUUID.is_nil()) { return; }

    if (const auto& l_scene = a_sceneManager.GetVALScene().lock();
        l_scene)
    {
        auto& l_sceneChanger = l_scene->GetMutableREFSceneChanger();

        // NextSceneとして登録済みのときだけSceneChanger側からも削除する
        if (l_sceneChanger.FetchPTRNexSceneData(a_sceneUUID))
        {
            l_sceneChanger.RemoveNextSceneData(a_sceneUUID);
        }
    }

    // CurrentSceneはNextSceneDataMapに存在しないため
    // m_currentSceneFilePathの一致によって判定する
    if (a_sceneManager.GetREFCurrentSceneFilePath() == a_deleteFilePath)
    {
        // 元のSceneJsonが物理的に存在しなくなったため
        // SaveScene()等が古いPathへ再保存しないように無効化する
        a_sceneManager.SetCurrentSceneFilePath({});
    }

    // Registryから削除
    a_assetFilePathRegistry.Erase(a_deleteFilePath);
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryDeleteChange::ApplyDirectoryDelete(const std::filesystem::path& a_deleteFilePath, AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager) const
{
    // Folder配下に存在していたPrefabやSceneJsonを集め
    // File単体削除と同じ処理を流す
    std::unordered_set<std::filesystem::path> l_deleteAssetFilePathSet = {};

    const auto& l_assetFilePathToUUIDMap = a_assetFilePathRegistry.GetREFAssetFilePathToUUIDMap();

    for (const auto& [l_filePath, l_uuid] : l_assetFilePathToUUIDMap)
    {
        // ファイルパスの階層よりも下でないファイルパスなら処理を飛ばす
        if (!IsChildFilePath(l_filePath, a_deleteFilePath)) { continue; }

        l_deleteAssetFilePathSet.emplace(l_filePath);
    }

    // CurrentSceneのPathは別途Path一覧へ追加しておく
    if (const auto& l_currentSceneFilePath = a_sceneManager.GetREFCurrentSceneFilePath();
        !l_currentSceneFilePath.empty() &&
        IsChildFilePath(l_currentSceneFilePath, a_deleteFilePath))
    {
        l_deleteAssetFilePathSet.emplace(l_currentSceneFilePath);
    }

    // 再帰的に処理を行う
    for (const auto& l_deleteAssetFilePath : l_deleteAssetFilePathSet)
    {
        ApplyFileDelete(l_deleteAssetFilePath, a_assetFilePathRegistry, a_sceneManager);
    }
}