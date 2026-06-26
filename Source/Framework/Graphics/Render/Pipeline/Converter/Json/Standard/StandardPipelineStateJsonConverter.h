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

		static constexpr std::string_view k_vertexShaderJsonKey   = "VertexShader";
		static constexpr std::string_view k_hullShaderJsonKey     = "HullShader";
		static constexpr std::string_view k_domainShaderJsonKey   = "DomainShader";
		static constexpr std::string_view k_geometryShaderJsonKey = "GeometryShader";
		static constexpr std::string_view k_pixelShaderJsonKey    = "PixelShader";
	};
}