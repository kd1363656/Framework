#pragma once

namespace FWK::Graphics
{
	class GraphicsPipelineStateBase : public PipelineStateBase
	{
	public:

		 GraphicsPipelineStateBase()          = default;
		~GraphicsPipelineStateBase() override = default;
		
		void Deserialize(const nlohmann::json& a_rootJson) override;

		nlohmann::json Serialize() const override;

		void AddRTVFormat(const DXGI_FORMAT a_format);

		void SetRasterizerDesc  (const D3D12_RASTERIZER_DESC&	 a_set) { m_rasterizerDesc   = a_set; }
		void SetBlendDesc       (const D3D12_BLEND_DESC&		 a_set) { m_blendDesc        = a_set; }
		void SetDepthStencilDesc(const D3D12_DEPTH_STENCIL_DESC& a_set) { m_depthStencilDesc = a_set; }
		void SetSampleDesc      (const DXGI_SAMPLE_DESC&		 a_set) { m_sampleDesc       = a_set; }

		void SetPrimitiveTopologyType(const D3D12_PRIMITIVE_TOPOLOGY_TYPE a_set) { m_primitiveTopologyType = a_set; }

		void SetDSVFormat(const DXGI_FORMAT a_set) { m_dsvFormat = a_set; }
		
		void SetSampleMask(const UINT a_set) { m_sampleMask = a_set; }

		const auto& GetREFRTVFormatList() const { return m_rtvFormatList; }

		const auto& GetREFRasterizerDesc  () const { return m_rasterizerDesc; }
		const auto& GetREFBlendDesc       () const { return m_blendDesc; }
		const auto& GetREFDepthStencilDesc() const { return m_depthStencilDesc; }
		const auto& GetREFSampleDesc      () const { return m_sampleDesc; }

		auto GetVALPrimitiveTopologyType() const { return m_primitiveTopologyType; }

		auto GetVALDSVFormat() const { return m_dsvFormat; }

		auto GetVALSampleMask() const { return m_sampleMask; }

	private:

		static constexpr UINT k_initialSampleMask = UINT_MAX;

		std::vector<DXGI_FORMAT> m_rtvFormatList = {};

		Converter::GraphicsPipelineStateBaseJsonConverter m_jsonConverter = {};

		D3D12_RASTERIZER_DESC    m_rasterizerDesc   = {};
		D3D12_BLEND_DESC         m_blendDesc        = {};
		D3D12_DEPTH_STENCIL_DESC m_depthStencilDesc = {};
		DXGI_SAMPLE_DESC	     m_sampleDesc       = {};

		D3D12_PRIMITIVE_TOPOLOGY_TYPE m_primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		DXGI_FORMAT m_dsvFormat = DXGI_FORMAT_UNKNOWN;

		UINT m_sampleMask = k_initialSampleMask;

		FWK_DEFINE_TYPE_INFO(GraphicsPipelineStateBase, PipelineStateBase)
	};
}