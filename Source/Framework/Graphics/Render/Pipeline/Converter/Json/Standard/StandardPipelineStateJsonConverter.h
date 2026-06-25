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
	};
}