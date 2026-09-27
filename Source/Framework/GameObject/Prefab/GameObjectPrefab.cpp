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
    // 1. 新しいPrefabUUIDを生成する
    auto& l_uuidManager = Utility::UUIDManager::GetInstance();
 
    const auto& l_prefabUUID = l_uuidManager.GenerateVALUUID();
 

    return m_jsonConverter.Save(a_filePath, a_gameObject, *this);
}