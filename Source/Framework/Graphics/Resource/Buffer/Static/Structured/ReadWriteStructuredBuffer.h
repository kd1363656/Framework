#pragma once

namespace FWK::Grraphics
{
	class ReadWriteStructuredBuffer final
	{
	public:

		 ReadWriteStructuredBuffer();
		~ReadWriteStructuredBuffer();

		ReadWriteStructuredBuffer(const ReadWriteStructuredBuffer&) = delete;
		ReadWriteStructuredBuffer(      ReadWriteStructuredBuffer&& a_other) noexcept;

		ReadWriteStructuredBuffer& operator=(const ReadWriteStructuredBuffer&) = delete;
		ReadWriteStructuredBuffer& operator=(      ReadWriteStructuredBuffer&& a_other) noexcept;

		template <typename Type>
		bool Create();

	private:

	};
}