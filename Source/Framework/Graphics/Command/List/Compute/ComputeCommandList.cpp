#include "ComputeCommandList.h"

FWK::Graphics::ComputeCommandList::ComputeCommandList() : 
	CommandListBase(Constant::k_createCommandListTypeCompute)
{}
FWK::Graphics::ComputeCommandList::~ComputeCommandList() = default;