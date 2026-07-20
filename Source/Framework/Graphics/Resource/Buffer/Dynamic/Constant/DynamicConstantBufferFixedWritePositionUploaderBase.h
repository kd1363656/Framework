#pragma once

namespace FWK::Graphics
{
	template <typename ConstantBufferType>
	class DynamicConstantBufferFixedWritePositionUploaderBase : public DynamicConstantBufferUploaderBase<ConstantBufferType>
	{
	public:

		 DynamicConstantBufferFixedWritePositionUploaderBase() : 
			 DynamicConstantBufferUploaderBase<ConstantBufferType>(DynamicBufferUploaderBase::k_keepWritePosition)
		 {}
		~DynamicConstantBufferFixedWritePositionUploaderBase() override = default;
	};
}