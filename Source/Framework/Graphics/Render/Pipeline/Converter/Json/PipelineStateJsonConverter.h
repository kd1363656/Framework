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

		void EnsureShader(std::shared_ptr<Graphics::Shader>& a_shader) const;

		// PipelineState
		static constexpr std::string_view k_amplificationShaderJsonKey   = "AmplificationShader";
		static constexpr std::string_view k_meshShaderJsonKey            = "MeshShader";
		static constexpr std::string_view k_pixelShaderJsonKey           = "PixelShader";
	};
}