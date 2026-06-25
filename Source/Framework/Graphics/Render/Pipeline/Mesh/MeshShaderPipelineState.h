#pragma once

namespace FWK::Graphics
{
	class MeshShaderPipelineState final : public PipelineStateBase
	{
	public:

		 MeshShaderPipelineState()          = default;
		~MeshShaderPipelineState() override = default;

		void Deserialize(const nlohmann::json& a_rootJson)																	 override;
		bool Create     (const Device&         a_device, const ShaderCompiler& a_shaderCompiler, const Renderer& a_renderer) override;

		nlohmann::json Serialize() const override;

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

		Converter::MeshShaderPipelineStateJsonConverter m_jsonConverter = {};
	};
}