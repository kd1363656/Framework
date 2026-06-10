#pragma once

namespace FWK::Graphics
{
	class PipelineState;
}

namespace FWK::Converter
{
	class PipelineStateJsonConverter final
	{
	public:
		
		 PipelineStateJsonConverter() = default;
		~PipelineStateJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::PipelineState& a_pipelineState) const;

		nlohmann::json Serialize(const Graphics::PipelineState& a_pipelineState) const;

	private:

		void DeserializeOptionalShader(const nlohmann::json& a_rootJson, std::shared_ptr<Graphics::Shader>& a_shader) const;

		void DeserializeRasterizerDesc  (const nlohmann::json& a_rootJson, Graphics::PipelineState& a_pipelineState) const;
		void DeserializeBlendDesc       (const nlohmann::json& a_rootJson, Graphics::PipelineState& a_pipelineState) const;
		void DeserializeDepthStencilDesc(const nlohmann::json& a_rootJson, Graphics::PipelineState& a_pipelineState) const;
		void DeserializeRTVFormatList   (const nlohmann::json& a_rootJson, Graphics::PipelineState& a_pipelineState) const;
		void DeserializeSampleDesc      (const nlohmann::json& a_rootJson, Graphics::PipelineState& a_pipelineState) const;

		nlohmann::json SerializeRasterizerDesc    (const Graphics::PipelineState   & a_pipelineState)      const;
		nlohmann::json SerializeBlendDesc         (const Graphics::PipelineState   & a_pipelineState)      const;
		nlohmann::json SerializeDepthStencilDesc  (const Graphics::PipelineState   & a_pipelineState)      const;
		nlohmann::json SerializeDepthStencilOPDesc(const D3D12_DEPTH_STENCILOP_DESC& a_depthStencilOPDesc) const;
		nlohmann::json SerializeRTVFormatList     (const Graphics::PipelineState   & a_pipelineState)      const;
		nlohmann::json SerializeSampleDesc        (const Graphics::PipelineState   & a_pipelineState)      const;

		void DeserializeDepthStencilOpDesc(const nlohmann::json& a_rootJson, D3D12_DEPTH_STENCILOP_DESC& a_depthStencilOPDesc) const;

		void EnsureShader(std::shared_ptr<Graphics::Shader>& a_shader) const;

		// PipelineState
		static constexpr std::string_view k_amplificationShaderJsonKey   = "AmplificationShader";
		static constexpr std::string_view k_meshShaderJsonKey            = "MeshShader";
		static constexpr std::string_view k_pixelShaderJsonKey           = "PixelShader";
		static constexpr std::string_view k_rasterizerDescJsonKey        = "RasterizerDesc";
		static constexpr std::string_view k_blendDescJsonKey             = "BlendDesc";
		static constexpr std::string_view k_depthStencilDescJsonKey      = "DepthStencilDesc";
		static constexpr std::string_view k_rtvFormatListJsonKey         = "RTVFormatList";
		static constexpr std::string_view k_dsvFormatJsonKey             = "DSVFormat";
		static constexpr std::string_view k_sampleDescJsonKey            = "SampleDesc";
		static constexpr std::string_view k_useRootSignatureTypeJsonKey  = "UseRootSignatureType";
		static constexpr std::string_view k_primitiveTopologyTypeJsonKey = "PrimitiveTopologyType";
		static constexpr std::string_view k_sampleMaskJsonKey            = "SampleMask";
		
		// RasterizerDesc
		static constexpr std::string_view k_fillModeJsonKey              = "FillMode";
		static constexpr std::string_view k_cullModeJsonKey              = "CullMode";
		static constexpr std::string_view k_frontCounterClockwiseJsonKey = "FrontCounterClockwise";
		static constexpr std::string_view k_depthBiasJsonKey			 = "DepthBias";
		static constexpr std::string_view k_depthBiasClampJsonKey	     = "DepthBiasClamp";
		static constexpr std::string_view k_slopeScaledDepthBiasJsonKey  = "SlopeScaledDepthBias";
		static constexpr std::string_view k_depthClipEnableJsonKey       = "DepthClipEnable";
		static constexpr std::string_view k_multisampleEnableJsonKey     = "MultisampleEnable";
		static constexpr std::string_view k_antialiasedLineEnableJsonKey = "AntialiasedLineEnable";
		static constexpr std::string_view k_forcedSampleCountJsonKey     = "ForcedSampleCount";
		static constexpr std::string_view k_conservativeRasterJsonKey    = "ConservativeRaster";

		// BlendDesc
		static constexpr std::string_view k_alphaToCoverageEnableJsonKey  = "AlphaToCoverageEnable";
		static constexpr std::string_view k_independentBlendEnableJsonKey = "IndependentBlendEnable";
		static constexpr std::string_view k_renderTargetJsonKey           = "RenderTarget";

		// RenderTargetBlendDesc
		static constexpr std::string_view k_blendEnableJsonKey           = "BlendEnable";
		static constexpr std::string_view k_logicOPEnableJsonKey         = "LogicOpEnable";
		static constexpr std::string_view k_srcBlendJsonKey              = "SrcBlend";
		static constexpr std::string_view k_destBlendJsonKey             = "DestBlend";
		static constexpr std::string_view k_blendOPJsonKey               = "BlendOp";
		static constexpr std::string_view k_srcBlendAlphaJsonKey         = "SrcBlendAlpha";
		static constexpr std::string_view k_destBlendAlphaJsonKey        = "DestBlendAlpha";
		static constexpr std::string_view k_blendOPAlphaJsonKey          = "BlendOpAlpha";
		static constexpr std::string_view k_logicOPJsonKey               = "LogicOp";
		static constexpr std::string_view k_renderTargetWriteMaskJsonKey = "RenderTargetWriteMask";

		// DepthStencilDesc
		static constexpr std::string_view k_depthEnableJsonKey      = "DepthEnable";
		static constexpr std::string_view k_depthWriteMaskJsonKey   = "DepthWriteMask";
		static constexpr std::string_view k_depthFUNCJsonKey        = "DepthFunc";
		static constexpr std::string_view k_stencilEnableJsonKey    = "StencilEnable";
		static constexpr std::string_view k_stencilReadMaskJsonKey  = "StencilReadMask";
		static constexpr std::string_view k_stencilWriteMaskJsonKey = "StencilWriteMask";
		static constexpr std::string_view k_frontFaceJsonKey        = "FrontFace";
		static constexpr std::string_view k_backFaceJsonKey         = "BackFace";

		// DepthStencilOpDesc
		static constexpr std::string_view k_stencilFailOpJsonKey      = "StencilFailOp";
		static constexpr std::string_view k_stencilDepthFailOpJsonKey = "StencilDepthFailOp";
		static constexpr std::string_view k_stencilPassOpJsonKey      = "StencilPassOp";
		static constexpr std::string_view k_stencilFUNCJsonKey        = "StencilFunc";

		// SampleDesc
		static constexpr std::string_view k_countJsonKey   = "Count";
		static constexpr std::string_view k_qualityJsonKey = "Quality";

		static constexpr UINT k_noForcedSampleCount = 0U;
	};
}