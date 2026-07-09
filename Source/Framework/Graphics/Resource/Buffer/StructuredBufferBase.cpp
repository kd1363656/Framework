//#include "StructuredBufferBase.h"
//
//FWK::Graphics::StructuredBufferBase::StructuredBufferBase(StructuredBufferBase&& a_other) noexcept
//{
//	MoveFrom(std::move(a_other));
//}
//
//FWK::Graphics::StructuredBufferBase& FWK::Graphics::StructuredBufferBase::operator=(StructuredBufferBase&& a_other) noexcept
//{
//	if (this == &a_other) { return *this; }
//
//	MoveFrom(std::move(a_other));
//
//	return *this;
//}
//
//void FWK::Graphics::StructuredBufferBase::ReleaseImmediatelySRVDescriptorIndex(TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
//{
//
//}
//
