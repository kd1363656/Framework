#include "FinalPresentRenderTargetPassDrawRequest.h"

bool FWK::Graphics::FinalPresentRenderTargetPassDrawRequest::SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource)
{
	const auto& l_sceneColorRenderTargetPassTexture = FetchVALRenderTargetPassTexture(a_frameResource).lock();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_sceneColorRenderTargetPassTexture, "シーンカラーレンダーターゲットパステクスチャが無効になっており、Pass定数の設定に失敗しました。", {});

	const auto& l_renderTargetTexture        = l_sceneColorRenderTargetPassTexture->GetREFRenderTargetTexture();
		  auto& l_finalPresentConstantBuffer = GetMutableREFConstantBuffer									 ();

	// SRVインデックスを更新
	l_finalPresentConstantBuffer.m_sceneColorTextureSRVIndex = l_renderTargetTexture.GetVALSRVDescriptorIndex();

	SetupConstantBuffer<FinalPresentRenderTargetPassConstantBufferUploader>(a_rootSignature,
																		    a_directCommandList,
																		    a_frameResource,
																		    l_finalPresentConstantBuffer,
																			Enum::RootParameterType::CBFinalPresentPass);

	return true;
}