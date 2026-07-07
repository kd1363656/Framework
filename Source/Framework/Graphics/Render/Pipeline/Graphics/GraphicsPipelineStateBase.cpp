#include "GraphicsPipelineStateBase.h"

void FWK::Graphics::GraphicsPipelineStateBase::Deserialize(const nlohmann::json& a_rootJson)
{
	PipelineStateBase::Deserialize(a_rootJson);

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::Graphics::GraphicsPipelineStateBase::Serialize() const
{
	auto l_rootJson = PipelineStateBase::Serialize();

	Utility::UpdateJson(l_rootJson, m_jsonConverter.Serialize(*this));

	return l_rootJson;
}

void FWK::Graphics::GraphicsPipelineStateBase::AddRTVFormat(const DXGI_FORMAT a_format)
{
	// もしRTVFormatの要素数を超えてしまっていたらreturn
	FWK_ASSERT_RETURN_IF(m_rtvFormatList.size() >= D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT, "RTVFormatListの要素数がD3DX12_MESH_SHADER_PIPELINE_STATE_DESCのRTVFormatの要素数を超えており、RTVFormatの追加処理に失敗しました。");

	m_rtvFormatList.emplace_back(a_format);
}