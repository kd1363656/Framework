#include "StandardPipelineState.h"


void FWK::Graphics::StandardPipelineState::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	GraphicsPipelineStateBase::Deserialize(a_rootJson);
	m_jsonConverter.Deserialize           (a_rootJson, *this);
}

bool FWK::Graphics::StandardPipelineState::Create(const Device& a_device, const ShaderCompiler& a_shaderCompiler, const Renderer& a_renderer)
{
	// 作成前にD3D12_INPUT_ELEMENT_DESCの情報を構築
	BuildInputElementDescList();

	// 使用するルートシグネチャを探すなどの処理を行う
	PrepareCommonPipelineStateCreate(a_device, a_renderer);

	const auto& l_device = a_device.GetREFDevice().Get();

	FWK_ASSERT_RETURN_VALUE_IF(!l_device, "デバイスが作成されておらず、パイプラインステートの作成処理に失敗しました。", false);

	const auto& l_useRootSignature = GetREFUseRootSignature().lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_useRootSignature, "対象となるルートシグネチャの取得に失敗し、パイプラインステートの作成処理に失敗しました。", false);

	const auto& l_rootSignature = l_useRootSignature->GetREFRootSignature();

	FWK_ASSERT_RETURN_VALUE_IF(!l_rootSignature, "ルートシグネチャが作成されておらず、パイプラインステートの作成処理に失敗しました。", false);

	const auto& l_rtvFormatList = GetREFRTVFormatList();

	// RTVFormatListが空ならreturn
	FWK_ASSERT_RETURN_VALUE_IF(l_rtvFormatList.empty(), "RTVFormatListが空のため、パイプラインステートの作成処理に失敗しました。", false);

	// RTVFormatListの要素数がレンダーターゲットの要素数を超えていたらreturn
	FWK_ASSERT_RETURN_VALUE_IF(l_rtvFormatList.size() > D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT, "RTVFormatListの要素数がDirectX12のRenderTarget上限を超えており、パイプラインステートの作成処理に失敗しました。", false);

	// バーテックスシェーダーのコンパイル
	FWK_ASSERT_RETURN_VALUE_IF(!m_vertexShader.CreateFromFile(a_shaderCompiler), "VertexShaderの作成に失敗したため、StandardPipelineStateの作成処理に失敗しました。", false);

	// ハルシェーダーのコンパイル
	if (m_hullShader)
	{
		FWK_ASSERT_RETURN_VALUE_IF(!m_hullShader->CreateFromFile(a_shaderCompiler), "HullShaderの作成に失敗したため、StandardPipelineStateの作成処理に失敗しました。", false);
	}

	// ドメインシェーダーのコンパイル
	if (m_domainShader)
	{
		FWK_ASSERT_RETURN_VALUE_IF(!m_domainShader->CreateFromFile(a_shaderCompiler), "DomainShaderの作成に失敗したため、StandardPipelineStateの作成処理に失敗しました。", false);
	}

	// ジオメトリシェーダーのコンパイル
	if (m_geometryShader)
	{
		FWK_ASSERT_RETURN_VALUE_IF(!m_geometryShader->CreateFromFile(a_shaderCompiler), "GeometryShaderの作成に失敗したため、StandardPipelineStateの作成処理に失敗しました。", false);
	}

	// ピクセルシェーダーのコンパイル
	if (m_pixelShader)
	{
		FWK_ASSERT_RETURN_VALUE_IF(!m_pixelShader->CreateFromFile(a_shaderCompiler), "PixelShaderの作成に失敗したため、StandardPipelineStateの作成処理に失敗しました。", false);
	}

	// HullShaderとDomainShaderは基本的にセットで使う。
	// 片方だけ設定されている場合は、テッセレーションPipelineとして不完全。
	FWK_ASSERT_RETURN_VALUE_IF( m_hullShader && !m_domainShader, "HullShaderが設定されていますがDomainShaderが無いため、StandardPipelineStateの作成処理に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(!m_hullShader &&  m_domainShader, "DomainShaderが設定されていますがHullShaderが無いため、StandardPipelineStateの作成処理に失敗しました。", false);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC l_pipelineStateDesc = {};


	// このPSOで使用するルートシグネチャを設定する
	// ルートシグネチャは「シェーダーへどのリソースをどう渡すか」のルール
	l_pipelineStateDesc.pRootSignature = l_rootSignature.Get();

	// 各シェーダーをセット
	l_pipelineStateDesc.VS = FetchShaderByteCode(m_vertexShader);

	if (m_hullShader)
	{
		l_pipelineStateDesc.HS = FetchShaderByteCode(*m_hullShader);
	}

	if (m_domainShader)
	{
		l_pipelineStateDesc.DS = FetchShaderByteCode(*m_domainShader);
	}

	if (m_geometryShader)
	{
		l_pipelineStateDesc.GS = FetchShaderByteCode(*m_geometryShader);
	}

	if (m_pixelShader)
	{
		l_pipelineStateDesc.PS = FetchShaderByteCode(*m_pixelShader);
	}

	l_pipelineStateDesc.InputLayout.pInputElementDescs = m_inputElementDescList.empty() ? nullptr : m_inputElementDescList.data();
	l_pipelineStateDesc.InputLayout.NumElements        = static_cast<UINT>           (m_inputElementDescList.size());

	// ラスタライザーやブレンドステートのセット
	l_pipelineStateDesc.RasterizerState   = GetREFRasterizerDesc  ();
	l_pipelineStateDesc.BlendState        = GetREFBlendDesc       ();
	l_pipelineStateDesc.DepthStencilState = GetREFDepthStencilDesc();

	l_pipelineStateDesc.SampleMask = GetVALSampleMask();

	l_pipelineStateDesc.Flags = GetVALPipelineStateFlags();

	l_pipelineStateDesc.PrimitiveTopologyType = GetVALPrimitiveTopologyType();

	l_pipelineStateDesc.NumRenderTargets = static_cast<UINT>(l_rtvFormatList.size());

	std::copy(l_rtvFormatList.begin(), l_rtvFormatList.end(), l_pipelineStateDesc.RTVFormats);

	l_pipelineStateDesc.DSVFormat  = GetVALDSVFormat ();
	l_pipelineStateDesc.SampleDesc = GetREFSampleDesc();

	auto& l_pipelineState = GetMutableREFPipelineState();

	// パイプラインステートの作成
	const auto l_hr = l_device->CreateGraphicsPipelineState(&l_pipelineStateDesc, IID_PPV_ARGS(l_pipelineState.ReleaseAndGetAddressOf()));

	FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "StandardPipelineStateの作成処理に失敗しました。", false);

	return true;
}

nlohmann::json FWK::Graphics::StandardPipelineState::Serialize() const
{
	auto l_rootJson = GraphicsPipelineStateBase::Serialize();

	Utility::UpdateJson(l_rootJson, m_jsonConverter.Serialize(*this));

	return l_rootJson;
}

void FWK::Graphics::StandardPipelineState::ClearInputLayout()
{
	m_inputElementList.clear    ();
	m_inputElementDescList.clear();
}

void FWK::Graphics::StandardPipelineState::AddInputElementDesc(const Struct::StandardPipelineInputElement& a_inputElement)
{
	if (a_inputElement.m_semanticName.empty()) { return; }

	m_inputElementList.emplace_back(a_inputElement);
}

void FWK::Graphics::StandardPipelineState::BuildInputElementDescList()
{
	m_inputElementDescList.clear();

	// 内部でconst char*を使っているため要素の再確保が生じたときにnullptrにならないようにreserve()する
	m_inputElementDescList.reserve(m_inputElementList.size());

	// l_inputElementの情報をコピーしてデスク情報を作成
	for (auto& l_inputElement : m_inputElementList)
	{
		auto& l_inputElementDesc = l_inputElement.m_inputElementDesc;

		l_inputElementDesc.SemanticName = l_inputElement.m_semanticName.c_str();

		m_inputElementDescList.emplace_back(l_inputElementDesc);
	}
}