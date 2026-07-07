#pragma once

namespace FWK::Graphics
{
	class StandardPipelineState;
}

namespace FWK::Converter
{
	class StandardPipelineStateJsonConverter final
	{
	public:
		
		 StandardPipelineStateJsonConverter() = default;
		~StandardPipelineStateJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::StandardPipelineState& a_standardPipelineState) const;

		nlohmann::json Serialize(const Graphics::StandardPipelineState& a_standardPipelineState) const;

	private:

		void DeserializeInputLayout(const nlohmann::json& a_rootJson, Graphics::StandardPipelineState& a_standardPipelineState) const;

		nlohmann::json SerializeInputLayout(const Graphics::StandardPipelineState& a_standardPipelineState) const;

		static constexpr std::string_view k_vertexShaderJsonKey   = "VertexShader";
		static constexpr std::string_view k_hullShaderJsonKey     = "HullShader";
		static constexpr std::string_view k_domainShaderJsonKey   = "DomainShader";
		static constexpr std::string_view k_geometryShaderJsonKey = "GeometryShader";
		static constexpr std::string_view k_pixelShaderJsonKey    = "PixelShader";

		static constexpr std::string_view k_inputLayoutJsonKey = "InputLayout";

		static constexpr std::string_view k_semanticNameJsonKey         = "SemanticName";
		static constexpr std::string_view k_semanticIndexJsonKey        = "SemanticIndex";
		static constexpr std::string_view k_formatJsonKey               = "Format";
		static constexpr std::string_view k_inputSlotJsonKey            = "InputSlot";
		static constexpr std::string_view k_alignedByteOffsetJsonKey    = "AlignedByteOffset";
		static constexpr std::string_view k_inputSlotClassJsonKey       = "InputSlotClass";
		static constexpr std::string_view k_instanceDataStepRateJsonKey = "InstanceDataStepRate";

		static constexpr UINT k_defaultInputElementSemanticIndex        = 0U;
		static constexpr UINT k_defaultInputElementInputSlot            = 0U;
		static constexpr UINT k_defaultInputElementInstanceDataStepRate = 0U;
	};
}