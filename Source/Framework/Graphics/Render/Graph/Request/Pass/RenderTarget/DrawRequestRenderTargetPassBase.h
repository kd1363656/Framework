#pragma once

namespace FWK::Graphics
{
	template <typename ConstantBufferType, Enum::RenderGraphResourceType ResourceType>
	class DrawRequestRenderTargetPassBase : public DrawRequestPassBase
	{
	public:

		 DrawRequestRenderTargetPassBase()          = default;
		~DrawRequestRenderTargetPassBase() override = default;

		std::weak_ptr<RenderTargetPassTexture> FetchVALRenderTargetPassTexture(const FrameResource& a_frameResource) const
		{
			const auto& l_renderGraphFrameResource = a_frameResource.GetREFRenderGraphFrameResource           ();
			const auto& l_renderTargetPassTexture  = l_renderGraphFrameResource.FindVALRenderTargetPassTexture(ResourceType);

			FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_renderTargetPassTexture.expired(), "レンダーグラフフレームリソースのレンダーターゲットパステクスチャが無効になっており、レンダーターゲットパステクスチャの取得に失敗しました。", {});

			return l_renderTargetPassTexture;
		}

	protected:

		auto& GetMutableREFConstantBuffer() { return m_constantBuffer; }

	private:

		ConstantBufferType m_constantBuffer = {};
	};
}