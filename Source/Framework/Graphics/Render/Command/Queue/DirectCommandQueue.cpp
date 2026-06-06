#include "DirectCommandQueue.h"

FWK::Graphics::DirectCommandQueue::DirectCommandQueue() : 
	CommandQueueBase(Constant::k_createCommandListTypeDirect)
{}
FWK::Graphics::DirectCommandQueue::~DirectCommandQueue() = default;