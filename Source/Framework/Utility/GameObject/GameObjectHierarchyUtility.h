#pragma once

namespace FWK::Utility
{
    inline void CollectDescendantGameObjectList(const std::weak_ptr<GameObject>& a_gameObject, std::vector<std::shared_ptr<GameObject>>& a_outGameObjectList)
    {
        // 指定したGameObjectの子・孫・ひ孫…(子孫すべて)を集めて、a_outGameObjectListの末尾へ追加する関数
        // 指定したGameObject自身は追加しない
        // 追加される順番は「親 → その子 → その孫」の順になる
        // weak_ptrは、指す先がすでに消えている可能性がある
        // そのため使う前にlock()でshared_ptrへ変換して、有効かどうかを確認する
        const auto& l_gameObject = a_gameObject.lock();

        // 消えているなら集める子孫がいないため、何もせずに終了する
        if (!l_gameObject) { return; }

        // GameObjectの階層情報(Hierarchy)から、子のリストを取り出す
        // 子はSmartPointerVectorListという入れ物に入っていて、
        // GetREFElementDataList()を呼ぶと「子1つ分のデータ」の配列が取れる
        const auto& l_hierarchy                   = l_gameObject->GetREFHierarchy                      ();
        const auto& l_childSmartPointerVectorList = l_hierarchy.GetREFChildSmartPointerVectorList      ();
        const auto& l_childDataList               = l_childSmartPointerVectorList.GetREFElementDataList();

        // 子を1つずつ順番に調べる
        for (const auto& l_childData : l_childDataList)
        {
            // 子のデータの中のm_typeが、子GameObjectへのweak_ptr
            // ここでもlock()で有効かどうかを確認する
            const auto& l_child = l_childData.m_type.lock();

            // 子がすでに消えている、またはDestroy()済み(削除予定)なら、集めずに次の子へ進む
            if (!l_child ||
                l_child->GetVALIsDestroyed())
            {
                continue;
            }

            // この子を結果のリストへ追加する
            // 子を先に追加してから孫を集めるため、リストは「親 → 子 → 孫」の順に並ぶ
            a_outGameObjectList.emplace_back(l_child);

            // この子の子(孫)以降も、同じ処理で集める
            // 関数の中から自分自身を呼ぶ書き方を再帰呼び出しと呼ぶ
            // 子がいなくなるところまで、この呼び出しが繰り返される
            CollectDescendantGameObjectList(l_child, a_outGameObjectList);
        }
    }

    inline void DetachGameObjectSubtree(const std::vector<std::shared_ptr<GameObject>>& a_subtreeList, Scene& a_scene)
    {
        // サブツリー(ルートと、そのすべての子孫のまとまり)を、シーンから取り外す関数
        // a_subtreeListの先頭がルート、その後ろに子孫が「親 → 子」の順で並んでいる想定
        // 取り外すだけでGameObjectは破棄しない
        // 実体は呼び出し側がshared_ptrで持ち続けるため、あとでシーンへ戻すこともできる
        // 取り外す対象が1つもなければ何もしない
        if (a_subtreeList.empty()) { return; }

        // ルートも子孫も、全員をシーンの管理から外す
        // (シーンのGameObjectリスト・UUIDの登録・実行順のリストから取り除かれる)
        // UnregisterGameObjectは親子関係を解除しない
        // そのため子孫同士の「親と子のつながり」はそのまま残る
        for (const auto& l_gameObject : a_subtreeList)
        {
            a_scene.UnregisterGameObject(l_gameObject);
        }

        // 先頭の要素がサブツリーのルート
        const auto& l_root = a_subtreeList.front();

        // ルートだけ、親との親子関係を解除する
        // 親がいる場合は、親の子リストからルートが外れる
        // 子孫はルートの下についたままなので、サブツリー全体がひとかたまりのまま切り離される
        auto& l_rootHierarchy = l_root->GetMutableREFHierarchy();

        l_rootHierarchy.ClearParent();
    }

    inline Struct::ReparentGameObjectState FetchVALReparentGameObjectState(const std::weak_ptr<GameObject>& a_gameObject, const Scene& a_scene)
    {
        // 指定したGameObjectの「親」と「兄弟(同じ親を持つGameObject)の並び順」を記録して返す関数
        // 並び順は、GameObjectそのものではなくSceneInstanceUUIDの配列として記録する
        // UUIDで記録しておくと、あとでUndo・Redoをするときに
        // 「UUIDからGameObjectを探して、記録した順番に並べ直す」ことができる
        // 添字(何番目か)で記録するより、リストが多少変わってもずれにくい
        // 親がいない(ルートの)場合は、親のUUIDはnilのままで、兄弟はシーンのルート全員になる
        Struct::ReparentGameObjectState l_state = {};

        const auto& l_gameObject = a_gameObject.lock();

        // GameObjectが消えている場合は、空の記録をそのまま返す
        if (!l_gameObject) { return l_state; }

        // 親のGameObjectを取り出す
        // l_siblingUUIDListは、記録の中の「兄弟のUUIDの配列」の別名
        // 長い名前を何度も書かなくて済むように参照で受けておく
        const auto& l_hierarchy       = l_gameObject->GetREFHierarchy();
        const auto& l_parent          = l_hierarchy.GetREFParent     ().lock();
              auto& l_siblingUUIDList = l_state.m_siblingUUIDList;

        // 親がいない場合は、シーンのルート(親を持たないGameObject)全員が兄弟になる
        if (!l_parent)
        {
            // シーンは、ルートも子も区別せず、すべてのGameObjectを1つのリストで持っている
            // そこから親を持たないものだけを、リストの順番どおりに拾っていく
            const auto& l_sceneGameObjectList = a_scene.GetREFGameObjectList();

            // 最大でシーン全体の数だけ追加されるため、先に領域を確保しておく
            l_siblingUUIDList.reserve(l_sceneGameObjectList.size());

            for (const auto& l_sceneGameObject : l_sceneGameObjectList)
            {
                if (!l_sceneGameObject) { continue; }

                // 親がいる(=ルートではない)GameObjectは、ルートの並びには含めない
                const auto& l_sceneGameObjectHierarchy = l_sceneGameObject->GetREFHierarchy();

                if (!l_sceneGameObjectHierarchy.GetREFParent().expired()) { continue; }

                // ルートのUUIDを、リストの順番どおりに記録する
                l_siblingUUIDList.emplace_back(l_sceneGameObject->GetREFSceneInstanceUUID());
            }

            // 親がいないため、親のUUIDは初期値(nil)のまま返す
            return l_state;
        }

        // 親がいる場合は、親のUUIDを記録する
        l_state.m_parentUUID = l_parent->GetREFSceneInstanceUUID();

        // 兄弟は、親の子リストに入っているGameObject全員(自分自身も含む)
        // 親の階層情報(Hierarchy)から、子のリストを取り出す
        const auto& l_parentHierarchy             = l_parent->GetREFHierarchy                          ();
        const auto& l_childSmartPointerVectorList = l_parentHierarchy.GetREFChildSmartPointerVectorList();
        const auto& l_childDataList               = l_childSmartPointerVectorList.GetREFElementDataList();

        // 子の数だけ追加されるため、先に領域を確保しておく
        l_siblingUUIDList.reserve(l_childDataList.size());

        for (const auto& l_childData : l_childDataList)
        {
            // 子のデータの中のm_typeが子GameObjectへのweak_ptr
            // lock()で有効かどうかを確認し、消えているものは記録しない
            const auto& l_child = l_childData.m_type.lock();

            if (!l_child) { continue; }

            // 親の子リストの順番どおりに、子のUUIDを記録する
            l_siblingUUIDList.emplace_back(l_child->GetREFSceneInstanceUUID());
        }

        return l_state;
    }

    inline std::size_t FetchVALGameObjectSiblingRank(const std::unordered_map<boost::uuids::uuid, std::size_t>& a_rankMap, const std::weak_ptr<GameObject>& a_gameObject)
    {
        // GameObjectが、記録した並びの中で「何番目か(順位)」を返す関数
        // a_rankMapは「UUID → 順位(0から始まる)」の対応表
        // 並べ替えのとき、順位が小さいものほど前に並べる
        // 順位が分からない場合は、対応表の要素数(どの順位よりも大きい値)を返す
        // これにより、記録にないGameObjectは最後尾に並ぶ
        const auto& l_gameObject = a_gameObject.lock();

        // GameObjectが消えている場合は最後尾の順位にする
        if (!l_gameObject) { return a_rankMap.size(); }

        // GameObjectのUUIDで対応表を検索する
        // find()は見つかれば、その要素を指すイテレータを返し、見つからなければend()を返す
        const auto& l_rankITR = a_rankMap.find(l_gameObject->GetREFSceneInstanceUUID());

        // 記録にないGameObjectは最後尾の順位にする
        if (l_rankITR == a_rankMap.end()) { return a_rankMap.size(); }

        // 見つかった場合は、対応表に書かれている順位を返す
        // (イテレータのsecondが、UUIDに対応する値=順位)
        return l_rankITR->second;
    }

    inline void ApplyGameObjectSiblingOrder(const std::weak_ptr<GameObject>& a_parent, const Struct::ReparentGameObjectState& a_state, Scene& a_scene)
    {
        // 記録しておいた兄弟のUUIDの並び(a_state)に合わせて、兄弟の並び順を並べ直す関数
        // a_parentが有効なら、その親の子リストを並べ直す
        // a_parentが無効(親がいない)なら、シーンのルートの並びを並べ直す
        // 並べ直しの流れは次のとおり
        //   1. 記録したUUIDから「UUID → 順位」の対応表を作る
        //   2. その順位が小さい順になるように、実際のリストを並べ替える
        // 記録に載っていないGameObjectは、順位が最後尾になるため、リストの後ろに残る
        // 1. 記録の何番目にあるかを「順位」として、UUIDと対応づける表を作る
        //    例 : 記録が[C, A, B]なら、C = 0番、A = 1番、B = 2番
        std::unordered_map<boost::uuids::uuid, std::size_t> l_rankMap = {};

        l_rankMap.reserve(a_state.m_siblingUUIDList.size());

        for (std::size_t l_i = 0ULL; l_i < a_state.m_siblingUUIDList.size(); ++l_i)
        {
            // try_emplaceは、同じUUIDがすでに表にある場合は何もしない
            // (同じUUIDが2つ記録されていても、先に出てきた順位が使われる)
            l_rankMap.try_emplace(a_state.m_siblingUUIDList[l_i], l_i);
        }

        // 親がいる場合は、親の子リストを並べ直す
        if (const auto& l_parent = a_parent.lock();
            l_parent)
        {
            // 親の階層情報(Hierarchy)から、並べ替えるための子リストを取り出す
            // 並べ替えて中身を書き換えるため、Mutable(変更できる)な取得関数を使う
            auto& l_parentHierarchy             = l_parent->GetMutableREFHierarchy                          ();
            auto& l_childSmartPointerVectorList = l_parentHierarchy.GetMutableREFChildSmartPointerVectorList();
            auto& l_childDataList               = l_childSmartPointerVectorList.GetMutableREFElementDataList();

            // 順位が小さい子ほど前に来るように並べ替える
            // stable_sortは、順位が同じ要素(記録にないもの同士など)の元の前後関係を変えない並べ替え
            // ラムダ式は「2つの要素を比べて、左の方を前に置くかどうか」を判定するもの
            // 判定は関数に任せて、ラムダの中では呼び出すだけにしている
            std::ranges::stable_sort(l_childDataList,
                                     [&l_rankMap](const auto& a_left, const auto& a_right)
                                     {
                                         return FetchVALGameObjectSiblingRank(l_rankMap, a_left.m_type) < FetchVALGameObjectSiblingRank(l_rankMap, a_right.m_type);
                                     });

            // 親の子リストの並べ直しが終わったため、ここで終了する
            return;
        }

        // ここから先は、親がいない場合(シーンのルートの並べ直し)
        // シーンはルートも子も区別せず、すべてのGameObjectを1つのリストで持っている
        // そのためルートだけを並べ替え、子の位置は動かさないようにする必要がある
        // 方法 : ルートが入っている場所(スロット)を覚えておき、
        //        並べ替えたルートを、同じスロットへ順番に入れ直す
        //   例 : シーンのリストが[A(ルート), B(子), C(ルート)]のとき
        //        ルートのスロットは0番と2番
        //        ルートを[C, A]の順に並べ替えたなら、0番にC、2番にAを入れて[C, B, A]になる
        auto& l_gameObjectList = a_scene.GetMutableREFGameObjectList();

        // l_rootSlotList : ルートが入っているスロット(リストの何番目か)
        // l_rootList     : 見つかったルート本体(l_rootSlotListと同じ順番で対応している)
        std::vector<std::size_t>                 l_rootSlotList = {};
        std::vector<std::shared_ptr<GameObject>> l_rootList     = {};

        for (std::size_t l_i = 0ULL; l_i < l_gameObjectList.size(); ++l_i)
        {
            const auto& l_gameObject = l_gameObjectList[l_i];

            if (!l_gameObject) { continue; }

            // 親がいる(=ルートではない)GameObjectは、並べ替えの対象にしない
            const auto& l_hierarchy = l_gameObject->GetREFHierarchy();

            if (l_hierarchy.GetREFParent().lock()) { continue; }

            // ルートが入っているスロットと、ルート本体を同時に記録する
            l_rootSlotList.emplace_back(l_i);
            l_rootList.emplace_back    (l_gameObject);
        }

        // ルートだけを、順位が小さい順に並べ替える
        // (スロットの方は並べ替えない。場所は元のまま使う)
        std::ranges::stable_sort(l_rootList,
                                 [&l_rankMap](const auto& a_left, const auto& a_right)
                                 {
                                     return FetchVALGameObjectSiblingRank(l_rankMap, a_left) < FetchVALGameObjectSiblingRank(l_rankMap, a_right);
                                 });

        // 並べ替えたルートを、覚えておいたスロットへ順番に入れ直す
        // 1番目のスロットには並べ替え後の1番目のルート、2番目のスロットには2番目のルート…となる
        // ルート以外のスロットには触らないため、子の位置は変わらない
        for (std::size_t l_i = 0ULL; l_i < l_rootList.size(); ++l_i)
        {
            l_gameObjectList[l_rootSlotList[l_i]] = l_rootList[l_i];
        }
    }
}