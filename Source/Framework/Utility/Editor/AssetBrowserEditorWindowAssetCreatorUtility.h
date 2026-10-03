#pragma once

namespace FWK::Utility
{
    inline bool HasAncestorInList(const std::vector<std::weak_ptr<GameObject>>& a_gameObjectList, const std::weak_ptr<GameObject>& a_gameObject) 
    {
        const auto& l_gameObject = a_gameObject.lock();
 
        if (!l_gameObject) { return false; }
 
        // 親を辿って選択リスト内に祖先がいるか判定する
        auto l_current = l_gameObject->GetREFHierarchy().GetREFParent().lock();
 
        while (l_current)
        {
            // 選択リスト内に現在の祖先と同じアドレスを持つ要素があるか
            if (std::ranges::any_of(a_gameObjectList,
                                   [&l_current](const auto& a_gameObjectWeak)
                                   {
                                       return a_gameObjectWeak.lock() == l_current;
                                   }))
            {
                return true; 
            }
 
            // さらに上の親へ
            const auto& l_currentHierarchy = l_current->GetREFHierarchy();

            l_current = l_currentHierarchy.GetREFParent().lock();
        }
 
        return false;
    }

}