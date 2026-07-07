#pragma once

namespace FWK::Graphics
{
	class ComputePipelineState : public PipelineStateBase
	{
	public:

		 ComputePipelineState()          = default;
		~ComputePipelineState() override = default;

		void Deserialize(const nlohmann::json& a_rootJson) override;

		bool Create(const Device& a_device, const ShaderCompiler& a_shaderCompiler, const Renderer& a_renderer) override;

		nlohmann::json Serialize() const override;

		const auto& GetREFComputeShader() const { return m_computeShader; }

		auto& GetMutableREFComputeShader() { return m_computeShader; }

	private:

		Shader m_computeShader = {};

		Converter::ComputePipelineStateJsonConverter m_jsonConverter = {};

		FWK_DEFINE_TYPE_INFO(ComputePipelineState, PipelineStateBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::PipelineStateSharedFactory, FWK::Graphics::ComputePipelineState)