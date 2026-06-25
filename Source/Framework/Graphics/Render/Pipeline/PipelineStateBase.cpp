#include "PipelineStateBase.h"

void FWK::Graphics::PipelineStateBase::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::Graphics::PipelineStateBase::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::PipelineStateBase::AddRTVFormat(const DXGI_FORMAT a_format)
{
	// もしRTVFormatの要素数を超えてしまっていたらreturn
	FWK_ASSERT_RETURN_IF_FAILED(m_rtvFormatList.size() >= D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT, "RTVFormatListの要素数がD3DX12_MESH_SHADER_PIPELINE_STATE_DESCのRTVFormatの要素数を超えており、RTVFormatの追加処理に失敗しました。");

	m_rtvFormatList.emplace_back(a_format);
}

D3D12_SHADER_BYTECODE FWK::Graphics::PipelineStateBase::FetchShaderByteCode(const Shader& a_shader) const
{
	const auto& l_blob = a_shader.GetDXCBlob();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_blob, "シェーダーバイトコードの取得に失敗しました。", {});

	return { l_blob->GetBufferPointer(), l_blob->GetBufferSize() };
}