#pragma once

namespace FWK::Graphics
{
	class StaticModelPerObjectDrawRequestBase : public DrawRequestPerObjectBase
	{
	public:

		 StaticModelPerObjectDrawRequestBase()			= default;
		~StaticModelPerObjectDrawRequestBase() override = default;

		void BeginFrame() override;

		void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) override;

		void AddDrawRequest(const std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData>& a_drawRequestData);

	private:

		bool DispatchModelMesh(const DirectCommandList& a_directCommandList, const Graphics::StaticModelRecord::ModelMesh& a_modelMesh) const;

		Utility::SmartPointerVectorArray<std::weak_ptr<Struct::StaticModelPerObjectDrawRequestData>> m_forwardDrawRequestDataSmartPointerVectorArray = {};
		
		static constexpr UINT k_defaultDispatchMeshThreadGroupCountY = 1U;
		static constexpr UINT k_defaultDispatchMeshThreadGroupCountZ = 1U;

		FWK_DEFINE_TYPE_INFO(StaticModelPerObjectDrawRequestBase, DrawRequestPerObjectBase)
	};
}