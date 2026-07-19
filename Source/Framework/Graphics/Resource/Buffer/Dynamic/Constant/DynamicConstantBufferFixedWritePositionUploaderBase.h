#pragma once

namespace FWK::Graphics
{
	template <typename ConstantBufferType>
	class DynamicConstantBufferFixedWritePositionUploaderBase : public DynamicConstantBufferUploaderBase<ConstantBufferType>
	{
	public:

		 DynamicConstantBufferFixedWritePositionUploaderBase() : 
			 DynamicConstantBufferUploaderBase<ConstantBufferType>(k_keepWritePosition)
		 {}
		~DynamicConstantBufferFixedWritePositionUploaderBase() override = default;
	};
}