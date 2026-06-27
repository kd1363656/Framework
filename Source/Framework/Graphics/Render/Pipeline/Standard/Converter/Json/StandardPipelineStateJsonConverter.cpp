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

	// InputLayoutのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_inputLayoutJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializeInputLayout(l_json, a_standardPipelineState);
	}
}

nlohmann::json FWK::Converter::StandardPipelineStateJsonConverter::Serialize(const Graphics::StandardPipelineState& a_standardPipelineState) const
{
	nlohmann::json l_rootJson = {};

	// バーテックスシェーダーのシリアライズ
	const auto& l_vertexShader = a_standardPipelineState.GetREFVertexShader();

	l_rootJson[k_vertexShaderJsonKey] = l_vertexShader.Serialize();

	// ハルシェーダーのシリアライズ
	if (const auto& l_hullShader = a_standardPipelineState.GetREFHullShader())
	{
		l_rootJson[k_hullShaderJsonKey] = l_hullShader->Serialize();
	}

	// ドメインシェーダーのシリアライズ
	if (const auto& l_domainShader = a_standardPipelineState.GetREFDomainShader())
	{
		l_rootJson[k_domainShaderJsonKey] = l_domainShader->Serialize();
	}

	// ジオメトリシェーダーのシリアライズ
	if (const auto& l_geometryShader = a_standardPipelineState.GetREFGeometryShader())
	{
		l_rootJson[k_geometryShaderJsonKey] = l_geometryShader->Serialize();
	}

	// ピクセルシェーダーのシリアライズ
	if (const auto& l_pixelShader = a_standardPipelineState.GetREFPixelShader())
	{
		l_rootJson[k_pixelShaderJsonKey] = l_pixelShader->Serialize();
	}

	// InputLayoutのシリアライズ
	l_rootJson[k_inputLayoutJsonKey] = SerializeInputLayout(a_standardPipelineState);

	return l_rootJson;
}

void FWK::Converter::StandardPipelineStateJsonConverter::DeserializeInputLayout(const nlohmann::json& a_rootJson, Graphics::StandardPipelineState& a_standardPipelineState) const
{
	if (a_rootJson.is_null())		   { return; }
	if (!Utility::IsArray(a_rootJson)) { return; }

	// まずはInput関係の配列のリセット
	a_standardPipelineState.ClearInputLayout();

	for (const auto& l_json : a_rootJson)
	{
		const auto l_semanticName = l_json.value(k_semanticNameJsonKey, std::string());

		if (l_semanticName.empty()) { continue; }

		Struct::StandardPipelineInputElement l_inputElement = {};

		l_inputElement.m_semanticName = l_semanticName;

		l_inputElement.m_inputElementDesc.SemanticName         = nullptr;
		l_inputElement.m_inputElementDesc.SemanticIndex        = l_json.value(k_semanticIndexJsonKey,        k_defaultInputElementSemanticIndex);
		l_inputElement.m_inputElementDesc.Format               = l_json.value(k_formatJsonKey,               DXGI_FORMAT_UNKNOWN);
		l_inputElement.m_inputElementDesc.InputSlot            = l_json.value(k_inputSlotJsonKey,            k_defaultInputElementInputSlot);
		l_inputElement.m_inputElementDesc.AlignedByteOffset    = l_json.value(k_alignedByteOffsetJsonKey,    D3D12_APPEND_ALIGNED_ELEMENT);
		l_inputElement.m_inputElementDesc.InputSlotClass       = l_json.value(k_inputSlotClassJsonKey,       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);
		l_inputElement.m_inputElementDesc.InstanceDataStepRate = l_json.value(k_instanceDataStepRateJsonKey, k_defaultInputElementInstanceDataStepRate);

		a_standardPipelineState.AddInputElementDesc(l_inputElement);
	}
}

nlohmann::json FWK::Converter::StandardPipelineStateJsonConverter::SerializeInputLayout(const Graphics::StandardPipelineState& a_standardPipelineState) const
{
	auto l_rootJsonArray = nlohmann::json::array();

	const auto& l_inputElementList = a_standardPipelineState.GetREFInputElementList();

	for (const auto& l_inputElement : l_inputElementList)
	{
		nlohmann::json l_json = {};

		l_json[k_semanticNameJsonKey]         = l_inputElement.m_semanticName;
		l_json[k_semanticIndexJsonKey]        = l_inputElement.m_inputElementDesc.SemanticIndex;
		l_json[k_formatJsonKey]               = l_inputElement.m_inputElementDesc.Format;
		l_json[k_inputSlotJsonKey]            = l_inputElement.m_inputElementDesc.InputSlot;
		l_json[k_alignedByteOffsetJsonKey]    = l_inputElement.m_inputElementDesc.AlignedByteOffset;
		l_json[k_inputSlotClassJsonKey]       = l_inputElement.m_inputElementDesc.InputSlotClass;
		l_json[k_instanceDataStepRateJsonKey] = l_inputElement.m_inputElementDesc.InstanceDataStepRate;

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}