#include "HierarchicalMatrixStrartegy.h"

void FWK::HierarchicalMatrixStrartegy::Execute(TransformComponent& a_transformComponent)
{
	auto l_parent = a_transformComponent.GetREFParentTransformComponent().lock();

	if (!l_parent) { return; }

	const auto& l_parentWorldPosition = l_parent->GetREFMatrix().Translation();

	// 親の行列が先に更新されていることを前提にしている処理です。
	auto l_resultMatrix = a_transformComponent.CalculateScaleMatrix()       * 
		                  a_transformComponent.CalculateTranslationMatrix() * 
		                  a_transformComponent.CalculateRotationMatrix()    * 
		                  TypeAlias::Math::Matrix::CreateTranslation(l_parentWorldPosition);

	a_transformComponent.SetMatrix(std::move(l_resultMatrix));
}