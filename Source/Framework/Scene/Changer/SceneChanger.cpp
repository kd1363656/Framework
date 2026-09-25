#include "SceneChanger.h"

void FWK::SceneChanger::INIT()
{
    m_nextSceneLoadFilePathMap.clear();

    m_sceneChangeEventObserver.INIT();
}

bool FWK::SceneChanger::AddNextSceneLoadFilePath(const boost::uuids::uuid& a_sceneUUID)
{
    if (a_sceneUUID.is_nil())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "シーン遷移に追加しようとしたUUIDが無効です、追加しようとしたシーン名の確認をしてください。");

        return false;
    }

    return m_nextSceneLoadFilePathMap.try_emplace(a_sceneUUID, l_nextSceneLoadFilePath).second;
}

bool FWK::SceneChanger::RemoveNextSceneLoadFilePath(const boost::uuids::uuid& a_sceneUUID)
{
    return false;
}

bool FWK::SceneChanger::AddNextSceneLoadFilePath(const std::filesystem::path& a_filePath, const boost::uuids::uuid& a_sceneUUID, AssetFilePathRegistry& a_assetFilePathRegistry)
{
    if (a_sceneUUID.is_nil())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetFilePathRegistryへ追加しようとしたSceneUUIDが無効です。");

        return false;
    }

    if (a_filePath.empty())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetFilePathRegistryへ追加しようとしたSceneFilePathが空です。");

        return false;
    }

    if (!Utility::CanLoadFilePath(
        a_filePath,
        Constant::k_lowerJsonExtension))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "追加しようとしたSceneFilePathが無効です。\nFilePath : {}", a_filePath.string());

        return false;
    }

    // まずSceneManager側AssetRegistryへSceneとして登録する
    if (!a_assetFilePathRegistry.Add(a_filePath, a_sceneUUID, Enum::AssetFilePathRegistryType::Scene)) { return false; }

    // Registry登録に成功した後、
    // UUIDだけ版を使ってRegistryから正式なPathを取得し、
    // NextSceneLoadFilePathMapへ登録する
    if (AddNextSceneLoadFilePath(a_assetFilePathRegistry, a_sceneUUID)) { return true; }

    // NextSceneLoadFilePathMapへの登録に失敗した場合
    // RegistryだけにSceneが残る中途半端な状態を防ぐ
    a_assetFilePathRegistry.Erase(a_filePath);

    return false;
}

bool FWK::SceneChanger::RemoveNextSceneLoadFilePath(const boost::uuids::uuid& a_sceneUUID, AssetFilePathRegistry& a_assetFilePathRegistry)
{
    if (a_sceneUUID.is_nil()) { return false; }

    bool l_isRemoved = false;

    const auto* l_assetFilePathData = a_assetFilePathRegistry.FindPTRAssetFilePathData(a_sceneUUID);

    if (l_assetFilePathData &&
        l_assetFilePathData->m_type == Enum::AssetFilePathRegistryType::Scene)
    {
        // Erase()を呼ぶとRegistry内部Dataが消えるため、
        // 先にPathを値として保持する。
        const std::filesystem::path l_sceneFilePath = l_assetFilePathData->m_assetFilePath;

        if (m_assetFilePathRegistry.Erase(
            l_sceneFilePath))
        {
            l_isRemoved = true;
        }
    }

    // unordered_map::erase(Key)は
    // 実際に削除した要素数を返す
    // staleなMap状態だけが残っていた場合でも
    // ここで削除して同期状態へ戻す
    if (m_nextSceneLoadFilePathMap.erase(a_sceneUUID) != Constant::k_noErasedElementCount)
    {
        l_isRemoved = true;
    }

    return l_isRemoved;
}

bool FWK::SceneChanger::ReplaceSceneFilePath(const std::filesystem::path& a_oldSceneFilePath, 
                                             const std::filesystem::path& a_newSceneFilePath, 
                                             const boost::uuids::uuid&    a_sceneUUID, 
                                                   AssetFilePathRegistry& a_assetFilePathRegistry)
{
    if (a_oldSceneFilePath.empty() ||
        a_newSceneFilePath.empty() ||
        a_sceneUUID.is_nil())
    {
        return false;
    }

    if (a_oldSceneFilePath == a_newSceneFilePath) { return true; }

    const auto* l_assetFilePathData = a_assetFilePathRegistry.FindPTRAssetFilePathData(a_sceneUUID);

    if (!l_assetFilePathData) { return false; }

    if (l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Scene ||
        l_assetFilePathData->m_assetFilePath != a_oldSceneFilePath)
    {
        return false;
    }

    const auto& l_nextSceneFilePathITR = m_nextSceneLoadFilePathMap.find(a_sceneUUID);

    if (l_nextSceneFilePathITR == m_nextSceneLoadFilePathMap.end())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneManagerのAssetFilePathRegistryにはSceneが存在しますが、NextSceneLoadFilePathMapに存在しません。");

        return false;
    }


    if (l_nextSceneFilePathITR->second !=
        a_oldSceneFilePath)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetFilePathRegistryとNextSceneLoadFilePathMapのSceneFilePathが一致していません。");

        return false;
    }

    // RegistryのUUIDは変更せずPathだけ変更する
    if (!a_assetFilePathRegistry.ReplaceFilePath(a_oldSceneFilePath, a_newSceneFilePath))
    {
        return false;
    }

    // Registry変更に成功した後でScene専用Indexも追従する
    l_nextSceneFilePathITR->second = a_newSceneFilePath;

    return true;
}