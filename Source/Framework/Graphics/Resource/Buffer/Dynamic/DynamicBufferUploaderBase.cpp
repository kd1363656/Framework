#include "DynamicBufferUploaderBase.h"

FWK::Graphics::DynamicBufferUploaderBase::DynamicBufferUploaderBase(const UINT64& a_typeSize) : 
	k_typeSize(a_typeSize),

	m_currentOffset(k_initialOffset)
{}
FWK::Graphics::DynamicBufferUploaderBase::~DynamicBufferUploaderBase() = default;