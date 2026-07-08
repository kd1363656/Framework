#include "StructuredBufferBase.h"

FWK::Graphics::StructuredBufferBase::StructuredBufferBase(StructuredBufferBase&&) noexcept
{}

FWK::Graphics::StructuredBufferBase& FWK::Graphics::StructuredBufferBase::operator=(StructuredBufferBase&&) noexcept
{
	// TODO: return ステートメントをここに挿入します
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::StructuredBufferBase::CreateStructuredBufferSRV(const Device& a_device, const Struct::GPUResource& a_bufferGPUResource, const UINT a_elementCount, const UINT a_structuredByteStride)
{
	return TypeAlias::DescriptorIndex();
}