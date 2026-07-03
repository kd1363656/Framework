#pragma once

namespace FWK::Graphics
{
	class StaticModelStandardPerObjectDrawRequestBase : public DrawRequestPerObjectBase
	{
	public:

		 StaticModelStandardPerObjectDrawRequestBase()			= default;
		~StaticModelStandardPerObjectDrawRequestBase() override = default;

		void BeginFrame() override;

		void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) override;

		void AddDrawRequest(const std::shared_ptr<Struct::StaticModelStandardPerObjectDrawRequestData>& a_drawRequestData);

	private:

		bool DispatchModelMesh(const DirectCommandList& a_directCommandList, const Struct::StaticModelMesh& a_modelMesh) const;

		float CalculateWorldMaxScale(const TypeAlias::Math::Matrix& a_worldMatrix) const;
	
		Utility::VectorArray<std::weak_ptr<Struct::StaticModelStandardPerObjectDrawRequestData>> m_forwardDrawRequestPerObjectDataList = {};
		
		FWK_DEFINE_TYPE_INFO(StaticModelStandardPerObjectDrawRequestBase, DrawRequestPerObjectBase)
	};
}