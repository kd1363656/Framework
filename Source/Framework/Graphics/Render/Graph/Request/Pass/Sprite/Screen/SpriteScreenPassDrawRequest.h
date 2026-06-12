#pragma once

namespace FWK::Graphics
{
	class SpriteScreenPassDrawRequest final : public CachedPassConstantBufferDrawRequestBase<Struct::CBSpritePass>
	{
	public:

		 SpriteScreenPassDrawRequest()		    = default;
		~SpriteScreenPassDrawRequest() override = default;

		bool SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource) override;

		FWK_DEFINE_TYPE_INFO(SpriteScreenPassDrawRequest, DrawRequestPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPassSharedFactory, FWK::Graphics::SpriteScreenPassDrawRequest)