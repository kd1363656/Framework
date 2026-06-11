#pragma once

namespace FWK::Graphics
{
	template <typename ConstantBufferType>
	class ConstantBufferUploader : public ConstantBufferUploaderBase
	{
	public:

		ConstantBufferUploader() : 
			ConstantBufferUploaderBase(sizeof(ConstantBufferType))
		{}
		~ConstantBufferUploader() override = default;

		ConstantBufferUploader(const ConstantBufferUploader&)  = delete;
		ConstantBufferUploader(		 ConstantBufferUploader&&) = delete;

		ConstantBufferUploader& operator=(const ConstantBufferUploader&)  = delete;
		ConstantBufferUploader& operator=(	    ConstantBufferUploader&&) = delete;

		D3D12_GPU_VIRTUAL_ADDRESS WritePerObject(const ConstantBufferType& a_constantBuffer)
		{
			return ConstantBufferUploaderBase::WritePerObject(a_constantBuffer);
		}

		D3D12_GPU_VIRTUAL_ADDRESS WriteCommonPass(const ConstantBufferType& a_constantBuffer)
		{
			return ConstantBufferUploaderBase::WriteCommonPass(a_constantBuffer);
		}
	};
}