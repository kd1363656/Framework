#pragma once

namespace FWK::Graphics
{
	class StaticModelStandardPerObjectDrawRequest : public DrawRequestPerObjectBase
	{
	public:

		 StaticModelStandardPerObjectDrawRequest()			= default;
		~StaticModelStandardPerObjectDrawRequest() override = default;

		void BeginFrame() override;

		void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource& a_frameResource) override;

		void AddDrawRequest(const std::shared_ptr<Struct::StaticModelStandardPerObjectDrawRequestData>& a_drawRequestData);

	protected:

		bool DispatchModelMesh(const DirectCommandList& a_directCommandList, const Struct::StaticModelMesh& a_modelMesh) const;

		const auto& GetREFForwardDrawRequestPerObjectDataList () const { return m_forwardDrawRequestPerObjectDataList; }
		
	private:

		DrawRequestPerObjectList<Struct::StaticModelStandardPerObjectDrawRequestData> m_forwardDrawRequestPerObjectDataList  = {};
		
		FWK_DEFINE_TYPE_INFO(StaticModelStandardPerObjectDrawRequest, DrawRequestPerObjectBase)
	};
}