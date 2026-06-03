#include "DirectCommandAllocator.h"

FWK::Graphics::DirectCommandAllocator::DirectCommandAllocator() : 
	CommandAllocatorBase(Constant::k_createCommandListTypeDirect)
{}
FWK::Graphics::DirectCommandAllocator::~DirectCommandAllocator() = default;