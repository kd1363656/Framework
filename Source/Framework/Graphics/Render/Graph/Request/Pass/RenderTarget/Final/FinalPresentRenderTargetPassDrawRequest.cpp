#include "FinalPresentRenderTargetPassDrawRequest.h"

bool FWK::Graphics::FinalPresentRenderTargetPassDrawRequest::SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(GetREFConstantBuffer().m_sceneColorTextureSRVIndex == Constant::k_invalidDescriptorIndex, "SceneColorのSRVDescriptorIndexが無効のため、FoinalPresentPass用定数バッファの設定に失敗しました。", false);

	SetupConstantBuffer<FinalPresentRenderTargetPassConstantBufferUploader>(a_rootSignature,
																		    a_directCommandList,
																		    a_frameResource,
																		    GetREFConstantBuffer(),
																		    Enum::RootParameterType::CBFinalPresentPass);

	return true;
}