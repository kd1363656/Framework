#pragma once

namespace FWK::Graphics
{
	template <typename VertexType>
	class DynamicVertexBufferAdvancingWritePositionUploaderBase : public DynamicVertexBufferUploaderBase<VertexType>
	{
	public:

		 DynamicVertexBufferAdvancingWritePositionUploaderBase() : 
			 DynamicVertexBufferUploaderBase<VertexType>(k_advanceWritePosition)
		 {}
		~DynamicVertexBufferAdvancingWritePositionUploaderBase() override = default;
	};
}