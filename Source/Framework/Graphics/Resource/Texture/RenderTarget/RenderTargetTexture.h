#pragma once

namespace FWK::Graphics
{
	class RenderTargetTexture final
	{
	public:

		 RenderTargetTexture() = default;
		~RenderTargetTexture() = default;

		RenderTargetTexture(const RenderTargetTexture&)			  = delete;
		RenderTargetTexture(	  RenderTargetTexture&&) noexcept = default;
		
		RenderTargetTexture& operator=(const RenderTargetTexture&)			 = delete;
		RenderTargetTexture& operator=(		 RenderTargetTexture&&) noexcept = default;


	};
}