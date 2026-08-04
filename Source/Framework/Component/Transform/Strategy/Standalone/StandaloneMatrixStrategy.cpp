#include "StandaloneMatrixStrategy.h"

void FWK::StandaloneMatrixStrategy::Execute(TransformComponent& a_transformComponent)
{
	auto l_resultMatrix = a_transformComponent.CalculateScaleMatrix() * a_transformComponent.CalculateRotationMatrix()* a_transformComponent.CalculateTranslationMatrix();

	a_transformComponent.SetMatrix(std::move(l_resultMatrix));
}