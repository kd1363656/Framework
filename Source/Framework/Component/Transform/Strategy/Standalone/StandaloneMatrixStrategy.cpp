#include "StandaloneMatrixStrategy.h"

void FWK::StandaloneMatrixStrategy::Execute(TransformComponent& a_owner)
{
	auto l_resultMatrix = a_owner.CalculateScaleMatrix()* a_owner.CalculateRotationMatrix()* a_owner.CalculateTranslationMatrix();

	a_owner.SetMatrix(std::move(l_resultMatrix));
}