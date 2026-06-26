#include "StandardPipelineStateJsonConverter.h"

void FWK::Converter::StandardPipelineStateJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::StandardPipelineState& a_standardPipelineState) const
{
	if (a_rootJson.is_null()) { return; }

	// バーテックスシェーダーのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_vertexShaderJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_vertexShader = a_standardPipelineState.GetMutableREFVertexShader();

		l_vertexShader.Deserialize(l_json);
	}

	// ハルシェーダーのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_hullShaderJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_hullShader = a_standardPipelineState.GetMutableREFHullShader();

		Utility::DeserializeOptionalShader(l_json, l_hullShader);
	}

	// ドメインシェーダーのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_domainShaderJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_domainShader = a_standardPipelineState.GetMutableREFDomainShader();

		Utility::DeserializeOptionalShader(l_json, l_domainShader);
	}

	// ジオメトリシェーダーのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_geometryShaderJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_geometryShader = a_standardPipelineState.GetMutableREFGeometryShader();

		Utility::DeserializeOptionalShader(l_json, l_geometryShader);
	}

	// ピクセルシェーダーのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_pixelShaderJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_pixelShader = a_standardPipelineState.GetMutableREFPixelShader();

		Utility::DeserializeOptionalShader(l_json, l_pixelShader);
	}
}

nlohmann::json FWK::Converter::StandardPipelineStateJsonConverter::Serialize(const Graphics::StandardPipelineState& a_standardPipelineState) const
{
	nlohmann::json l_rootJson = {};

	// バーテックスシェーダーのシリアライズ
	const auto& l_vertexShader = a_standardPipelineState.GetREFVertexShader();

	l_rootJson[k_vertexShaderJsonKey] = l_vertexShader.Serialize();

	// ハルシェーダーのデシリアライズ
	if (const auto& l_hullShader = a_standardPipelineState.GetREFHullShader())
	{
		l_rootJson[k_hullShaderJsonKey] = l_hullShader->Serialize();
	}

	// ドメインシェーダーのデシリアライズ
	if (const auto& l_domainShader = a_standardPipelineState.GetREFDomainShader())
	{
		l_rootJson[k_domainShaderJsonKey] = l_domainShader->Serialize();
	}

	// ジオメトリシェーダーのデシリアライズ
	if (const auto& l_geometryShader = a_standardPipelineState.GetREFGeometryShader())
	{
		l_rootJson[k_geometryShaderJsonKey] = l_geometryShader->Serialize();
	}

	// ピクセルシェーダーのデシリアライズ
	if (const auto& l_pixelShader = a_standardPipelineState.GetREFPixelShader())
	{
		l_rootJson[k_pixelShaderJsonKey] = l_pixelShader->Serialize();
	}

	return l_rootJson;
}