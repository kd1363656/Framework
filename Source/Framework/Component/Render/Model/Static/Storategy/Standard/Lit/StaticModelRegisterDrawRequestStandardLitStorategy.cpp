#include "StaticModelRegisterDrawRequestStandardLitStorategy.h"

void FWK::StaticModelRegisterDrawRequestStandardLitStorategy::Execute(StaticModelComponent& a_staticModelComponent)
{
	RegisterDrawRequestData<Graphics::StaticModelStandardLitPerObjectDrawRequest>(a_staticModelComponent);
}