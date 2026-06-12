#include "SpriteScreenPassDrawRequest.h"

bool FWK::Graphics::SpriteScreenPassDrawRequest::SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource)
{
	SetupConstantBuffer<SpriteScreenPassConstantBufferUploader>(a_rootSignature,
																a_directCommandList,
																a_frameResource,
																GetREFConstantBuffer(),
																Enum::RootParameterType::CBSpritePass);

	return true;
}