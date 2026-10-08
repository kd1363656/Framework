#include "WorldOutlinerEditorWindowAssetCreator.h"
#include "../../../../../Application/Application.h"

std::shared_ptr<FWK::GameObject> FWK::Editor::WorldOutlinerEditorWindowAssetCreator::CreateEmptyGameObject(const std::weak_ptr<GameObject>& a_parent, Scene& a_scene) const
{
    auto l_gameObject = std::make_shared<GameObject>();

    // Transform / Hierarchy / ComponentContainerのOwner設定
    l_gameObject->INIT   ();
    l_gameObject->SetName(std::string{ Constant::k_gameObjectString });

    // 親への接続に成功したかどうか
    // 失敗した場合はルートGameObjectとして生成する
    bool l_isParentApplied = false;

    // 親がいる場合はScene登録より先に親子関係を構築する
    // Scene::AddGameObjectは親鎖から実行レベルを計算するため
    // 接続前に登録するとルート扱いになってしまう
    // ApplyParentは親側の子リスト登録とTransformModeの切替まで一括で行う
    if (const auto& l_parent = a_parent.lock())
    {
        auto& l_hierarchy = l_gameObject->GetMutableREFHierarchy();

        // GameObjectHierarchy::ApplyParentは
        // ・旧親からの切り離し(ClearParent)
        // ・親側の子リストへの登録(AddChild)
        // ・自身のTransformComponent::ApplyParent()(親行列への依存切替)
        // まで一括で行うため、ここでTransformを個別に切り替える必要はない
        l_isParentApplied = l_hierarchy.ApplyParent(a_parent);

        if (!l_isParentApplied)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "親GameObjectへの接続に失敗したため、ルートGameObjectとして生成します。");
        }
    }

    // 親を持たない場合は
    // 行列の合成を自身のみで完結するようにする(拡縮・回転・座標のすべてが自身のみに依存)
    if (!l_isParentApplied)
    {
        if (const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock())
        {
            l_transformComponent->ApplyStandalone();
        }
    }

    a_scene.AddGameObject(l_gameObject);

    // GameObject::Cloneと同じく構築完了後にPostDeserializeを行う
    l_gameObject->PostDeserialize();

    return l_gameObject;
}

std::shared_ptr<FWK::GameObject> FWK::Editor::WorldOutlinerEditorWindowAssetCreator::CreateGameObjectFromPrefab(const std::weak_ptr<GameObject>& a_parent, const std::filesystem::path& a_prefabFilePath, Scene& a_scene) const
{
    // RegistryからPrefabUUIDを取得してPrefabとして登録されているファイルのみ受け付ける
    const auto& l_application           = Application::GetInstance                 ();
    const auto& l_assetFilePathRegistry = l_application.GetREFAssetFilePathRegistry();
    const auto* l_prefabUUID            = l_assetFilePathRegistry.FindPTRAssetUUID (a_prefabFilePath);

    if (!l_prefabUUID ||
        l_prefabUUID->is_nil())
    {
        // フォルダやFBXなどPrefabでないファイルが混ざったドロップは通常操作なので警告にはしない
        FWK_ADD_LOG(Constant::k_imguiDebugINFOColor, "Prefabとして登録されていないファイルのため、GameObject生成をスキップしました。\nFilePath : {}", a_prefabFilePath.string());

        return nullptr;
    }

    const auto* l_assetFilePathData = l_assetFilePathRegistry.FindPTRAssetFilePathData(*l_prefabUUID);

    // PrefabUUIDからアセットレジストリーにあるファイルパスを参照する
    if (!l_assetFilePathData ||
        l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Prefab)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugINFOColor, "Prefabではないファイルのため、GameObject生成をスキップしました。\nFilePath : {}", a_prefabFilePath.string());

        return nullptr;
    }

    auto& l_prefabSystem = a_scene.GetMutableREFGameObjectPrefabSystem();

    // SceneのPrefabSystemに未登録ならファイルから読み込んで登録する
    // ネストPrefabの解決やシーン保存時の差分化にPrefabSystem側のJsonが必要なため
    if (!l_prefabSystem.FindPTRPrefab(*l_prefabUUID))
    {
        GameObjectPrefab l_gameObjectPrefab = {};

        l_gameObjectPrefab.Load(l_assetFilePathData->m_assetFilePath);

        if (l_gameObjectPrefab.GetREFJson().is_null())
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "PrefabのJsonを読み込めなかったため、GameObject生成をスキップしました。\nFilePath : {}", l_assetFilePathData->m_assetFilePath.string());

            return nullptr;
        }

        l_prefabSystem.AddPrefab(l_gameObjectPrefab, *l_prefabUUID);
    }

    const auto* l_prefab = l_prefabSystem.FindPTRPrefab(*l_prefabUUID);

    if (!l_prefab) { return nullptr; }

    // GameObjectはINIT内でweak_from_thisを使うため
    // shared_ptr管理でなければbad_weak_ptrが投げられる
    auto l_gameObject = std::make_shared<GameObject>();

    l_gameObject->INIT();

    // PrefabのJsonをフル形式として読み込み
    // SceneへのAddGameObject登録と子の再帰生成はDeserializePrefab内部で行われる
    l_gameObject->DeserializePrefab(l_prefab->GetREFJson(), l_prefabSystem, a_scene);

    // 親が指定されている場合は親子関係を構築する
    // 無効・破棄済みの親ならルートGameObjectとして残す
    if (const auto& l_parent = a_parent.lock();
        l_parent &&
        !l_parent->GetVALIsDestroyed())
    {
        auto& l_hierarchy = l_gameObject->GetMutableREFHierarchy();

        // ApplyParentは親側の子リスト登録とTransformModeの切替まで一括で行う
        l_hierarchy.ApplyParent(l_parent);

        // 階層の深さが変わったため実行レベルを再構築する
        a_scene.RebuildGameObjectExecutionLevelList();
    }

    // 構築完了後にTransform確定やComponentのPostDeserializeを行う
    // Cloneと同じく親子接続後に呼ぶ
    l_gameObject->PostDeserialize();

    // ドロップ配置は座標(0,0,0)で生成する
    // 親がいればローカル座標0、いなければワールド座標0になる
    if (const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();
        l_transformComponent)
    {
        l_transformComponent->ApplyTransformPosition(TypeAlias::Math::Vector3::Zero);
    }

    return l_gameObject;
}

std::vector<std::weak_ptr<FWK::GameObject>> FWK::Editor::WorldOutlinerEditorWindowAssetCreator::CreateGameObjectFromPrefabDrop(const std::vector<std::filesystem::path>& a_droppedFilePathList, const std::weak_ptr<GameObject>& a_parent, Scene& a_scene) const
{
    std::vector<std::weak_ptr<GameObject>> l_createdList = {};

    l_createdList.reserve(a_droppedFilePathList.size());

    // Prefabとして登録されていないファイルはCreateGameObjectFromPrefab内で弾かれる
    // ドロップされたPrefabファイルの分だけGameObjectを生成する
    for (const auto& l_filePath : a_droppedFilePathList)
    {
        const auto& l_gameObject = CreateGameObjectFromPrefab(a_parent, l_filePath, a_scene);

        if (!l_gameObject) { continue; }

        l_createdList.emplace_back(l_gameObject);
    }

    return l_createdList;
}