#pragma once

namespace FWK::Graphics
{
	class PipelineState final
	{
	public:

		 PipelineState() = default;
		~PipelineState() = default;

		void Deserialize(const nlohmann::json& a_rootJson);
		bool Create     (const Device&         a_device, const ShaderCompiler& a_shaderCompiler, const Renderer& a_renderer);

		nlohmann::json Serialize() const;

		const auto& GetREFAmplificationShader() const { return m_amplificationShader; }
		const auto& GetREFMeshShader         () const { return m_meshShader; }
		const auto& GetREFPixelShader        () const { return m_pixelShader; }

		auto& GetMutableREFAmplificationShader() { return m_amplificationShader; }
		auto& GetMutableREFMeshShader         () { return m_meshShader; }
		auto& GetMutableREFPixelShader        () { return m_pixelShader; }

	private:

		std::shared_ptr<Shader> m_amplificationShader = nullptr;
		std::shared_ptr<Shader> m_pixelShader         = nullptr;

		Shader m_meshShader = {};

		Converter::PipelineStateJsonConverter m_jsonConverter = {};
	};
}