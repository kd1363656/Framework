#include "FinalPresentRenderTargetPassDrawRequest.h"

bool FWK::Graphics::FinalPresentRenderTargetPassDrawRequest::SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!EnsureCurrentRenderTargetPassTexture(a_frameResource), "SceneColorRenderTargetPassTextureのポインタが空になっており、FinalPresentPass用定数バッファの設定に失敗しました。", false);


	return true;
}