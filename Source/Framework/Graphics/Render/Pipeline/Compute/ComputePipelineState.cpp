#include "ComputePipelineState.h"

void FWK::Graphics::ComputePipelineState::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	PipelineStateBase::Deserialize(a_rootJson);
	m_jsonConverter.Deserialize   (a_rootJson, *this);
}

bool FWK::Graphics::ComputePipelineState::Create(const Device& a_device, const ShaderCompiler& a_shaderCompiler, const Renderer& a_renderer)
{
	// JSONで指定されたRootSignatureTypeから、
	// このComputePipelineStateが使用するRootSignatureを取得する
	PrepareCommonPipelineStateCreate(a_device, a_renderer);

	const auto& l_device = a_device.GetREFDevice();

	FWK_ASSERT_RETURN_VALUE_IF(!l_device, "Deviceが作成されておらず、ComputePipelineStateの作成に失敗しました。", false);

	const auto& l_useRootSignature = GetREFUseRootSignature().lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_useRootSignature, "ComputePipelineStateが使用するRootSignatureの取得に失敗しました。", false);

	const auto& l_rootSignature = l_useRootSignature->GetREFRootSignature();

	// Shaderクラスは、VisualStudioとDXCによって
	// 事前コンパイルされた.csoファイルを読み込む
	FWK_ASSERT_RETURN_VALUE_IF(!m_computeShader.CreateFromFile(a_shaderCompiler), "ComputeShaderの作成に失敗したため、ComputePipelineStateの作成に失敗しました。", false);

	D3D12_COMPUTE_PIPELINE_STATE_DESC l_pipelineStateDesc = {};

	// ルートシグネチャをセットする
	l_pipelineStateDesc.pRootSignature = l_rootSignature.Get();

	// 実際に実行するComputeShaderのDXILBytecodeを設定する
	l_pipelineStateDesc.CS = FetchShaderByteCode(m_computeShader);

	// ノード数
	l_pipelineStateDesc.NodeMask = Constant::k_defaultGPUNodeMask;

	l_pipelineStateDesc.Flags = GetVALPipelineStateFlags();

	l_pipelineStateDesc.CachedPSO = {};

	auto& l_pipelineState = GetMutableREFPipelineState();

	const auto l_hr = l_device->CreateComputePipelineState(&l_pipelineStateDesc, IID_PPV_ARGS(l_pipelineState.ReleaseAndGetAddressOf()));

	FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "ComputePipelineStateの作成に失敗しました。", false);

	return true;
}

nlohmann::json FWK::Graphics::ComputePipelineState::Serialize() const
{
	auto l_rootJson = PipelineStateBase::Serialize();

	Utility::UpdateJson(l_rootJson, m_jsonConverter.Serialize(*this));

	return l_rootJson;
}