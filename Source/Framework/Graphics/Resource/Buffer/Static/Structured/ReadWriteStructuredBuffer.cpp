#include "ReadWriteStructuredBuffer.h"

FWK::Graphics::ReadWriteStructuredBuffer::ReadWriteStructuredBuffer() = default;
FWK::Graphics::ReadWriteStructuredBuffer::~ReadWriteStructuredBuffer()
{
	Release();
}

FWK::Graphics::ReadWriteStructuredBuffer::ReadWriteStructuredBuffer(ReadWriteStructuredBuffer && a_other) noexcept
{}

FWK::Graphics::ReadWriteStructuredBuffer& FWK::Grraphics::ReadWriteStructuredBuffer::operator=(ReadWriteStructuredBuffer && a_other) noexcept
{
}