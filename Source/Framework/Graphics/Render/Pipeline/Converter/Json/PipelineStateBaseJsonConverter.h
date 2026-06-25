#pragma once

namespace FWK::Graphics
{
	class PipelineState;
}

namespace FWK::Converter
{
	class PipelineStateBaseJsonConverter final
	{
	public:
		
		 PipelineStateBaseJsonConverter() = default;
		~PipelineStateBaseJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::PipelineState& a_pipelineState) const;

		nlohmann::json Serialize(const Graphics::PipelineState& a_pipelineState) const;
	};
}