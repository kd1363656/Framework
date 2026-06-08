#pragma once

namespace FWK::Graphics
{
	class CopyCommandList final : public CommandListBase
	{
	public:

		 CopyCommandList();
		~CopyCommandList() override;

		void CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION& a_destination,
							   const D3D12_TEXTURE_COPY_LOCATION& a_source, 
							   const D3D12_BOX*					  a_sourceBox,
							   const UINT						  a_destinationX,
							   const UINT						  a_destinationY,
							   const UINT						  a_destinationZ) const;
	};
}