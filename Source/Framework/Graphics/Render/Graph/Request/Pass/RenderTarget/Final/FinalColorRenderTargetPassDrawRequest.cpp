#include "FinalColorRenderTargetPassDrawRequest.h"

bool FWK::Graphics::FinalColorRenderTargetPassDrawRequest::SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource)
{
	// FinalColorPassでは、SceneColorをSRVとして読んでFinalColorへ書き込む
	const auto& l_sceneColorRenderTargetPassTexture = FetchVALRenderTargetPassTexture(a_frameResource).lock();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_sceneColorRenderTargetPassTexture, "SceneColorのRenderTargetPassTextureが無効のため、FinalColorPass定数の設定に失敗しました。", false);

	const auto& l_renderTargetTexture      = l_sceneColorRenderTargetPassTexture->GetREFRenderTargetTexture();
		  auto& l_finalColorConstantBuffer = GetMutableREFConstantBuffer								   ();

	// 定数バッファにファイナルカラーテクスチャのSRVDescriptorIndexを渡す
	l_finalColorConstantBuffer.m_sceneColorTextureSRVIndex = l_renderTargetTexture.GetVALSRVDescriptorIndex();

	// 定数バッファをセット
	SetupConstantBuffer<FinalColorRenderTargetPassConstantBufferUploader>(a_rootSignature,
																		  a_directCommandList,
																		  a_frameResource,
																		  l_finalColorConstantBuffer,
																		  Enum::RootParameterType::CBFinalColorPass);

	return true;
}