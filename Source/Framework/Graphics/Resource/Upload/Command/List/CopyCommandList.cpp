#include "CopyCommandList.h"

FWK::Graphics::CopyCommandList::CopyCommandList() : 
	CommandListBase(Constant::k_createCommandListTypeCopy)
{}
FWK::Graphics::CopyCommandList::~CopyCommandList() = default;