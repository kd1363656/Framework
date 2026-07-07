#include "PipelineStateBaseJsonConverter.h"

void FWK::Converter::PipelineStateBaseJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::PipelineStateBase& a_pipelineStateBase) const
{
	if (a_rootJson.is_null()) { return; }

	const auto l_useRootSignatureType  = a_rootJson.value(k_useRootSignatureTypeJsonKey,  Enum::RootSignatureType::Invalid);

	a_pipelineStateBase.SetUseRootSignatureType (l_useRootSignatureType);
}

nlohmann::json FWK::Converter::PipelineStateBaseJsonConverter::Serialize(const Graphics::PipelineStateBase& a_pipelineStateBase) const
{
	nlohmann::json l_rootJson = {};

	// 使用するルートシグネチャタイプのをシリアライズ
	l_rootJson[k_useRootSignatureTypeJsonKey]  = a_pipelineStateBase.GetVALUseRootSignatureType ();
	
	return l_rootJson;
}