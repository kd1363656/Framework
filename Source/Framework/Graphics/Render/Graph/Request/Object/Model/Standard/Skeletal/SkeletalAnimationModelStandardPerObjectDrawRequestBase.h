#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelStandardPerObjectDrawRequestBase : public DrawRequestPerObjectBase
	{
	public:

		struct DrawRequestData final
		{
			std::weak_ptr<SkeletalAnimationPlayer> m_skeletalAnimationPlayer = {};

			TypeAlias::Math::Matrix m_worldMatrix                 = TypeAlias::Math::Matrix::Identity;
			TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;
		};

	public:

		 SkeletalAnimationModelStandardPerObjectDrawRequestBase()          = default;
		~SkeletalAnimationModelStandardPerObjectDrawRequestBase() override = default;

		void BeginFrame() override;

		void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) override;

		void AddDrawRequest(const std::shared_ptr<DrawRequestData>& a_drawRequestData);

	private:

		bool DispatchModelMesh(const DirectCommandList& a_directCommandList, const SkeletalAnimationModelRecord::ModelMesh& a_modelMesh) const;

		Utility::VectorArray<std::weak_ptr<DrawRequestData>> m_forwardDrawRequestDataList = {};

		static constexpr UINT k_defaultDispatchMeshThreadGroupCountY = 1U;
		static constexpr UINT k_defaultDispatchMeshThreadGroupCountZ = 1U;

		FWK_DEFINE_TYPE_INFO(SkeletalAnimationModelStandardPerObjectDrawRequestBase, DrawRequestPerObjectBase)
	};
}