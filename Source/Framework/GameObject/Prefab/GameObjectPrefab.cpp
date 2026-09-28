#include "GameObjectPrefab.h"

void FWK::GameObjectPrefab::Load(const std::filesystem::path& a_filePath)
{
    if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerJsonExtension)) { return; }

    const auto& l_rootJson = Utility::LoadJsonFile(a_filePath);

    m_jsonConverter.Load(l_rootJson, *this);
}

bool FWK::GameObjectPrefab::Save(const std::filesystem::path& a_filePath, GameObject& a_gameObject)
{
    if (a_filePath.empty() ||
        a_filePath.extension() != Constant::k_lowerJsonExtension)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "GameObjectPrefabの保存先FilePathが無効です。\nFilePath : {}", a_filePath.string());

        return false;
    }

    // プレハブ化処理
    // 新しいPrefabUUIDを生成する
    auto& l_uuidManager = Utility::UUIDManager::GetInstance();

    const auto& l_prefabUUID = l_uuidManager.GenerateVALUUID();

    // 自身とすべての子をIsPrefabOrigin=trueにしてPrefabUUIDを設定する
    // PrefabHierarchyNodeUUIDがnilなら生成する
    // 既に別のPrefabのインスタンスである子には伝播しない
    a_gameObject.ConvertToPrefab(l_prefabUUID);

    // すべてのRemovedUUIDSetをクリアする
    // プレハブは新しい「元」なので差分（削除）情報を保持しない
    a_gameObject.ClearAllPrefabRemovedUUIDSet();


    return m_jsonConverter.Save(a_filePath, a_gameObject, *this);
}