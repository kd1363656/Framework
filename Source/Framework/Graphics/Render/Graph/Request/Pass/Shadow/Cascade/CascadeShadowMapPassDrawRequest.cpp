#include "CascadeShadowMapPassDrawRequest.h"

bool FWK::Graphics::CascadeShadowMapPassDrawRequest::SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource)
{
	// カスケードシャドウマップクラスから定数バッファを取得
	const auto& l_graphicsManager  = GraphicsManager::GetInstance          ();
	const auto& l_renderer         = l_graphicsManager.GetREFRenderer      ();
	const auto& l_shadowContext    = l_renderer.GetREFShadowContext        (); 
	const auto& l_cascadeShadowMap = l_shadowContext.GetREFCascadeShadowMap(); 

	SetupConstantBuffer<CascadeShadowMapPassDynamicConstantBufferUploader>(l_cascadeShadowMap.GetREFCBCascadeShadowMapPass(),
																           a_rootSignature,
																           a_directCommandList,
																           a_frameResource,
																           Enum::RootParameterType::CBCascadeShadowMapPass);

	return true;
}