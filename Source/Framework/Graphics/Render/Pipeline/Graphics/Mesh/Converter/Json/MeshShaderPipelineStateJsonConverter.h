#pragma once

namespace FWK::Graphics
{
	class MeshShaderPipelineState;
}

namespace FWK::Converter
{
	class MeshShaderPipelineStateJsonConverter final
	{
	public:
		
		 MeshShaderPipelineStateJsonConverter() = default;
		~MeshShaderPipelineStateJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::MeshShaderPipelineState& a_meshShaderPipelineState) const;

		nlohmann::json Serialize(const Graphics::MeshShaderPipelineState& a_meshShaderPipelineState) const;

	private:

		static constexpr std::string_view k_amplificationShaderJsonKey   = "AmplificationShader";
		static constexpr std::string_view k_meshShaderJsonKey            = "MeshShader";
		static constexpr std::string_view k_pixelShaderJsonKey           = "PixelShader";
	};
}