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

	private:

		Converter::StandardPipelineStateJsonConverter m_jsonConverter = {};
	};
}