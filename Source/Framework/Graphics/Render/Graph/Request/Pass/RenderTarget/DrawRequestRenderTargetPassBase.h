#pragma once

namespace FWK::Graphics
{
	template <typename ConstantBufferType, Enum::RenderGraphResourceType ResourceType>
	class DrawRequestRenderTargetPassBase : public DrawRequestPassBase
	{
	public:

		 DrawRequestRenderTargetPassBase()          = default;
		~DrawRequestRenderTargetPassBase() override = default;

	protected:

		bool EnsureCurrentRenderTargetPassTexture(const FrameResource& a_frameResource)
		{
			// レンダーターゲットパステクスチャが無効でない場合セットする必要がないので
			// セットが完了しているというtrueを返す
			if (!m_renderTargetPassTexture.expired()) { return true; }

			const auto& l_renderGraphFrameResource = a_frameResource.GetREFRenderGraphFrameResource			  ();
			const auto& l_renderTargetPassTexture  = l_renderGraphFrameResource.FindVALRenderTargetPassTexture(ResourceType);

			// 取得したいレンダーターゲットパステクスチャが無効ならreturn
			if (l_renderTargetPassTexture.expired()) { return false; }

			m_renderTargetPassTexture = l_renderTargetPassTexture;

			return true;
		}

		const auto& GetREFRenderTargetPasTexture() const { return m_renderTargetPassTexture; }

		auto& GetMutableREFConstantBuffer() { return m_constantBuffer; }

	private:

		std::weak_ptr<RenderTargetPassTexture> m_renderTargetPassTexture = {};

		ConstantBufferType m_constantBuffer = {};
	};
}