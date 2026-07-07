#include "CopyCommandQueue.h"

FWK::Graphics::CopyCommandQueue::CopyCommandQueue() : 
	CommandQueueBase(Constant::k_createCommandListTypeCopy)
{}
FWK::Graphics::CopyCommandQueue::~CopyCommandQueue() = default;