#pragma once

namespace FWK::Graphics
{
	class PipelineStateBase;
}

namespace FWK::Converter
{
	class PipelineStateBaseJsonConverter final
	{
	public:
		
		 PipelineStateBaseJsonConverter() = default;
		~PipelineStateBaseJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::PipelineStateBase& a_pipelineStateBase) const;

		nlohmann::json Serialize(const Graphics::PipelineStateBase& a_pipelineStateBase) const;

	private:

		static constexpr std::string_view k_useRootSignatureTypeJsonKey = "UseRootSignatureType";
		static constexpr std::string_view k_pipelineStateFlagsJsonKey   = "PipelineStateFlags";
	};
}