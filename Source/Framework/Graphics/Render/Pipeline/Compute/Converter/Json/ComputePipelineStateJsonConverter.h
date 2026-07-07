#pragma once

namespace FWK::Graphics
{
	class ComputePipelineState;
}

namespace FWK::Converter
{
	class ComputePipelineStateJsonConverter
	{
	public:

		 ComputePipelineStateJsonConverter() = default;
		~ComputePipelineStateJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::ComputePipelineState& a_computePipelineState) const;

		nlohmann::json Serialize(const Graphics::ComputePipelineState& a_computePipelineState) const;

	private:

		static constexpr std::string_view k_computeShaderJsonKey = "ComputeShader";
	};
}