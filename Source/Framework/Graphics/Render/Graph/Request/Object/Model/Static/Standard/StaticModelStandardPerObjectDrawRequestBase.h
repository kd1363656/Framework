#pragma once

namespace FWK::Graphics
{
	class StaticModelStandardPerObjectDrawRequestBase : public DrawRequestPerObjectBase
	{
	public:

		 StaticModelStandardPerObjectDrawRequestBase()			= default;
		~StaticModelStandardPerObjectDrawRequestBase() override = default;

		void BeginFrame() override;

		virtual void RequestForwardDraw (const TextureSystem& a_textureSystem, Renderer& a_renderer) = 0;
		
		void AddDrawRequest(const std::shared_ptr<Struct::StaticModelStandardPerObjectDrawRequestData>& a_drawRequestData);

	protected:

		void SetupModelMeshConstantBuffer(const RootSignature&																   a_rootSignature,
										  const DirectCommandList&															   a_directCommandList,
										  const FrameResource&																   a_frameResource,
										  const DrawRequestPerObjectList<Struct::StaticModelStandardPerObjectDrawRequestData>& a_drawRequestDataList,
										  const TextureSystem&																   a_textureSystem);

		bool DispatchModelMesh(const DirectCommandList& a_directCommandList, const Struct::StaticModelMesh& a_modelMesh) const;

		const auto& GetREFForwardDrawRequestPerObjectDataList () const { return m_forwardDrawRequestPerObjectDataList; }
		
	private:

		DrawRequestPerObjectList<Struct::StaticModelStandardPerObjectDrawRequestData> m_forwardDrawRequestPerObjectDataList  = {};
		
		FWK_DEFINE_TYPE_INFO(StaticModelStandardPerObjectDrawRequestBase, DrawRequestPerObjectBase)
	};
}