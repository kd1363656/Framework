#include "HierarchicalMatrixStrartegy.h"

void FWK::HierarchicalMatrixStrartegy::Execute(TransformComponent& a_owner)
{
	auto l_parent = a_owner.GetREFParentTransformComponent().lock();

	if (!l_parent) { return; }

	// 親の行列が先に更新されていることを前提にしている処理です。
	auto l_resultMatrix = a_owner.CalculateScaleMatrix() * a_owner.CalculateTranslationMatrix() * a_owner.CalculateRotationMatrix();

	a_owner.SetMatrix(std::move(l_resultMatrix));
}