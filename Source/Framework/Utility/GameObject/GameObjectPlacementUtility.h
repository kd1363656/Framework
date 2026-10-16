#pragma once

namespace FWK::Utility
{
    inline Struct::GameObjectPlacement FetchVALGameObjectPlacement(const std::weak_ptr<GameObject>& a_gameObject, const Scene& a_scene)
    {
        Struct::GameObjectPlacement l_placement = {};

        const auto& l_gameObject = a_gameObject.lock();

        if (!l_gameObject) { return l_placement; }

        const auto& l_hierarchy = l_gameObject->GetREFHierarchy();
        const auto& l_parent    = l_hierarchy.GetREFParent     ().lock();

        // 並び順は添字ではなくSceneInstanceUUIDの並びで記録する
        // 添字だと途中で別のGameObjectが増減したときに指す場所がずれるが、
        // UUIDの並びなら残っているものだけで並びを組み直せる
        // 例 : 兄弟が[A, B, C]でBを記録 → { 親のUUID, [A, B, C] }
        if (l_parent)
        {
            l_placement.m_parentUUID = l_parent->GetREFSceneInstanceUUID();

            const auto& l_parentHierarchy             = l_parent->GetREFHierarchy                          ();
            const auto& l_childSmartPointerVectorList = l_parentHierarchy.GetREFChildSmartPointerVectorList();
            const auto& l_childDataList               = l_childSmartPointerVectorList.GetREFElementDataList();

            l_placement.m_siblingUUIDList.reserve(l_childDataList.size());

            for (const auto& l_childData : l_childDataList)
            {
                const auto& l_child = l_childData.m_type.lock();

                if (!l_child) { continue; }

                l_placement.m_siblingUUIDList.emplace_back(l_child->GetREFSceneInstanceUUID());
            }

            return l_placement;
        }

        // 親がいなければシーンのルートの並びを記録する
        // シーンのGameObjectListには子も混ざっているため親を持たないものだけを集める
        for (const auto& l_sceneGameObject : a_scene.GetREFGameObjectList())
        {
            if (!l_sceneGameObject) { continue; }

            if (const auto& l_sceneGameObjectHierarchy = l_sceneGameObject->GetREFHierarchy();
                !l_sceneGameObjectHierarchy.GetREFParent().expired())
            {
                continue;
            }

            l_placement.m_siblingUUIDList.emplace_back(l_sceneGameObject->GetREFSceneInstanceUUID());
        }

        return l_placement;
    }

    inline std::size_t FetchVALSiblingOrder(const std::unordered_map<boost::uuids::uuid, std::size_t>& a_siblingOrderMap, const std::shared_ptr<GameObject>& a_gameObject)
    {
        // 記録に無いGameObject(記録した後に増えたもの)は末尾へ回す
        if (!a_gameObject) { return a_siblingOrderMap.size(); }

        const auto& l_itr = a_siblingOrderMap.find(a_gameObject->GetREFSceneInstanceUUID());

        if (l_itr == a_siblingOrderMap.end()) { return a_siblingOrderMap.size(); }

        return l_itr->second;
    }

    inline void ApplySiblingOrder(const std::vector<boost::uuids::uuid>& a_siblingUUIDList, const std::shared_ptr<GameObject>& a_parent, Scene& a_scene)
    {
        // UUID → 記録した並びの順番 の表を作る
        // 例 : [A, B, C] → { A : 0, B : 1, C : 2 }
        std::unordered_map<boost::uuids::uuid, std::size_t> l_siblingOrderMap = {};

        l_siblingOrderMap.reserve(a_siblingUUIDList.size());

        for (std::size_t l_index = 0ULL; l_index < a_siblingUUIDList.size(); ++l_index)
        {
            l_siblingOrderMap.try_emplace(a_siblingUUIDList[l_index], l_index);
        }

        // 親がいるなら親の子リストを記録した順番で並べ替える
        // stable_sortなので記録に無いもの同士は今の並びのまま末尾に残る
        if (a_parent)
        {
            auto& l_parentHierarchy             = a_parent->GetMutableREFHierarchy                          ();
            auto& l_childSmartPointerVectorList = l_parentHierarchy.GetMutableREFChildSmartPointerVectorList();
            auto& l_childDataList               = l_childSmartPointerVectorList.GetMutableREFElementDataList();

            std::ranges::stable_sort(l_childDataList,
                                     [&l_siblingOrderMap](const auto& a_left, const auto& a_right)
                                     {
                                         return FetchVALSiblingOrder(l_siblingOrderMap, a_left.m_type.lock()) <
                                                FetchVALSiblingOrder(l_siblingOrderMap, a_right.m_type.lock());
                                     });

            return;
        }

        // ルートはシーンのGameObjectListの中で子と混ざって並んでいるため、
        // ルートだけを取り出して並べ替え、ルートが入っていた場所へ順番に戻す
        // 例 : [B, b1, A] (b1はBの子) を[A, B]の順へ → ルートの場所0と2へA, Bを入れて[A, b1, B]
        auto& l_gameObjectList = a_scene.GetMutableREFGameObjectList();

        std::vector<std::shared_ptr<GameObject>> l_rootGameObjectList = {};
        std::vector<std::size_t>                 l_rootIndexList      = {};

        for (std::size_t l_index = 0ULL; l_index < l_gameObjectList.size(); ++l_index)
        {
            const auto& l_gameObject = l_gameObjectList[l_index];

            if (!l_gameObject) { continue; }

            if (const auto& l_hierarchy = l_gameObject->GetREFHierarchy();
                !l_hierarchy.GetREFParent().expired())
            {
                continue;
            }

            l_rootGameObjectList.emplace_back(l_gameObject);
            l_rootIndexList.emplace_back     (l_index);
        }

        std::ranges::stable_sort(l_rootGameObjectList,
                                 [&l_siblingOrderMap](const auto& a_left, const auto& a_right)
                                 {
                                     return FetchVALSiblingOrder(l_siblingOrderMap, a_left) <
                                            FetchVALSiblingOrder(l_siblingOrderMap, a_right);
                                 });

        for (std::size_t l_index = 0ULL; l_index < l_rootGameObjectList.size(); ++l_index)
        {
            l_gameObjectList[l_rootIndexList[l_index]] = l_rootGameObjectList[l_index];
        }
    }

    inline bool ApplyGameObjectPlacement(const std::weak_ptr<GameObject>& a_gameObject, const Struct::GameObjectPlacement& a_placement, Scene& a_scene)
    {
        const auto& l_gameObject = a_gameObject.lock();

        if (!l_gameObject) { return false; }

        // 親をUUIDから探す
        // nilは親なし(シーンのルート)を表す
        std::shared_ptr<GameObject> l_parent = {};

        if (!a_placement.m_parentUUID.is_nil())
        {
            l_parent = a_scene.FindVALGameObject(a_placement.m_parentUUID).lock();

            // 親が既にシーンから無くなっているなら元の場所へは戻せない
            if (!l_parent ||
                l_parent->GetVALIsDestroyed())
            {
                return false;
            }
        }

        // 循環する親子付けなどはApplyParent側で弾かれる
        if (auto& l_hierarchy = l_gameObject->GetMutableREFHierarchy();
            !l_hierarchy.ApplyParent(l_parent))
        {
            return false;
        }

        if (l_parent)
        {
            // Prefab由来の子を親から外すと、親側に「Prefabから消した子」として記録される
            // 戻した子がその記録に残っているとPrefabとの差分で「消した子」として保存されるため記録から消す
            auto& l_parentHierarchy           = l_parent->GetMutableREFHierarchy                        ();
            auto& l_prefabRemovedChildUUIDSet = l_parentHierarchy.GetMutableREFPrefabRemovedChildUUIDSet();

            l_prefabRemovedChildUUIDSet.erase(l_gameObject->GetREFPrefabHierarchyNodeUUID());
        }

        ApplySiblingOrder(a_placement.m_siblingUUIDList, l_parent, a_scene);

        // 階層の深さが変わり得るため実行レベルを作り直す
        a_scene.RebuildGameObjectExecutionLevelList();

        return true;
    }
}