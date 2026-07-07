#include "ComputeCommandList.h"

void FWK::Graphics::ComputeCommandList::Reset(const TypeAlias::ComputeCommandAllocator& a_computeCommandAllocator)
{
	DirectAndComputeCommandListBase::Reset(a_computeCommandAllocator);
}