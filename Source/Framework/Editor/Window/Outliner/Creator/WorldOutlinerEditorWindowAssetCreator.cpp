#include "WorldOutlinerEditorWindowAssetCreator.h"
#include "../../../../../Application/Application.h"

std::shared_ptr<FWK::GameObject> FWK::Editor::WorldOutlinerEditorWindowAssetCreator::CreateEmptyGameObject(const std::weak_ptr<GameObject>& a_parent, Scene& a_scene) const
{
    auto l_gameObject = std::make_shared<GameObject>();

    // Transform / Hierarchy / ComponentContainerのOwner設定
    l_gameObject->INIT   ();
    l_gameObject->SetName(std::string{ Constant::k_gameObjectString });

    // 親がいる場合はScene登録より先に親子関係を構築する
    // Scene::AddGameObjectは親鎖から実行レベルを計算するため
    // 接続前に登録するとルート扱いになってしまう
    // ApplyParentは親側の子リスト登録とTransformModeの切替まで一括で行う
    if (const auto& l_parent = a_parent.lock())
    {
        auto& l_hierarchy = l_gameObject->GetMutableREFHierarchy();

        l_hierarchy.ApplyParent(a_parent);

        // 行列の合成方法を親の行列に依存するようにする(拡縮・回転・座標のすべてに依存)
        if (const auto& l_transformComponent = l_parent->GetVALTransformComponent().lock())
        {
            l_transformComponent->ApplyParent();
        }
    }
    else
    {
        // 行列の合成を自身のみで完結するようにする(拡縮・回転・座標のすべてに依存)
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