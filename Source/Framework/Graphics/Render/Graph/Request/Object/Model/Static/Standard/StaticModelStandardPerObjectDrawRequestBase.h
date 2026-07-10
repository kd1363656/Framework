#pragma once

namespace FWK::Graphics
{
	class StaticModelStandardPerObjectDrawRequestBase : public DrawRequestPerObjectBase
	{
	public:

		// 静的モデル標準描画用
		struct DrawRequestData final
		{
			static constexpr float k_defaultWorldMaxScale = 0.0F;

			std::weak_ptr<Graphics::StaticModelRecord> m_staticModelRecord = {};

			TypeAlias::Math::Matrix m_worldMatrix = TypeAlias::Math::Matrix::Identity;

			TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;

			float m_worldMaxScale = k_defaultWorldMaxScale;
		};

	public:

		 StaticModelStandardPerObjectDrawRequestBase()			= default;
		~StaticModelStandardPerObjectDrawRequestBase() override = default;

		void BeginFrame() override;

		void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) override;

		void AddDrawRequest(const std::shared_ptr<DrawRequestData>& a_drawRequestData);

	private:

		bool DispatchModelMesh(const DirectCommandList& a_directCommandList, const Graphics::StaticModelRecord::ModelMesh& a_modelMesh) const;

		Utility::VectorArray<std::weak_ptr<DrawRequestData>> m_forwardDrawRequestDataList = {};
		
		static constexpr UINT k_defaultDispatchMeshThreadGroupCountY = 1U;
		static constexpr UINT k_defaultDispatchMeshThreadGroupCountZ = 1U;

		FWK_DEFINE_TYPE_INFO(StaticModelStandardPerObjectDrawRequestBase, DrawRequestPerObjectBase)
	};
}