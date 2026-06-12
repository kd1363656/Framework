#include "SpritePassDrawRequest.h"

bool FWK::Graphics::SpritePassDrawRequest::SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource)
{
	SetupConstantBuffer<SpritePassConstantBufferUploader>(a_rootSignature,
						a_directCommandList,
						a_frameResource,
						GetREFConstantBuffer(),
						Enum::RootParameterType::CBSpritePass);

	return true;
}