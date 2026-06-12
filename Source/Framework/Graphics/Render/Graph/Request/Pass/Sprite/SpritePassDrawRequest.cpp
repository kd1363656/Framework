#include "SpritePassDrawRequest.h"

bool FWK::Graphics::SpritePassDrawRequest::SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource)
{
	const bool l_isSuccess = SetupConstantBuffer<SpritePassConstantBufferUploader>(a_rootSignature,
																				   a_directCommandList,
																				   a_frameResource,
																				   GetREFConstantBuffer(),
																				   Enum::RootParameterType::CBSpritePass);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_isSuccess, "スプライト共通定数バッファの設定に失敗しました。。", false);

	return true;
}