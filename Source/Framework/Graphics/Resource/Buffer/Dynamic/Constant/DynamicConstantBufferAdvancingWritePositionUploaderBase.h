#pragma once

namespace FWK::Graphics
{
	template <typename ConstantBufferType>
	class DynamicConstantBufferAdvancingWritePositionUploaderBase : public DynamicConstantBufferUploaderBase<ConstantBufferType>
	{
	public:

		 DynamicConstantBufferAdvancingWritePositionUploaderBase() : 
			 DynamicConstantBufferUploaderBase<ConstantBufferType>(DynamicBufferUploaderBase::k_advanceWritePosition)
		 {}
		~DynamicConstantBufferAdvancingWritePositionUploaderBase() override = default;
	};
}