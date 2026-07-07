#include "MeshShaderPipelineState.h"

void FWK::Graphics::MeshShaderPipelineState::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	GraphicsPipelineStateBase::Deserialize(a_rootJson);
	m_jsonConverter.Deserialize           (a_rootJson, *this);
}

bool FWK::Graphics::MeshShaderPipelineState::Create(const Device& a_device, const ShaderCompiler& a_shaderCompiler, const Renderer& a_renderer)
{
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

	// 使用するシェーダーをコンパイルする
	// AmplificationShaderとPixelShaderは任意なので、
	// 存在するときだけコンパイルする。
	// MeshShaderはこのPSOで必須なので必ずコンパイルする
	if (m_amplificationShader)
	{
		FWK_ASSERT_RETURN_VALUE_IF(!m_amplificationShader->CreateFromFile(a_shaderCompiler), "AmplificationShaderの作成に失敗したため、パイプラインステートの作成処理に失敗しました。", false);
	}

	FWK_ASSERT_RETURN_VALUE_IF(!m_meshShader.CreateFromFile(a_shaderCompiler), "MeshShaderの作成に失敗したため、パイプラインステートの作成処理に失敗しました。", false);

	if (m_pixelShader)
	{
		FWK_ASSERT_RETURN_VALUE_IF(!m_pixelShader->CreateFromFile(a_shaderCompiler), "PixelShaderの作成に失敗したため、パイプラインステートの作成処理に失敗しました。", false);
	}

	// メッシュシェーダー用パイプラインステート設定構造体
	D3DX12_MESH_SHADER_PIPELINE_STATE_DESC l_pipelineStateDesc = {};

	// AS(AmplificationShader)を設定する
	// ASはメッシュレットの増幅や振り分けを行う前段シェーダー
	// 使わない場合は設定しなくてもよい
	if (m_amplificationShader)
	{
		l_pipelineStateDesc.AS = FetchShaderByteCode(*m_amplificationShader);
	}

	// MS(MeshShader)を設定する
	// メッシュシェーダーパイプラインの中心になる必須シェーダー
	l_pipelineStateDesc.MS = FetchShaderByteCode(m_meshShader);
	
	// PS(PixelShader)を設定する
	// ピクセルの最終色を決めるシェーダー
	// 使わない構成もあるため任意
	if (m_pixelShader) 
	{
		l_pipelineStateDesc.PS = FetchShaderByteCode(*m_pixelShader);
	}

	// このPSOで使用するルートシグネチャを設定する
	// ルートシグネチャは「シェーダーへどのリソースをどう渡すか」のルール
	l_pipelineStateDesc.pRootSignature = l_rootSignature.Get();
	
	// D3D12_RASTERIZER_DESCについての説明
	// FillMode              : ポリゴンを塗りつぶすか、ワイヤーフレームで描くか
	// CullMode              : 前面 / 背面のどちらを描画しないか
	// FrontCounterClockwise : 反時計回りを前面扱いするか
	// DepthBias             : 深度値に加える補正値
	// DepthBiasClamp        : 深度バイアスの最大補正量
	// SlopeScaledDepthBias  : 面の傾きに応じて増える深度バイアス係数
	// DepthClipEnable       : 深度範囲外のピクセルをクリップするか
	// MultisampleEnable     : MSAA関連のラスタライズ設定
	// AntialiasedLineEnable : 線描画時のアンチエイリアスを有効にするか
	// ForcedSampleCount     : 強制サンプル数(通常は0)
	// ConservativeRaster    : 保守的ラスタライズを使うか
	// ラスタライザ設定を指定する
	l_pipelineStateDesc.RasterizerState = GetREFRasterizerDesc();

	// D3D12_BLEND_DESCについての説明
	// AlphaToCoverageEnable  : MSAA使用時にalpha値からサンプル被覆率を作るか
	// IndependentBlendEnable : 複数RenderTargetごとに別々のブレンド設定を使うか
	// RenderTarget[8]        : 最大8枚の描画先それぞれのブレンド設定
	// RenderTarget[i]について説明
	// BlendEnable		     : 色ブレンドを有効にするか
	// SrcBlend			     : 今描く色(RGB)にかける係数
	// DestBlend		     : 既にある色(RGB)にかける係数
	// BlendOp			     : RGBの合成方法
	// SrcBlendAlpha	     : 今描くalphaにかける係数
	// DestBlendAlpha	     : 既にあるalphaの合成方法
	// BlendOpAlpha		     : alphaの合成方法
	// LogicOpEnable	     : 論理演算を使うか
	// LogicOp			     : 論理演算の種類
	// RenderTargetWriteMask : RGBAのどの成分を書き込むか
	// ブレンド設定を指定する
	// ここでは基本的な既定設定を使用する
	// 透明合成などをしたい場合は後でここを変更する
	l_pipelineStateDesc.BlendState = GetREFBlendDesc();

	// 対象となるGPUのノードマスク
	l_pipelineStateDesc.NodeMask = Constant::k_defaultGPUNodeMask;

	l_pipelineStateDesc.Flags = GetVALPipelineStateFlags();

	// D3D12_DEPTH_STENCIL_DESCについての説明
	// DepthEnable      : 深度テストを使うかどうか
	// DepthWriteMask   : 深度値を深度バッファへ書き込むかどうか
	// DepthFunc        : 深度比較の方法(手前なら描く、常に描くなど)
	// StencilEnable    : ステンシルテストを使うかどうか
	// StencilReadMask  : ステンシル値を読むときのビットマスク
	// StencilWriteMask : ステンシル値を書き込むときのビットマスク
	// FrontFace        : 前面ポリゴンに対するステンシル動作
	// BackFace         : 背面ポリゴンに対するステンシル動作
	l_pipelineStateDesc.DepthStencilState = GetREFDepthStencilDesc();

	// サンプルマスクを設定する
	// 通常はUINT_MAXで全サンプル有効
	l_pipelineStateDesc.SampleMask = GetVALSampleMask();

	// RenderTargetの枚数を指定する
	l_pipelineStateDesc.NumRenderTargets = static_cast<UINT>(l_rtvFormatList.size());

	// RTVFormatListの内容をPSO作成用の固定長RTVFormats配列へコピーする
	std::copy(l_rtvFormatList.begin(), l_rtvFormatList.end(), l_pipelineStateDesc.RTVFormats);
	
	// 深度ステンシルビューのフォーマットを設定する
	// 深度を使うPSOでは、実際にOMへセットするDSVと同じフォーマットを指定する必要がある
	l_pipelineStateDesc.DSVFormat = GetVALDSVFormat();

	// DXGI_SAMPLE_DESCについての説明
	// Count : 1ピクセル当たりのサンプル数(1なら通常描画、4なら4x MSAAのように複数回サンプリングする)
	// Quality : サンプル品質レベル(通常は0を使うことが多い、利用可能な値はデバイス / フォーマットごとに確認が必要)
	// MSAAなどのサンプル設定を指定する
	// Count = 1なら通常の非MSAA描画
	l_pipelineStateDesc.SampleDesc = GetREFSampleDesc();

	// このPSOが使うプリミティブ種類を指定する
	l_pipelineStateDesc.PrimitiveTopologyType = GetVALPrimitiveTopologyType();

	// 上で設定したPSO情報をストリーム形式へまとめる
	auto l_psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM{ l_pipelineStateDesc };
	
	D3D12_PIPELINE_STATE_STREAM_DESC l_streamDesc = {};

	// PSOサブオブジェクトストリームの先頭アドレス
	l_streamDesc.pPipelineStateSubobjectStream = &l_psoStream;

	// ストリーム全体のサイズ
	l_streamDesc.SizeInBytes = sizeof(l_psoStream);

	auto& l_pipelineState = GetMutableREFPipelineState();

	// パイプラインステートを実際に作成する
	// CreatePipelineState(パイプライン設定ストリーム、
	//					   受け取りたいCOMインターフェース型のID、
	//					   作成結果のポインタを書き込むアドレス);
	auto l_hr = l_device->CreatePipelineState(&l_streamDesc, IID_PPV_ARGS(l_pipelineState.ReleaseAndGetAddressOf()));

	FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "パイプラインステート作成処理に失敗しました。", false);

	return true;
}

nlohmann::json FWK::Graphics::MeshShaderPipelineState::Serialize() const
{
	auto l_rootJson = GraphicsPipelineStateBase::Serialize();

	Utility::UpdateJson(l_rootJson, m_jsonConverter.Serialize(*this));

	return l_rootJson;
}