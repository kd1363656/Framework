#include "AssetBrowserEditorWindowDirectoryFilePathChange.h"

void FWK::Editor::AssetBrowserEditorWindowDirectoryFilePathChange::Apply(AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager)
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
                                     a_assetFilePathRegistry,
                                     a_sceneManager);

        return;
    }

    ApplyFilePathChange(l_oldFilePath,
                        m_newFilePath,
                        a_assetFilePathRegistry,
                        a_sceneManager);
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryFilePathChange::ApplyFilePathChange(const std::filesystem::path& a_oldFilePath,
                                                                                       const std::filesystem::path& a_newFilePath,
                                                                                             AssetFilePathRegistry& a_assetFilePathRegistry,
                                                                                             SceneManager&          a_sceneManager) const
{
    // AssetBrowser側RegistryはProject全体のAsset情報を持つため、
    // ここからUUIDとAssetTypeを取得して処理を分ける
    const auto* l_assetUUID = a_assetFilePathRegistry.FindPTRAssetUUID(a_oldFilePath);

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
    const auto* l_assetFilePathData = a_assetFilePathRegistry.FindPTRAssetFilePathData(l_copiedAssetUUID);

    if (!l_assetFilePathData)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
                    "AssetBrowserのAssetFilePathRegistry内部で、UUIDに対応するAssetFilePathDataを取得できませんでした。\nOldFilePath : {}\nNewFilePath : {}",
                    a_oldFilePath.string(),
                    a_newFilePath.string());

        return;
    }

    switch (l_assetFilePathData->m_type)
    {
        case Enum::AssetFilePathRegistryType::Prefab:
        {
            ApplyPrefabFilePathChange(a_oldFilePath,
                                      a_newFilePath,
                                      l_copiedAssetUUID,
                                      a_assetFilePathRegistry);

            return;
        }
        break;

        case Enum::AssetFilePathRegistryType::Scene:
        {
            ApplySceneFilePathChange(a_oldFilePath,
                                     a_newFilePath,
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
void FWK::Editor::AssetBrowserEditorWindowDirectoryFilePathChange::ApplyPrefabFilePathChange(const std::filesystem::path& a_oldFilePath,
                                                                                             const std::filesystem::path& a_newFilePath,
                                                                                             const boost::uuids::uuid&    a_prefabUUID,
                                                                                                   AssetFilePathRegistry& a_assetFilePathRegistry) const
{
    if (a_prefabUUID.is_nil()) { return; }

    // PrefabSystemはPrefabUUIDをKeyとしてPrefabを保持しておりFilePathを持たないため
    // Registryの登録Pathだけを変更すればよい
    a_assetFilePathRegistry.ReplaceFilePath(a_oldFilePath, a_newFilePath);
}
void FWK::Editor::AssetBrowserEditorWindowDirectoryFilePathChange::ApplySceneFilePathChange(const std::filesystem::path& a_oldFilePath,
                                                                                            const std::filesystem::path& a_newFilePath,
                                                                                            const boost::uuids::uuid&    a_sceneUUID,
                                                                                                  AssetFilePathRegistry& a_assetFilePathRegistry,
                                                                                                  SceneManager&          a_sceneManager) const
{
    if (a_sceneUUID.is_nil()) { return; }

    // Registryの登録Pathを新しいPathへ変更する
    if (!a_assetFilePathRegistry.ReplaceFilePath(a_oldFilePath, a_newFilePath)) { return; }

    // CurrentSceneはNextSceneDataMapに存在しないため
    // FilePathそのものの一致によって判定する
    if (a_sceneManager.GetREFCurrentSceneFilePath() == a_oldFilePath)
    {
        a_sceneManager.SetCurrentSceneFilePath(a_newFilePath);
    }

    const auto& l_scene = a_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    auto& l_sceneChanger = l_scene->GetMutableREFSceneChanger();

    // NextSceneとして登録済みのときだけSceneChanger側のFilePathも追従する
    if (auto* l_nextScene = l_sceneChanger.FetchMutablePTRNextScene(a_sceneUUID))
    {
        l_nextScene->Load(a_newFilePath);
    }
}
void FWK::Editor::AssetBrowserEditorWindowDirectoryFilePathChange::ApplyDirectoryFilePathChange(const std::filesystem::path& a_oldFilePath,
                                                                                                const std::filesystem::path& a_newFilePath,
                                                                                                      AssetFilePathRegistry& a_assetFilePathRegistry,
                                                                                                      SceneManager&          a_sceneManager) const
{
    // Registryを走査中にReplaceFilePath()すると
    // unordered_map内部の要素が変更されるため、
    // まず変更対象となるOldPathだけを別Containerへコピーする
    std::unordered_set<std::filesystem::path> l_oldeAssetFilePathSet = {};

    for (const auto& [l_assetFilePath, l_uuid] : a_assetFilePathRegistry.GetREFAssetFilePathToUUIDMap())
    {
        if (!IsChildFilePath(l_assetFilePath, a_oldFilePath)) { continue; }

        l_oldeAssetFilePathSet.emplace(l_assetFilePath);
    }

    // CurrentSceneのPathは別途Path一覧へ追加しておく
    if (const auto& l_currentSceneFilePath = a_sceneManager.GetREFCurrentSceneFilePath();
        !l_currentSceneFilePath.empty() &&
        IsChildFilePath(l_currentSceneFilePath, a_oldFilePath))
    {
        l_oldeAssetFilePathSet.emplace(l_currentSceneFilePath);
    }

    for (const auto& l_oldAssetFilePath : l_oldeAssetFilePathSet)
    {
        // OldDirectoryから見た相対Pathを取得する
        const auto& l_relativeAssetFilePath = l_oldAssetFilePath.lexically_relative(a_oldFilePath);

        if (l_relativeAssetFilePath.empty()) { continue; }

        // NewDirectoryへ同じ相対階層をつけなおす
        const auto& l_newAssetFilePath = a_newFilePath / l_relativeAssetFilePath;

        ApplyFilePathChange(l_oldAssetFilePath,
                            l_newAssetFilePath,
                            a_assetFilePathRegistry,
                            a_sceneManager);
    }
}