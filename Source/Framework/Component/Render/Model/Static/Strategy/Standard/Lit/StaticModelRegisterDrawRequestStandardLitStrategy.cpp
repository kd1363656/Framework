#include "StaticModelRegisterDrawRequestStandardLitStrategy.h"

void FWK::StaticModelRegisterDrawRequestStandardLitStrategy::Execute(StaticModelComponent& a_staticModelComponent)
{
	RegisterDrawRequestData<Graphics::StaticModelStandardLitPerObjectDrawRequest>(a_staticModelComponent);
}