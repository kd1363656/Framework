#include "ComputeCommandQueue.h"

FWK::Graphics::ComputeCommandQueue::ComputeCommandQueue() : 
	CommandQueueBase(Constant::k_createCommandListTypeCompute)
{}
FWK::Graphics::ComputeCommandQueue::~ComputeCommandQueue() = default;