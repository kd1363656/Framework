#include "PipelineStateJsonConverter.h"

void FWK::Converter::PipelineStateJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::PipelineState& a_pipelineState) const
{
	if (a_rootJson.is_null()) { return; }

	// アンプリフィケーションシェーダーのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_amplificationShaderJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_amplificationShader = a_pipelineState.GetMutableREFAmplificationShader();

		DeserializeOptionalShader(l_json, l_amplificationShader);
	}

	// メッシュシェーダーのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_meshShaderJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_meshShader = a_pipelineState.GetMutableREFMeshShader();

		l_meshShader.Deserialize(l_json);
	}

	// ピクセルシェーダーのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_pixelShaderJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_pixelShader = a_pipelineState.GetMutableREFPixelShader();

		DeserializeOptionalShader(l_json, l_pixelShader);
	}

}
nlohmann::json FWK::Converter::PipelineStateJsonConverter::Serialize(const Graphics::PipelineState & a_pipelineState) const
{
	nlohmann::json l_rootJson = {};
	
	// アンプリフィケーションシェーダーのシリアライズ
	if (auto& l_amplificationShader = a_pipelineState.GetREFAmplificationShader())
	{
		l_rootJson[k_amplificationShaderJsonKey] = l_amplificationShader->Serialize();
	}

	auto& l_meshShader = a_pipelineState.GetREFMeshShader();
	
	// メッシュシェーダーのシリアライズ
	l_rootJson[k_meshShaderJsonKey] = l_meshShader.Serialize();

	// ピクセルシェーダーのシリアライズ
	if (auto& l_pixelShader = a_pipelineState.GetREFPixelShader())
	{
		l_rootJson[k_pixelShaderJsonKey] = l_pixelShader->Serialize();
	}

	return l_rootJson;
}

void FWK::Converter::PipelineStateJsonConverter::DeserializeOptionalShader(const nlohmann::json& a_rootJson, std::shared_ptr<Graphics::Shader>& a_shader) const
{
	if (a_rootJson.is_null()) { return; }

	// jsonで保存されていたということは使う予定のシェーダーなのでインスタンス化
	// されていなければインスタンス化する
	EnsureShader(a_shader);

	a_shader->Deserialize(a_rootJson);
}

void FWK::Converter::PipelineStateJsonConverter::EnsureShader(std::shared_ptr<Graphics::Shader>&a_shader) const
{
	// 既にインスタンス化されていたら"return"
	if (a_shader) { return; }

	a_shader = std::make_shared<Graphics::Shader>();
}