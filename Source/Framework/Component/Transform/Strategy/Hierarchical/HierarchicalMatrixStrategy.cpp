#include "HierarchicalMatrixStrategy.h"

void FWK::HierarchicalMatrixStrategy::Execute(TransformComponent& a_transformComponent)
{
    const auto& l_calculateParentWorldMatrixEnumBitShift = a_transformComponent.GetREFCalculateParentWorldMatrixEnumBitShift();

    if (!l_calculateParentWorldMatrixEnumBitShift) { return; }

    const auto& l_parentGameObject = a_transformComponent.GetREFParentGameObject().lock();

    if (!l_parentGameObject) { return; }

    const auto& l_parent = l_parentGameObject->GetVALTransformComponent().lock();

    // 親の行列が先に更新されていることを前提にしている処理です。
    auto l_resultMatrix = a_transformComponent.CalculateScaleMatrix()       *
                          a_transformComponent.CalculateTranslationMatrix() *
                          a_transformComponent.CalculateRotationMatrix()    *
                          l_parent->CalculateWorldMatrixData(*l_calculateParentWorldMatrixEnumBitShift);

    a_transformComponent.SetMatrix(std::move(l_resultMatrix));
}