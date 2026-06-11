#pragma once

namespace FWK::Graphics
{
	template <typename ConstantBufferType>
	class ConstantBufferUplaoder : public ConstantBufferUploaderBase
	{
	public:

		ConstantBufferUplaoder() : 
			ConstantBufferUploaderBase(sizeof(ConstantBufferType))
		{}
		~ConstantBufferUplaoder() override = default;

		ConstantBufferUplaoder(const ConstantBufferUplaoder&)  = delete;
		ConstantBufferUplaoder(		 ConstantBufferUplaoder&&) = delete;

		ConstantBufferUplaoder& operator=(const ConstantBufferUplaoder&)  = delete;
		ConstantBufferUplaoder& operator=(	    ConstantBufferUplaoder&&) = delete;

		D3D12_GPU_VIRTUAL_ADDRESS WritePerObject(const ConstantBufferType& a_constantBuffer)
		{
			return WritePerObject(a_constantBuffer);
		}

		D3D12_GPU_VIRTUAL_ADDRESS WriteCommonPasss(const ConstantBufferType& a_constantBuffer)
		{
			return WriteCommonPass(a_constantBuffer);
		}
	};
}