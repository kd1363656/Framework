#pragma once

namespace FWK::Graphics
{
	class DepthStencilTexture final
	{
	public:
	
		 DepthStencilTexture() = default;
		~DepthStencilTexture() = default;
	
		DepthStencilTexture(const DepthStencilTexture&)			  = delete;
		DepthStencilTexture(	  DepthStencilTexture&&) noexcept = default;
		
		DepthStencilTexture& operator=(const DepthStencilTexture&)			 = delete;
		DepthStencilTexture& operator=(		 DepthStencilTexture&&) noexcept = default;

	};
}