#include "CascadeShadowPass.h"

FWK::Graphics::CascadeShadowPass::CascadeShadowPass()
{
	// SkeletalAnimation計算完了後
	// 通常のModel描画より前にShadowMap作成する
	SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::Shadow);

	// CascadeShadowMapへDepthを書き込むため
	// Pass実行前にResource全体をDEPTH_WRITEへ遷移する
	WriteShadowMap(Enum::RenderGraphShadowMapType::Cascade, Enum::RenderGraphResourceUsage::DepthWrite);
}
FWK::Graphics::CascadeShadowPass::~CascadeShadowPass() = default;

void FWK::Graphics::CascadeShadowPass::Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer, RenderGraph&)
{
	const auto& l_dsvDescriptorPool = a_resourceContext.GetREFDSVDescriptorPool    ();
	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList           ();
	      auto& l_shadowContext     = a_renderer.GetMutableREFShadowContext        ();
	      auto& l_cascadeShadowMap  = l_shadowContext.GetMutableREFCascadeShadowMap();

	// 登録されているCamera CBとLight CBから、
	// このFrameで使用するCascade行列を計算する。
	FWK_ASSERT_RETURN_IF(!l_cascadeShadowMap.Update(), "CascadeShadowMapの更新処理に失敗したため、CascadeShadowPassの実行に失敗しました。");

	const auto& l_renderArea                  = l_cascadeShadowMap.GetREFRenderArea                 ();
	const auto& l_depthStencilTextureSettings = l_cascadeShadowMap.GetREFDepthStencilTextureSettings();
	
	// ShadowMapの解像度に対応したViewportと
	// ScissorRECTをDirectCommandListへ設定する
	l_directCommandList.SetupRenderArea(l_renderArea);

	// Cascadeごとに異なるTexture2DArrayのSliceへ
	// DSVの設定とDepthClearを行う
	for (UINT l_cascadeIndex = k_firstCascadeIndex; l_cascadeIndex < l_depthStencilTextureSettings.m_arraySize; ++l_cascadeIndex)
	{
		const auto l_dsvDescriptorIndex = l_cascadeShadowMap.FetchVALCascadeDSVDescriptorIndex(l_cascadeIndex);

		FWK_ASSERT_RETURN_IF(l_dsvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "Cascadeに対応するDSVDescriptorIndexが無効のため、CascadeShadowPassの実行に失敗しました。");

		// ColorRenderTargetは使用せず
		// 現在のCascadeに対応するDSVだけをOutputMergerへ設定する
		l_directCommandList.SetupDepthStencil(l_dsvDescriptorPool, l_dsvDescriptorIndex);

		// 現在のCascadeのDepthを初期値へ戻す
		l_directCommandList.ClearDepthStencil(l_dsvDescriptorPool,
			                                  l_depthStencilTextureSettings.m_depthClearValue,
			                                  l_dsvDescriptorIndex,
			                                  l_depthStencilTextureSettings.m_stencilClearValue);
	}
}