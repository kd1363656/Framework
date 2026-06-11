#pragma once

namespace FWK::Graphics
{
	class ConstantBufferBase
	{
	public:

		 ConstantBufferBase() = default;
		~ConstantBufferBase() = default;

	private:

		UploadBuffer m_uploadBuffer = {};
	};
}