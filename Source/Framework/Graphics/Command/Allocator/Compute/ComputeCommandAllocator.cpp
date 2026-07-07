#include "ComputeCommandAllocator.h"

FWK::Graphics::ComputeCommandAllocator::ComputeCommandAllocator() : 
	CommandAllocatorBase(Constant::k_createCommandListTypeCompute)
{}
FWK::Graphics::ComputeCommandAllocator::~ComputeCommandAllocator() = default;