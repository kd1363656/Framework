#include "WorldOutlinerEditorWindowAssetCreator.h"

std::weak_ptr<FWK::GameObject> FWK::Editor::WorldOutlinerEditorWindowAssetCreator::CreateEmptyGameObject(Scene& a_scene) const
{
    auto l_gameObject = std::make_shared<GameObject>();

    l_gameObject->INIT();

    // SceneInstanceNameを"GameObject"に設定
    // からかつm_prefabSceneInstanceNUMが無効値の場合は描画側で"GameObject"と表示されるが
    // 内部データにも明示的に設定しておく
    l_gameObject->SetSceneInstanceName(std::string{ Constant::k_gameObjectString });

    // Sceneへ追加
    // AddGameObject内部でSceneInstanceUUIDが発行・UUIDRegistryへ登録される
    a_scene.AddGameObject(l_gameObject);

    const auto& l_transformComponentWeak = l_gameObject->GetVALTransformComponent          ();
    const auto& l_transformComponent     = l_transformComponentWeak.lock                   ();
          auto& l_componentUUIDRegistry  = l_gameObject->GetMutableREFComponentUUIDRegistry();

    if (l_transformComponent)
    {
        boost::uuids::uuid l_transformUUID = {};

        // UUIDRegistry::Addはweak_ptrを受け取り、UUIDを発行して登録する
        if (l_componentUUIDRegistry.Add(l_transformComponent, l_transformUUID))
        {
            // 発行されたUUIDをTransformComponentへ設定
            l_transformComponent->SetUUID(l_transformUUID);

            // プレハブ登録されていないゲームオブジェクトは親子関係を構築できないため
            // Standaloneで行列を構築
            l_transformComponent->ApplyStandalone();
        }
    }

    // シーンに格納しているため戻り値にしても問題ない
    return l_gameObject;
}

std::vector<boost::uuids::uuid> FWK::Editor::WorldOutlinerEditorWindowAssetCreator::DuplicateGameObjects(const std::vector<boost::uuids::uuid>& a_selectedUUIDList, Scene& a_scene) const
{
    std::vector<boost::uuids::uuid> l_duplicatedUUIDLIst = {};

    // 複製数を選択したUUIDListの数予約
    l_duplicatedUUIDLIst.reserve(a_selectedUUIDList.size());

    for (const auto& l_sourceUUID : a_selectedUUIDList)
    {
        // UUIDに対応するゲームオブジェクトを取得
        const auto& l_source = a_scene.FindVALGameObject(l_sourceUUID).lock();

        if (!l_source ||
             l_source->GetVALIsDestroyed())
        {
            continue;
        }

        // 複製
        const auto& l_duplicate = DuplicateGameObjectRecursive(*l_source, a_scene);

        if (!l_duplicate) { continue; }

        // 元と同じ親の子として追加
        // 親子関係の構築はOperationの責務だが
        // 複製直後の親子関係復元はCreator内で完結させる
        if (const auto& l_sourceParent = l_source->GetREFParent().lock();
            l_sourceParent)
        {
            // 親のChildListへ追加
            l_duplicate->SetParent                                  (l_sourceParent);
            l_sourceParent->GetMutableREFChildSmartPointerVectorList().Add(l_duplicate);

            // Transformを親追従モードへ
            if (const auto& l_transformComponent = l_duplicate->GetVALTransformComponent().lock())
            {
                l_transformComponent->ApplyParent(l_sourceParent);
            }
        }
        else
        {
            // 元がRootなら複製もRoot
            if (const auto& l_transformComponent = l_duplicate->GetVALTransformComponent().lock())
            {
                l_transformComponent->ApplyStandalone();
            }
        }

        l_duplicatedUUIDLIst.emplace_back(l_duplicate->GetREFSceneInstanceUUID());
    }

    // 親子関係が変化したためSceneの実行レベルリストをDirtyにする
    a_scene.SetIsGameObjectExecutionLevelListDirty(true);

    return l_duplicatedUUIDLIst;
}

std::shared_ptr<FWK::GameObject> FWK::Editor::WorldOutlinerEditorWindowAssetCreator::DuplicateGameObjectRecursive(const GameObject& a_source, Scene& a_scene) const
{
    // 新規GameObjectを作成
    auto l_duplicate = std::make_shared<GameObject>();

    l_duplicate->INIT();

    // 元のSceneInstanceNameをコピー
    l_duplicate->SetSceneInstanceName(a_source.GetREFSceneInstanceName());

    // Sceneへ追加(SceneInstanceUUIDが発行・登録される)
    a_scene.AddGameObject(l_duplicate);

    // transformComponentの値をコピー
    if (const auto& l_sourceTransformComponent = a_source.GetVALTransformComponent().lock())
    {
        if (const auto& l_duplicateTransform = l_duplicate->GetVALTransformComponent().lock())
        {
            // Transform値をコピー
            const auto& l_initialTransform = l_sourceTransformComponent->GetREFInitialSettingTransform();

            l_duplicateTransform->SetTransformScale   (l_initialTransform.m_scale);
            l_duplicateTransform->SetTransformRotation(l_initialTransform.m_rotation);
            l_duplicateTransform->SetTransformPosition(l_initialTransform.m_position);

            // TransformComponentのUUIDを発行して登録
            const auto& l_transformWeak                = l_duplicate->GetVALTransformComponent ();
                  auto  l_sourceTransformComponentUUID = l_sourceTransformComponent->GetREFUUID();
            
            // Sourceと同じTransformComponentのUUIDを割り当てる
            if (auto& l_componentUUIDRegistry = l_duplicate->GetMutableREFComponentUUIDRegistry();
                l_componentUUIDRegistry.Add(l_transformWeak, l_sourceTransformComponentUUID))
            {
                l_duplicateTransform->SetUUID(l_sourceTransformComponentUUID);
            }
        }
    }

    // 子GameObjectを再帰的に複製
    const auto& l_childList     = a_source.GetREFChildSmartPointerVectorList();
    const auto& l_childDataList = l_childList.GetREFElementDataList         ();

    for (const auto& l_childData : l_childDataList)
    {
        const auto& l_child = l_childData.m_type.lock();

        if (!l_child ||
            l_child->GetVALIsDestroyed())
        {
            continue;
        }

        const auto& l_childDuplicate = DuplicateGameObjectRecursive(*l_child, a_scene);

        if (l_childDuplicate)
        {
            // 親子関係を構築
            l_childDuplicate->SetParent                          (l_duplicate);
            l_duplicate->GetMutableREFChildSmartPointerVectorList().Add(l_childDuplicate);
        }

        if (const auto& l_transformComponent = l_childDuplicate->GetVALTransformComponent().lock())
        {
            l_transformComponent->ApplyParent(l_duplicate);
        }
    }

    return l_duplicate;
}