#include "ReadWriteStructuredBuffer.h"

FWK::Grraphics::ReadWriteStructuredBuffer::ReadWriteStructuredBuffer() = default;
FWK::Grraphics::ReadWriteStructuredBuffer::~ReadWriteStructuredBuffer()
{
	Release();
}

FWK::Grraphics::ReadWriteStructuredBuffer::ReadWriteStructuredBuffer(ReadWriteStructuredBuffer && a_other) noexcept
{}

FWK::Grraphics::ReadWriteStructuredBuffer& FWK::Grraphics::ReadWriteStructuredBuffer::operator=(ReadWriteStructuredBuffer && a_other) noexcept
{
}