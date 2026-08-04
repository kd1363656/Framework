#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelCascadeShadowPerObjectDrawRequest : public DrawRequestPerObjectBase
	{
	public:

		 SkeletalAnimationModelCascadeShadowPerObjectDrawRequest()          = default;
		~SkeletalAnimationModelCascadeShadowPerObjectDrawRequest() override = default;

		void BeginFrame() override;

		void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) override;

		void AddDrawRequest(const std::shared_ptr<Struct::SkeletalAnimationModelPerObjectDrawRequestData>& a_drawRequestData);

	private:

		SkeletalAnimationModelPerObjectDrawRequestBase m_skeletalAnimationModelPerObjectDrawRequest = {};

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationModelCascadeShadowPerObjectDrawRequest, DrawRequestPerObjectBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::SkeletalAnimationModelCascadeShadowPerObjectDrawRequest)