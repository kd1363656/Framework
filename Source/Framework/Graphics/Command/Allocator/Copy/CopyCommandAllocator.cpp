#include "CopyCommandAllocator.h"

FWK::Graphics::CopyCommandAllocator::CopyCommandAllocator() : 
	CommandAllocatorBase(Constant::k_createCommandListTypeCopy)
{}
FWK::Graphics::CopyCommandAllocator::~CopyCommandAllocator() = default;