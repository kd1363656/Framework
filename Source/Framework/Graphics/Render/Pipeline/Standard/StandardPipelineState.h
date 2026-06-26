#pragma once

namespace FWK::Graphics
{
	class StandardPipelineState final : public PipelineStateBase
	{
	public:

		 StandardPipelineState()          = default;
		~StandardPipelineState() override = default;

		void Deserialize(const nlohmann::json& a_rootJson)																	 override;
		bool Create     (const Device&         a_device, const ShaderCompiler& a_shaderCompiler, const Renderer& a_renderer) override;

		nlohmann::json Serialize() const override;

		const auto& GetREFVertexShader() const { return m_vertexShader; }
		const auto& GetREFPixelShader () const { return m_pixelShader; }

		auto& GetMutableREFVertexShader() { return m_vertexShader; }
		auto& GetMutableREFPixelShader () { return m_pixelShader; }

	private:

		Shader m_vertexShader = {};

		std::shared_ptr<Shader> m_pixelShader = nullptr;

		Converter::StandardPipelineStateJsonConverter m_jsonConverter = {};

		FWK_DEFINE_TYPE_INFO(StandardPipelineState, PipelineStateBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::PipelineStateSharedFactory, FWK::Graphics::StandardPipelineState)