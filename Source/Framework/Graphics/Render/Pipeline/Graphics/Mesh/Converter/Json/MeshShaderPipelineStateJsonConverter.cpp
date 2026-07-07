#include "MeshShaderPipelineStateJsonConverter.h"

void FWK::Converter::MeshShaderPipelineStateJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::MeshShaderPipelineState& a_meshShaderPipelineState) const
{
	if (a_rootJson.is_null()) { return; }

	// アンプリフィケーションシェーダーのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_amplificationShaderJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_amplificationShader = a_meshShaderPipelineState.GetMutableREFAmplificationShader();

		Utility::DeserializeOptionalShader(l_json, l_amplificationShader);
	}

	// メッシュシェーダーのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_meshShaderJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_meshShader = a_meshShaderPipelineState.GetMutableREFMeshShader();

		l_meshShader.Deserialize(l_json);
	}

	// ピクセルシェーダーのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_pixelShaderJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_pixelShader = a_meshShaderPipelineState.GetMutableREFPixelShader();

		Utility::DeserializeOptionalShader(l_json, l_pixelShader);
	}	
}

nlohmann::json FWK::Converter::MeshShaderPipelineStateJsonConverter::Serialize(const Graphics::MeshShaderPipelineState& a_meshShaderPipelineState) const
{
	nlohmann::json l_rootJson = {};
	
	// アンプリフィケーションシェーダーのシリアライズ
	if (auto& l_amplificationShader = a_meshShaderPipelineState.GetREFAmplificationShader())
	{
		l_rootJson[k_amplificationShaderJsonKey] = l_amplificationShader->Serialize();
	}

	auto& l_meshShader = a_meshShaderPipelineState.GetREFMeshShader();
	
	// メッシュシェーダーのシリアライズ
	l_rootJson[k_meshShaderJsonKey] = l_meshShader.Serialize();

	// ピクセルシェーダーのシリアライズ
	if (auto& l_pixelShader = a_meshShaderPipelineState.GetREFPixelShader())
	{
		l_rootJson[k_pixelShaderJsonKey] = l_pixelShader->Serialize();
	}

	return l_rootJson;
}