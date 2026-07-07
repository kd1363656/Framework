#include "StandaloneMatrixStrategy.h"

void FWK::StandaloneMatrixStrategy::Execute(TransformComponent& a_owner)
{
	auto l_resultMatrix = a_owner.CalcScaleMatrix() * a_owner.CalcRotationMatrix() * a_owner.CalcTranslationMatrix();

	a_owner.SetMatrix(std::move(l_resultMatrix));
}