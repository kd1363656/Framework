#include "ComputePipelineStateJsonConverter.h"

void FWK::Converter::ComputePipelineStateJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::ComputePipelineState& a_computePipelineState) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_computeShaderJson = a_rootJson.value(k_computeShaderJsonKey, nlohmann::json{});
		!l_computeShaderJson.is_null())
	{
		auto& l_computeShader = a_computePipelineState.GetMutableREFComputeShader();

		// ComputeShaderが使用する.csoのパスや、
		// EntryPoint、ShaderModelなどを復元する
		l_computeShader.Deserialize(l_computeShaderJson);
	}
}

nlohmann::json FWK::Converter::ComputePipelineStateJsonConverter::Serialize(const Graphics::ComputePipelineState& a_computePipelineState) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_computeShader = a_computePipelineState.GetREFComputeShader();

	l_rootJson[k_computeShaderJsonKey] = l_computeShader.Serialize();

	return l_rootJson;
}