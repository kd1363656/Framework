#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelStandardPerObjectDrawRequestBase : public DrawRequestPerObjectBase
	{
	public:

		 SkeletalAnimationModelStandardPerObjectDrawRequestBase()          = default;
		~SkeletalAnimationModelStandardPerObjectDrawRequestBase() override = default;

		void BeginFrame() override;

		void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) override;

		void AddDrawRequest(const std::shared_ptr<Struct::SkeletalAnimationModelStandardPerObjectDrawRequestData>& a_drawRequestData);

	private:

		bool DispatchModelMesh(const DirectCommandList& a_directCommandList, const SkeletalAnimationModelRecord::ModelMesh& a_modelMesh) const;

		Utility::SmartPointerVectorArray<std::weak_ptr<Struct::SkeletalAnimationModelStandardPerObjectDrawRequestData>> m_forwardDrawRequestDataSmartPointerVectorArray = {};

		static constexpr UINT k_defaultDispatchMeshThreadGroupCountY = 1U;
		static constexpr UINT k_defaultDispatchMeshThreadGroupCountZ = 1U;

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationModelStandardPerObjectDrawRequestBase, DrawRequestPerObjectBase)
	};
}