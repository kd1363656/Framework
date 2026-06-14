#pragma once

namespace FWK::Graphics
{
	template <typename ConstantBufferType>
	class DrawRequestPassBase : public CachedPassConstantBufferDrawRequestBase<ConstantBufferType>
	{
	public:

				 DrawRequestPassBase() = default;
		virtual ~DrawRequestPassBase() = default;

	private:

		std::weak_ptr<RenderTargetPassTexture> m_renderTargetPassTexture = {};
	};
}