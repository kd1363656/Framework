#include "PipelineStateJsonConverter.h"

void FWK::Converter::PipelineStateJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::PipelineState& a_pipelineState) const
{
	if (a_rootJson.is_null()) { return; }

	// アンプリフィケーションシェーダーのデシリアライズ
	if (a_rootJson.contains(k_amplificationShaderJsonKey))
	{
		auto& l_amplificationShader = a_pipelineState.GetMutableREFAmplificationShader();

		DeserializeOptionalShader(a_rootJson[k_amplificationShaderJsonKey], l_amplificationShader);
	}

	// メッシュシェーダーのデシリアライズ
	if (a_rootJson.contains(k_meshShaderJsonKey))
	{
		auto& l_meshShader = a_pipelineState.GetMutableREFMeshShader();

		l_meshShader.Deserialize(a_rootJson[k_meshShaderJsonKey]);
	}

	// ピクセルシェーダーのデシリアライズ
	if (a_rootJson.contains(k_pixelShaderJsonKey))
	{
		auto& l_pixelShader = a_pipelineState.GetMutableREFPixelShader();

		DeserializeOptionalShader(a_rootJson[k_pixelShaderJsonKey], l_pixelShader);
	}

	// ラスタライザーデスクのデシリアライズ
	if (a_rootJson.contains(k_rasterizerDescJsonKey))
	{
		DeserializeRasterizerDesc(a_rootJson[k_rasterizerDescJsonKey], a_pipelineState);
	}

	// ブレンドのデシリアライズ
	if (a_rootJson.contains(k_blendDescJsonKey))
	{
		DeserializeBlendDesc(a_rootJson[k_blendDescJsonKey], a_pipelineState);
	}

	// 深度ステンシルのデシリアライズ
	if (a_rootJson.contains(k_depthStencilDescJsonKey))
	{
		DeserializeDepthStencilDesc(a_rootJson[k_depthStencilDescJsonKey], a_pipelineState);
	}

	// RTVフォーマットリストのデシリアライズ
	if (a_rootJson.contains(k_rtvFormatListJsonKey))
	{
		DeserializeRTVFormatList(a_rootJson[k_rtvFormatListJsonKey], a_pipelineState);
	}

	// DSVフォーマットののデシリアライズ
	if (a_rootJson.contains(k_dsvFormatJsonKey))
	{
		const auto l_dsvFormat = a_rootJson.value(k_dsvFormatJsonKey, DXGI_FORMAT_UNKNOWN);

		a_pipelineState.SetDSVFormat(l_dsvFormat);
	}

	// サンプルデスクのデシリアライズ
	if (a_rootJson.contains(k_sampleDescJsonKey))
	{
		DeserializeSampleDesc(a_rootJson[k_sampleDescJsonKey], a_pipelineState);
	}

	const auto l_useRootSignatureType  = a_rootJson.value(k_useRootSignatureTypeJsonKey,  Enum::RootSignatureType::Invalid);
	const auto l_primitiveTopologyType = a_rootJson.value(k_primitiveTopologyTypeJsonKey, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	const auto l_sampleMask			   = a_rootJson.value(k_sampleMaskJsonKey,		      UINT_MAX);

	a_pipelineState.SetUseRootSignatureType (l_useRootSignatureType);
	a_pipelineState.SetPrimitiveTopologyType(l_primitiveTopologyType);
	a_pipelineState.SetSampleMask		    (l_sampleMask);
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

	// ラスタライザーデスクシェーダーのシリアライズ
	l_rootJson[k_rasterizerDescJsonKey] = SerializeRasterizerDesc(a_pipelineState);

	// ブレンドデスクのシリアライズ
	l_rootJson[k_blendDescJsonKey] = SerializeBlendDesc(a_pipelineState);

	// 深度バッファデスクのシリアライズ
	l_rootJson[k_depthStencilDescJsonKey] = SerializeDepthStencilDesc(a_pipelineState);

	// RTVフォーマットリストのシリアライズ
	l_rootJson[k_rtvFormatListJsonKey] = SerializeRTVFormatList(a_pipelineState);

	// DSVフォーマトのシリアライズ
	l_rootJson[k_dsvFormatJsonKey] = a_pipelineState.GetVALDSVFormat();

	// サンプルデスクのシリアライズ
	l_rootJson[k_sampleDescJsonKey] = SerializeSampleDesc(a_pipelineState);

	// 使用するルートシグネチャタイプのをシリアライズ
	l_rootJson[k_useRootSignatureTypeJsonKey] = a_pipelineState.GetVALUseRootSignatureType  ();
	l_rootJson[k_primitiveTopologyTypeJsonKey] = a_pipelineState.GetVALPrimitiveTopologyType();
	l_rootJson[k_sampleMaskJsonKey]            = a_pipelineState.GetVALSampleMask           ();

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

void FWK::Converter::PipelineStateJsonConverter::DeserializeRasterizerDesc(const nlohmann::json & a_rootJson, Graphics::PipelineState & a_pipelineState) const
{
	if (a_rootJson.is_null()) { return; }

	D3D12_RASTERIZER_DESC l_rasterizerDesc = {};

	// FillMode : ポリゴンを塗りつぶすか、ワイヤーフレームで描くか
	l_rasterizerDesc.FillMode = a_rootJson.value(k_fillModeJsonKey, D3D12_FILL_MODE_SOLID);

	// CullMode : 前面 / 背面のどちらを描画しないか
	l_rasterizerDesc.CullMode = a_rootJson.value(k_cullModeJsonKey, D3D12_CULL_MODE_BACK);

	// FrontCounterClockwise : 反時計回りを前面扱いするか
	l_rasterizerDesc.FrontCounterClockwise = a_rootJson.value(k_frontCounterClockwiseJsonKey, FALSE);

	// DepthBias : 深度値に加える補正値
	l_rasterizerDesc.DepthBias = a_rootJson.value(k_depthBiasJsonKey, D3D12_DEFAULT_DEPTH_BIAS);

	// DepthBiasClamp : 深度バイアスの最大補正量
	l_rasterizerDesc.DepthBiasClamp = a_rootJson.value(k_depthBiasClampJsonKey, D3D12_DEFAULT_DEPTH_BIAS_CLAMP);

	// SlopeScaledDepthBias : 面の傾きに応じて増える深度バイアス係数
	l_rasterizerDesc.SlopeScaledDepthBias = a_rootJson.value(k_slopeScaledDepthBiasJsonKey, D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS);

	// DepthClipEnable : 深度範囲外のピクセルをクリップするか
	l_rasterizerDesc.DepthClipEnable = a_rootJson.value(k_depthClipEnableJsonKey, TRUE);

	// MultisampleEnable : MSAA関連のラスタライズ設定
	l_rasterizerDesc.MultisampleEnable = a_rootJson.value(k_multisampleEnableJsonKey, FALSE);

	// AntialiasedLineEnable : 線描画時のアンチエイリアスを有効にするか
	l_rasterizerDesc.AntialiasedLineEnable = a_rootJson.value(k_antialiasedLineEnableJsonKey, FALSE);

	// ForcedSampleCount : 強制サンプル数(通常は0)
	l_rasterizerDesc.ForcedSampleCount = a_rootJson.value(k_forcedSampleCountJsonKey, k_noForcedSampleCount);

	// ConservativeRaster : 保守的ラスタライズを使うか
	l_rasterizerDesc.ConservativeRaster = a_rootJson.value(k_conservativeRasterJsonKey, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);

	a_pipelineState.SetRasterizerDesc(l_rasterizerDesc);
}
void FWK::Converter::PipelineStateJsonConverter::DeserializeBlendDesc(const nlohmann::json & a_rootJson, Graphics::PipelineState & a_pipelineState) const
{
	if (a_rootJson.is_null()) { return; }

	D3D12_BLEND_DESC l_blendDesc = {};

	// AlphaToCoverageEnable : MSAA使用時にalpha値からサンプル被覆率を作るか
	l_blendDesc.AlphaToCoverageEnable = a_rootJson.value(k_alphaToCoverageEnableJsonKey, FALSE);

	// IndependentBlendEnable : 複数RenderTargetごとに別々のブレンド設定を使うか
	l_blendDesc.IndependentBlendEnable = a_rootJson.value(k_independentBlendEnableJsonKey, FALSE);

	// RenderTarget : 各RenderTargetごとのブレンド設定配列
	if (Utility::IsArray(a_rootJson, k_renderTargetJsonKey))
	{
		const auto& l_jsonArray = a_rootJson[k_renderTargetJsonKey];

		for (auto l_renderTargetIndex = 0U; l_renderTargetIndex < l_jsonArray.size(); ++l_renderTargetIndex)
		{
			if (l_renderTargetIndex >= D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT)
			{
				assert(false && "BlendDescのRenderTarget配列数がDirectX12の上限を超えています。");
				break;
			}

			const auto& l_json         = l_jsonArray			 [l_renderTargetIndex];
				  auto& l_renderTarget = l_blendDesc.RenderTarget[l_renderTargetIndex];

			// BlendEnable : ブレンド演算を有効にするか
			l_renderTarget.BlendEnable = l_json.value(k_blendEnableJsonKey, FALSE);

			// LogicOpEnable : 論理演算を有効にするか
			l_renderTarget.LogicOpEnable = l_json.value(k_logicOPEnableJsonKey, FALSE);

			// SrcBlend : カラー計算時のソース係数
			l_renderTarget.SrcBlend = l_json.value(k_srcBlendJsonKey, D3D12_BLEND_ONE);

			// DestBlend : カラー計算時のデスティネーション係数
			l_renderTarget.DestBlend = l_json.value(k_destBlendJsonKey, D3D12_BLEND_ZERO);

			// BlendOp : カラー計算時のブレンド演算
			l_renderTarget.BlendOp = l_json.value(k_blendOPJsonKey, D3D12_BLEND_OP_ADD);

			// SrcBlendAlpha : alpha計算時のデスティネーション係数
			l_renderTarget.SrcBlendAlpha = l_json.value(k_srcBlendAlphaJsonKey, D3D12_BLEND_ONE);

			// DestBlendAlpha : alpha計算時のデスティネーション係数
			l_renderTarget.DestBlendAlpha = l_json.value(k_destBlendAlphaJsonKey, D3D12_BLEND_ZERO);

			// BlendOpAlpha : alpha計算時のブレンド演算
			l_renderTarget.BlendOpAlpha = l_json.value(k_blendOPAlphaJsonKey, D3D12_BLEND_OP_ADD);

			// LogicOp : 論理演算の種類
			l_renderTarget.LogicOp = l_json.value(k_logicOPJsonKey, D3D12_LOGIC_OP_NOOP);

			// RenderTargetWriteMask : 書き込み対象RGBAチャンネルのビットマスク
			l_renderTarget.RenderTargetWriteMask = l_json.value(k_renderTargetWriteMaskJsonKey, static_cast<UINT8>(D3D12_COLOR_WRITE_ENABLE_ALL));
		}
	}

	a_pipelineState.SetBlendDesc(l_blendDesc);
}
void FWK::Converter::PipelineStateJsonConverter::DeserializeDepthStencilDesc(const nlohmann::json & a_rootJson, Graphics::PipelineState & a_pipelineState) const
{
	if (a_rootJson.is_null()) { return; }

	D3D12_DEPTH_STENCIL_DESC l_depthStencilDesc = {};

	// DepthEnable : 深度テストを有効にするか
	l_depthStencilDesc.DepthEnable = a_rootJson.value(k_depthEnableJsonKey, FALSE);

	// DepthWriteMask : 深度バッファへ書き込むか
	l_depthStencilDesc.DepthWriteMask = a_rootJson.value(k_depthWriteMaskJsonKey, D3D12_DEPTH_WRITE_MASK_ALL);

	// DepthFunc : 深度比較方法
	l_depthStencilDesc.DepthFunc = a_rootJson.value(k_depthFUNCJsonKey, D3D12_COMPARISON_FUNC_LESS);

	// StencilEnable : ステンシルテストを有効にするか
	l_depthStencilDesc.StencilEnable = a_rootJson.value(k_stencilEnableJsonKey, FALSE);

	// StencilReadMask : ステンシル値を読むときのビットマスク
	l_depthStencilDesc.StencilReadMask = a_rootJson.value(k_stencilReadMaskJsonKey, static_cast<UINT8>(D3D12_DEFAULT_STENCIL_READ_MASK));

	// StencilWriteMask : ステンシル値を書き込むときのビットマスク
	l_depthStencilDesc.StencilWriteMask = a_rootJson.value(k_stencilWriteMaskJsonKey, static_cast<UINT8>(D3D12_DEFAULT_STENCIL_WRITE_MASK));

	// FrontFace : 前面ポリゴンに対するステンシル動作
	if (a_rootJson.contains(k_frontFaceJsonKey))
	{
		DeserializeDepthStencilOpDesc(a_rootJson[k_frontFaceJsonKey], l_depthStencilDesc.FrontFace);
	}
	else
	{
		l_depthStencilDesc.FrontFace.StencilFailOp      = D3D12_STENCIL_OP_KEEP;
		l_depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		l_depthStencilDesc.FrontFace.StencilPassOp      = D3D12_STENCIL_OP_KEEP;
		l_depthStencilDesc.FrontFace.StencilFunc        = D3D12_COMPARISON_FUNC_ALWAYS;
	}

	// BackFace : 背面ポリゴンに対するステンシル動作
	if (a_rootJson.contains(k_backFaceJsonKey))
	{
		DeserializeDepthStencilOpDesc(a_rootJson[k_backFaceJsonKey], l_depthStencilDesc.BackFace);
	}
	else
	{
		l_depthStencilDesc.BackFace = l_depthStencilDesc.FrontFace;
	}

	a_pipelineState.SetDepthStencilDesc(l_depthStencilDesc);
}
void FWK::Converter::PipelineStateJsonConverter::DeserializeRTVFormatList(const nlohmann::json & a_rootJson, Graphics::PipelineState & a_pipelineState) const
{
	if (a_rootJson.is_null())		   { return; }
	if (!Utility::IsArray(a_rootJson)) { return; }

	// レンダーターゲット数がDirectX12の上限を超えていないかどうかを確認
	FWK_ASSERT_RETURN_IF_FAILED(a_rootJson.size() > D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT, "RTVFormatListの要素数がDirectX12のRenderTarget上限を超えおり、RTVFormatListのデシリアライズ処理に失敗しました。");

	for (const auto& l_json : a_rootJson)
	{
		a_pipelineState.AddRTVFormat(l_json.get<DXGI_FORMAT>());
	}
}
void FWK::Converter::PipelineStateJsonConverter::DeserializeSampleDesc(const nlohmann::json & a_rootJson, Graphics::PipelineState & a_pipelineState) const
{
	if (a_rootJson.is_null()) { return; }

	DXGI_SAMPLE_DESC l_sampleDesc = {};

	// MSAAのサンプル数。通常描画の既定値は1。
	l_sampleDesc.Count = a_rootJson.value(k_countJsonKey, Constant::k_defaultSampleCount);

	// サンプル品質レベル通常の既定値は0。
	l_sampleDesc.Quality = a_rootJson.value(k_qualityJsonKey, Constant::k_defaultSampleQuality);

	a_pipelineState.SetSampleDesc(l_sampleDesc);
}

nlohmann::json FWK::Converter::PipelineStateJsonConverter::SerializeRasterizerDesc(const Graphics::PipelineState & a_pipelineState) const
{
		  nlohmann::json l_rootJson	      = {};
	const auto&          l_rasterizerDesc = a_pipelineState.GetREFRasterizerDesc();

	// FillMode : ポリゴンを塗りつぶすか、ワイヤーフレームで描くか
	l_rootJson[k_fillModeJsonKey] = l_rasterizerDesc.FillMode;

	// CullMode : 前面 / 背面のどちらを描画しないか
	l_rootJson[k_cullModeJsonKey] = l_rasterizerDesc.CullMode;

	// FrontCounterClockwise : 反時計回りを前面扱いするか
	l_rootJson[k_frontCounterClockwiseJsonKey] = l_rasterizerDesc.FrontCounterClockwise;

	// DepthBias : 深度値に加える補正値
	l_rootJson[k_depthBiasJsonKey] = l_rasterizerDesc.DepthBias;

	// DepthBiasClamp : 深度バイアスの最大補正量
	l_rootJson[k_depthBiasClampJsonKey] = l_rasterizerDesc.DepthBiasClamp;

	// SlopeScaledDepthBias : 面の傾きに応じて増える深度バイアス係数
	l_rootJson[k_slopeScaledDepthBiasJsonKey] = l_rasterizerDesc.SlopeScaledDepthBias;

	// DepthClipEnable : 深度範囲外のピクセルをクリップするか
	l_rootJson[k_depthClipEnableJsonKey] = l_rasterizerDesc.DepthClipEnable;

	// MultisampleEnable : MSAA関連のラスタライズ設定
	l_rootJson[k_multisampleEnableJsonKey] = l_rasterizerDesc.MultisampleEnable;

	// AntialiasedLineEnable : 線描画時のアンチエイリアスを有効にするか
	l_rootJson[k_antialiasedLineEnableJsonKey] = l_rasterizerDesc.AntialiasedLineEnable;

	// ForcedSampleCount : 強制サンプル数(通常は0)
	l_rootJson[k_forcedSampleCountJsonKey] = l_rasterizerDesc.ForcedSampleCount;

	// ConservativeRaster : 保守的ラスタライズを使うか
	l_rootJson[k_conservativeRasterJsonKey] = l_rasterizerDesc.ConservativeRaster;

	return l_rootJson;
}
nlohmann::json FWK::Converter::PipelineStateJsonConverter::SerializeBlendDesc(const Graphics::PipelineState& a_pipelineState) const
{
		  nlohmann::json l_rootJson  = {};
	const auto&			 l_blendDesc = a_pipelineState.GetREFBlendDesc();

	// AlphaToCoverageEnable : MSAA使用時にalpha値からサンプル被覆率を作るか
	l_rootJson[k_alphaToCoverageEnableJsonKey] = l_blendDesc.AlphaToCoverageEnable;

	// IndependentBlendEnable : 複数RenderTargetごとに別々のブレンド設定を使うか
	l_rootJson[k_independentBlendEnableJsonKey] = l_blendDesc.IndependentBlendEnable;

	auto l_jsonArray = nlohmann::json::array();

	for (const auto& l_renderTarget : l_blendDesc.RenderTarget)
	{
		auto l_json = nlohmann::json();

		// BlendEnable : ブレンド演算を有効にするか
		l_json[k_blendEnableJsonKey] = l_renderTarget.BlendEnable;

		// LogicOpEnable : 論理演算を有効にするか
		l_json[k_logicOPEnableJsonKey] = l_renderTarget.LogicOpEnable;

		// SrcBlend : カラー計算時のソース係数
		l_json[k_srcBlendJsonKey] = l_renderTarget.SrcBlend;

		// DestBlend : カラー計算時のデスティネーション係数
		l_json[k_destBlendJsonKey] = l_renderTarget.DestBlend;

		// BlendOp : カラー計算時のブレンド演算
		l_json[k_blendOPJsonKey] = l_renderTarget.BlendOp;

		// SrcBlendAlpha : alpha計算時のソース係数
		l_json[k_srcBlendAlphaJsonKey] = l_renderTarget.SrcBlendAlpha;

		// DestBlendAlpha : alpha計算時のソース係数
		l_json[k_destBlendAlphaJsonKey] = l_renderTarget.DestBlendAlpha;

		// BlendOpAlpha : alpha計算時のブレンド演算
		l_json[k_blendOPAlphaJsonKey] = l_renderTarget.BlendOpAlpha;

		// LogicOp : 論理演算の種類
		l_json[k_logicOPJsonKey] = l_renderTarget.LogicOp;

		// RenderTargetWriteMask : 書き込み対象RGBAチャンネルのビットマスク
		l_json[k_renderTargetWriteMaskJsonKey] = l_renderTarget.RenderTargetWriteMask;

		l_jsonArray.emplace_back(l_json);
	}

	l_rootJson[k_renderTargetJsonKey] = l_jsonArray;

	return l_rootJson;
}
nlohmann::json FWK::Converter::PipelineStateJsonConverter::SerializeDepthStencilDesc(const Graphics::PipelineState& a_pipelineState) const
{
		  nlohmann::json l_rootJson	        = {};
	const auto&			 l_depthStencilDesc = a_pipelineState.GetREFDepthStencilDesc();

	// DepthEnable : 深度テストを有効にするか
	l_rootJson[k_depthEnableJsonKey] = l_depthStencilDesc.DepthEnable;

	// DepthWriteMask : 深度バッファへ書き込むか
	l_rootJson[k_depthWriteMaskJsonKey] = l_depthStencilDesc.DepthWriteMask;

	// DepthFunc : 深度比較方法
	l_rootJson[k_depthFUNCJsonKey] = l_depthStencilDesc.DepthFunc;

	// StencilEnable : ステンシルテストを有効にするか
	l_rootJson[k_stencilEnableJsonKey] = l_depthStencilDesc.StencilEnable;

	// StencilReadMask : ステンシル値を読むときのビットマスク
	l_rootJson[k_stencilReadMaskJsonKey] = l_depthStencilDesc.StencilReadMask;
	
	// StencilWriteMask : ステンシル値を書き込むときのビットマスク
	l_rootJson[k_stencilWriteMaskJsonKey] = l_depthStencilDesc.StencilWriteMask;

	// FrontFace : 前面ポリゴンに対するステンシル動作
	l_rootJson[k_frontFaceJsonKey] = SerializeDepthStencilOPDesc(l_depthStencilDesc.FrontFace);

	// BackFace : 背面ポリゴンに対するステンシル動作
	l_rootJson[k_backFaceJsonKey] = SerializeDepthStencilOPDesc(l_depthStencilDesc.BackFace);

	return l_rootJson;
}
nlohmann::json FWK::Converter::PipelineStateJsonConverter::SerializeDepthStencilOPDesc(const D3D12_DEPTH_STENCILOP_DESC& a_depthStencilOPDesc) const
{
	nlohmann::json l_rootJson = {};

	// StencilFailOp : ステンシルテスト失敗時の動作
	l_rootJson[k_stencilFailOpJsonKey] = a_depthStencilOPDesc.StencilFailOp;

	// StencilDepthFail : ステンシル成功かつ深度失敗時の動作
	l_rootJson[k_stencilDepthFailOpJsonKey] = a_depthStencilOPDesc.StencilDepthFailOp;

	// StencilPassOp : ステンシル成功かつ深度成功時の動作
	l_rootJson[k_stencilPassOpJsonKey] = a_depthStencilOPDesc.StencilPassOp;

	// StencilFunc : ステンシル比較方法
	l_rootJson[k_stencilFUNCJsonKey] = a_depthStencilOPDesc.StencilFunc;

	return l_rootJson;
}
nlohmann::json FWK::Converter::PipelineStateJsonConverter::SerializeRTVFormatList(const Graphics::PipelineState& a_pipelineState) const
{
		  auto  l_rootJsonArray = nlohmann::json::array				 ();
	const auto& l_rtvFormatList = a_pipelineState.GetREFRTVFormatList();

	for (const auto& l_rtvFormat : l_rtvFormatList)
	{
		l_rootJsonArray.emplace_back(l_rtvFormat);
	}

	return l_rootJsonArray;
}
nlohmann::json FWK::Converter::PipelineStateJsonConverter::SerializeSampleDesc(const Graphics::PipelineState& a_pipelineState) const
{
	nlohmann::json l_rootJson   = {};
	const auto&	   l_sampleDesc = a_pipelineState.GetREFSampleDesc();

	l_rootJson[k_countJsonKey]   = l_sampleDesc.Count;
	l_rootJson[k_qualityJsonKey] = l_sampleDesc.Quality;
	
	return l_rootJson;
}

void FWK::Converter::PipelineStateJsonConverter::DeserializeDepthStencilOpDesc(const nlohmann::json& a_rootJson, D3D12_DEPTH_STENCILOP_DESC& a_depthStencilOPDesc) const
{
	if (a_rootJson.is_null())
	{
		a_depthStencilOPDesc.StencilFailOp      = D3D12_STENCIL_OP_KEEP;
		a_depthStencilOPDesc.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		a_depthStencilOPDesc.StencilPassOp      = D3D12_STENCIL_OP_KEEP;
		a_depthStencilOPDesc.StencilFunc        = D3D12_COMPARISON_FUNC_ALWAYS;
		return;
	}
	
	// StencilFailOp : ステンシルテスト失敗時の動作
	a_depthStencilOPDesc.StencilFailOp = a_rootJson.value(k_stencilFailOpJsonKey, D3D12_STENCIL_OP_KEEP);

	// StencilDepthFailOp : ステンシル成功かつ深度失敗時の動作
	a_depthStencilOPDesc.StencilDepthFailOp = a_rootJson.value(k_stencilDepthFailOpJsonKey, D3D12_STENCIL_OP_KEEP);

	// StencilPassOp : ステンシル成功かつ深度成功時の動作
	a_depthStencilOPDesc.StencilPassOp = a_rootJson.value(k_stencilPassOpJsonKey, D3D12_STENCIL_OP_KEEP);

	// StencilFunc : ステンシル比較方法
	a_depthStencilOPDesc.StencilFunc = a_rootJson.value(k_stencilFUNCJsonKey, D3D12_COMPARISON_FUNC_ALWAYS);
}
void FWK::Converter::PipelineStateJsonConverter::EnsureShader(std::shared_ptr<Graphics::Shader>&a_shader) const
{
	// 既にインスタンス化されていたら"return"
	if (a_shader) { return; }
	a_shader = std::make_shared<Graphics::Shader>();
}