#include "GameObjectPrefab.h"

void FWK::GameObjectPrefab::Load(const std::filesystem::path& a_filePath)
{
    if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerJsonExtension)) { return; }

    const auto& l_rootJson = Utility::LoadJsonFile(a_filePath);

    m_jsonConverter.Load(l_rootJson, *this);
}

bool FWK::GameObjectPrefab::Save(const std::filesystem::path&       a_filePath, 
                                 const boost::uuids::uuid&          a_prefabUUID, 
                                       SceneGameObjectPrefabSystem& a_prefabSystem,
                                       GameObject&                  a_gameObject)
{
    if (a_filePath.empty() ||
        a_filePath.extension() != Constant::k_lowerJsonExtension)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "GameObjectPrefabの保存先FilePathが無効です。\nFilePath : {}", a_filePath.string());

        return false;
    }

    // プレハブ化処理
    // PrefabUUIDは呼び出し側(AssetFilePathRegistryへ登録済みのUUID)から受け取る
    // ここで新規生成するとRegistry登録UUIDとノード埋め込みUUIDが不一致になる
    if (a_prefabUUID.is_nil())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "PrefabUUIDがnilのためGameObjectPrefabを保存できません。");
     
        return false;
    }
     
    // 自身とすべての子をIsPrefabOrigin=trueにしてPrefabUUIDを設定する
    // PrefabHierarchyNodeUUIDがnilなら生成する
    // 既に別のPrefabのインスタンスである子には伝播しない
    a_gameObject.ConvertToPrefab(a_prefabUUID);
    
    // PrefabのルートノードはHierarchy内の子ではないため
    // 子照合用のPrefabHierarchyNodeUUIDはnilへ戻す
    // (ConvertToPrefabでnilなら自動発行されるため発行後にnilへ戻す)
    // nilのままにすることでPrefabから生成したGameObjectを
    // 他のGameObjectの子にした際にAddChildが毎回新規UUIDを発行でき
    // 同一Prefab由来のインスタンス同士でUUIDが重複しない
    a_gameObject.SetPrefabHierarchyNodeUUID({});

    // すべてのRemovedUUIDSetをクリアする
    // プレハブは新しい「元」なので差分（削除）情報を保持しない
    a_gameObject.ClearAllPrefabRemovedUUIDSet();

    return m_jsonConverter.Save(a_filePath, 
                                a_gameObject, 
                                a_prefabSystem,
                                *this);
}