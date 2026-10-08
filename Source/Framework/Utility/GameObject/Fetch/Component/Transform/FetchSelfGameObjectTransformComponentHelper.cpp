#include "FetchSelfGameObjectTransformComponentHelper.h"

void FWK::Utility::FetchSelfGameObjectTransformComponentHelper::PostDeserialize(const std::weak_ptr<GameObject>& a_selfGameObject)
{
    const auto& l_selfGameObject = a_selfGameObject.lock();

    if (!l_selfGameObject) { return; }

    // 自身のGameObjectが持つTransformComponentをキャッシュする
    // 呼び出し側が毎フレームGameObjectからTransformComponentを取り直さなくて済むようにするため
    // weak_ptrのまま持つので、GameObjectが破棄されても、このヘルパーがTransformComponentの寿命を延ばすことはない
    m_transformComponent = l_selfGameObject->GetVALTransformComponent();
}