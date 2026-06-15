#include "FinalPresentRenderTargetPassDrawRequest.h"

bool FWK::Graphics::FinalPresentRenderTargetPassDrawRequest::SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource)
{
	// FinalPresentPassでは、FinalColorPassで作成された最終表示用テクスチャを読む
	const auto& l_finalColorRenderTargetPassTexture = FetchVALRenderTargetPassTexture(a_frameResource).lock();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_finalColorRenderTargetPassTexture, "ファイナルカラーレンダーターゲットパステクスチャが無効になっており、Pass定数の設定に失敗しました。", {});

	const auto& l_renderTargetTexture        = l_finalColorRenderTargetPassTexture->GetREFRenderTargetTexture();
		  auto& l_finalPresentConstantBuffer = GetMutableREFConstantBuffer									 ();

	// 定数バッファにファイナルカラーテクスチャのSRVDescriptorIndexを渡す
	l_finalPresentConstantBuffer.m_finalColorTextureSRVIndex = l_renderTargetTexture.GetVALSRVDescriptorIndex();

	SetupConstantBuffer<FinalPresentRenderTargetPassConstantBufferUploader>(a_rootSignature,
																		    a_directCommandList,
																		    a_frameResource,
																		    l_finalPresentConstantBuffer,
																			Enum::RootParameterType::CBFinalPresentPass);

	return true;
}