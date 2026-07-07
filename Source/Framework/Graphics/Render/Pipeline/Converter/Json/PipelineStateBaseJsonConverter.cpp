#include "PipelineStateBaseJsonConverter.h"

void FWK::Converter::PipelineStateBaseJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::PipelineStateBase& a_pipelineStateBase) const
{
	if (a_rootJson.is_null()) { return; }

	const auto l_useRootSignatureType = a_rootJson.value(k_useRootSignatureTypeJsonKey, Enum::RootSignatureType::Invalid);
	const auto l_pipelineStateFlags   = a_rootJson.value(k_pipelineStateFlagsJsonKey,   static_cast<UINT>(D3D12_PIPELINE_STATE_FLAG_NONE));

	a_pipelineStateBase.SetUseRootSignatureType(l_useRootSignatureType);
	a_pipelineStateBase.SetPipelineStateFlags  (static_cast<D3D12_PIPELINE_STATE_FLAGS>(l_pipelineStateFlags));
}

nlohmann::json FWK::Converter::PipelineStateBaseJsonConverter::Serialize(const Graphics::PipelineStateBase& a_pipelineStateBase) const
{
	nlohmann::json l_rootJson = {};

	// 使用するルートシグネチャタイプのをシリアライズ
	l_rootJson[k_useRootSignatureTypeJsonKey]  = a_pipelineStateBase.GetVALUseRootSignatureType ();
	
	// D3D12_PIPELINE_STATE_FLAGSはビットフラグなので、
	// 複数フラグの組み合わせが存在するが、絶対に変くしないEnumなので数値変換で保存する
	l_rootJson[k_pipelineStateFlagsJsonKey] = static_cast<UINT>(a_pipelineStateBase.GetVALPipelineStateFlags());

	return l_rootJson;
}