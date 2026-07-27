#include "ModelCascadeShadowPass.h"

FWK::Graphics::ModelCascadeShadowPass::ModelCascadeShadowPass()
{
	// SkeletalAnimation計算完了後
	// 通常のModel描画より前にShadowMap作成する
	SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::Shadow);

	// CascadeShadowMapへDepthを書き込むため
	// Pass実行前にResource全体をDEPTH_WRITEへ遷移する
	WriteShadowMap(Enum::RenderGraphShadowMapType::Cascade, Enum::RenderGraphResourceUsage::DepthWrite);
}
FWK::Graphics::ModelCascadeShadowPass::~ModelCascadeShadowPass() = default;

void FWK::Graphics::ModelCascadeShadowPass::Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer, RenderGraph& a_renderGraph)
{
	const auto& l_dsvDescriptorPool    = a_resourceContext.GetREFDSVDescriptorPool();
	const auto& l_directCommandList    = a_renderer.GetREFDirectCommandList       ();
	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource    ().lock();

	FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "現在のFrameResourceを取得できないため、ModelCascadeShadowPassを実行できません。");

	// CascadeごとのPassConstantBufferを書き込むUploader
	const auto& l_modelCascadeShadowPassDynamicConstantBufferUploader = l_currentFrameResource->FindPTRDynamicBufferUploader<ModelCascadeShadowPassDynamicConstantBufferUploader>().lock();

	FWK_ASSERT_RETURN_IF(!l_modelCascadeShadowPassDynamicConstantBufferUploader, "ModelCascadeShadowPassDynamicConstantBufferUploaderを取得できないため、ModelCascadeShadowPassを実行できません。");

	// 通常描画で登録されているModelの描画申請を
	// Shadow描画でも再利用する
	const auto& l_staticModelStandardUnLitPerObjectDrawRequest            = a_renderGraph.FindVALDrawRequestPerObject<StaticModelStandardUnLitPerObjectDrawRequest>           ().lock();
	const auto& l_staticModelStandardLitPerObjectDrawRequest              = a_renderGraph.FindVALDrawRequestPerObject<StaticModelStandardLitPerObjectDrawRequest>             ().lock();
	const auto& l_skeletalAnimationModelStandardUnLitPerObjectDrawRequest = a_renderGraph.FindVALDrawRequestPerObject<SkeletalAnimationModelStandardUnLitPerObjectDrawRequest>().lock();
	const auto& l_skeletalAnimationModelStandardLitPerObjectDrawRequest   = a_renderGraph.FindVALDrawRequestPerObject<SkeletalAnimationModelStandardLitPerObjectDrawRequest>  ().lock();

	FWK_ASSERT_RETURN_IF(!l_staticModelStandardUnLitPerObjectDrawRequest,            "StaticModelStandardUnLitPerObjectDrawRequestを取得できないため、ModelCascadeShadowPassを実行できません。");
	FWK_ASSERT_RETURN_IF(!l_staticModelStandardLitPerObjectDrawRequest,              "StaticModelStandardLitPerObjectDrawRequestを取得できないため、ModelCascadeShadowPassを実行できません。");
	FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelStandardUnLitPerObjectDrawRequest, "SkeletalAnimationModelStandardUnLitPerObjectDrawRequestを取得できないため、ModelCascadeShadowPassを実行できません。");
	FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelStandardLitPerObjectDrawRequest,   "SkeletalAnimationModelStandardLitPerObjectDrawRequestを取得できないため、ModelCascadeShadowPassを実行できません。");

	auto& l_shadowContext    = a_renderer.GetMutableREFShadowContext        ();
	auto& l_cascadeShadowMap = l_shadowContext.GetMutableREFCascadeShadowMap();

	// 現在のCameraとDirectionalLightから
	// Cascadeごとの行列とAmplicficationShader用カリング範囲を更新する
	FWK_ASSERT_RETURN_IF(!l_cascadeShadowMap.Update(), "CascadeShadowMapを更新できないため、ModelCascadeShadowPassを実行できません。");

	const auto& l_renderArea                  = l_cascadeShadowMap.GetREFRenderArea                 ();
	const auto& l_depthStencilTextureSettings = l_cascadeShadowMap.GetREFDepthStencilTextureSettings();

	// ShadowMapの解像度に対応する
	// ViewportとScissorRectangleを設定する
	l_directCommandList.SetupRenderArea(l_renderArea);

	// CascadeごとにTexture2DArrayの異なるSliceへ
	// StaticModelとSkeletalAnimationModelのDepthを書き込む
	for (UINT l_cascadeIndex = 0U; l_cascadeIndex < l_depthStencilTextureSettings.m_arraySize; ++l_cascadeIndex)
	{
		const auto l_dsvDescriptorIndex = l_cascadeShadowMap.FetchVALCascadeDSVDescriptorIndex(l_cascadeIndex);

		FWK_ASSERT_RETURN_IF(l_dsvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "Cascadeに知王するDSVDescriptorIndexが無効なため、ModelCascadeShadowPassを実行できませんでした。");

		// 現在Cascadeに対応するTexture2DArraySliceを、Depth出力先として設定する
		l_directCommandList.SetupDepthStencil(l_dsvDescriptorPool, l_dsvDescriptorIndex);

		// 前FrameのDepthを残さないように、現在のCascadeSliceをClearする
		l_directCommandList.ClearDepthStencil(l_dsvDescriptorPool,
			                                  l_depthStencilTextureSettings.m_depthClearValue,
			                                  l_dsvDescriptorIndex,
			                                  l_depthStencilTextureSettings.m_stencilClearValue);

		// CascadeShadowMapが所有しているConstantBufferデータを、
		// 一時保持せずconstポインタで直接参照する
		const auto* const l_cbModelCascadeShadowPass = l_cascadeShadowMap.FetchPTRModelCascadeShadowPass(l_cascadeIndex);

		FWK_ASSERT_RETURN_IF(!l_cbModelCascadeShadowPass, "CBModelCascadeShadowPassを取得できないため、ModelCascadeShadowPassを実行できません。");

		// Uploader::Write内部でUploadBufferへmemcpyされる
		const auto& l_gpuVirtualAddress = l_modelCascadeShadowPassDynamicConstantBufferUploader->Write(*l_cbModelCascadeShadowPass);

		FWK_ASSERT_RETURN_IF(l_gpuVirtualAddress == DynamicBufferUploaderBase::k_invalidGPUVirtualAddress, "CBModelCascadeShadowPassをUploadBufferへ書き込めないため、ModelCascadeShadowPassを実行できません。");


	}
}