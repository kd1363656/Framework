#pragma once

namespace FWK::Graphics
{
	class CopyCommandList final : public CommandListBase<D3D12_COMMAND_LIST_TYPE_COPY>
	{
	public:

		 CopyCommandList()          = default;
		~CopyCommandList() override = default;

		void Reset(const TypeAlias::CopyCommandAllocator& a_copyCommandAllocator);

		void CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION& a_destination,
							   const D3D12_TEXTURE_COPY_LOCATION& a_source, 
							   const D3D12_BOX*					  a_sourceBox,
							   const UINT						  a_destinationX,
							   const UINT						  a_destinationY,
							   const UINT						  a_destinationZ) const;

		void CopyBufferRegion(const UINT64&	         a_destinationOffset,
							  const UINT64&	         a_sourceOffset,
							  const UINT64&	         a_bufferSize,
							        ID3D12Resource2& a_destinationBuffer,
							        ID3D12Resource2& a_sourceBuffer) const;
	};
}