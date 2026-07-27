#pragma once

namespace FWK::Graphics
{
	class ModelShadowPerObjectDrawRequest : public DrawRequestPerObjectBase
	{
	public:

		 ModelShadowPerObjectDrawRequest()          = default;
		~ModelShadowPerObjectDrawRequest() override = default;

		void BeginFrame() override;

		void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) override;

		void AddDrawRequest(const std::shared_ptr<Struct::StaticModelStandardPerObjectDrawRequestData>&            a_drawRequestData);
		void AddDrawRequest(const std::shared_ptr<Struct::SkeletalAnimationModelStandardPerObjectDrawRequestData>& a_drawRequestData);

	private:

		Utility::SmartPointerVectorArray<std::weak_ptr<Struct::StaticModelStandardPerObjectDrawRequestData>>            m_staticModelStandardPerObjectDrawRequestDataSmartPointerVectorArray            = {};
		Utility::SmartPointerVectorArray<std::weak_ptr<Struct::SkeletalAnimationModelStandardPerObjectDrawRequestData>> m_skeletalAnimationModelStandardPerObjectDrawRequestDataSmartPointerVectorArray = {};

		FWK_DEFINE_TYPE_INFO(ModelShadowPerObjectDrawRequest, RenderGraphPassBase)
	};
}