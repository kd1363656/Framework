#pragma once

namespace FWK::Graphics
{
	class PipelineStateBase
	{
	public:

				 PipelineStateBase() = default;
		virtual ~PipelineStateBase() = default;

		virtual void Deserialize(const nlohmann::json& a_rootJson);
		virtual bool Create     (const Device&		   a_device, const ShaderCompiler& a_shaderCompiler, const Renderer& a_renderer) = 0;

		virtual nlohmann::json Serialize() const;

	private:

	};
}