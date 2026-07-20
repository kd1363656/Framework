#pragma once

namespace FWK::Graphics
{
	template <typename VertexType>
	class DynamicVertexBufferAdvancingWritePositionUploaderBase : public DynamicVertexBufferUploaderBase<VertexType>
	{
	public:

		 DynamicVertexBufferAdvancingWritePositionUploaderBase() : 
			 DynamicVertexBufferUploaderBase<VertexType>(DynamicBufferUploaderBase::k_advanceWritePosition)
		 {}
		~DynamicVertexBufferAdvancingWritePositionUploaderBase() override = default;
	};
}