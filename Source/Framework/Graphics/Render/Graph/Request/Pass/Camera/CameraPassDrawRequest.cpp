#include "CameraPassDrawRequest.h"

bool FWK::Graphics::CameraPassDrawRequest::SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource)
{
	SetupConstantBuffer<CameraPassConstantBufferUploader>(GetREFConstantBuffer(), 
														  a_rootSignature,
														  a_directCommandList,
														  a_frameResource,
														  Enum::RootParameterType::CBCameraPass);

	return true;
}