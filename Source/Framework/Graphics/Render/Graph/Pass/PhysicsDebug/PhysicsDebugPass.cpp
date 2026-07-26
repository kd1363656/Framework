#include "PhysicsDebugPass.h"

FWK::Graphics::PhysicsDebugPass::PhysicsDebugPass()
{
	SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::PhysicsDebug);

	// FinalColorPassでガンマ補正済みになったFinalColorへ、
	// デバッグ描画を直接書き込む。
	// ここではデバッグ色に追加のガンマ補正を掛けない。
	WriteRenderTarget(Enum::RenderGraphRenderTargetType::FinalColor, Enum::RenderGraphResourceUsage::RenderTarget);

	// SceneDepthをDSVとして一緒にセットする
	// RenderGraph上はWriteDepthStencilだが、PSO側でDepthWriteMaskをZeroにすれば、
	// 実際には深度を書き込まず、深度テストだけにできる
	WriteDepthStencil(Enum::RenderGraphDepthStencilType::SceneDepth, Enum::RenderGraphResourceUsage::DepthWrite);
}
FWK::Graphics::PhysicsDebugPass::~PhysicsDebugPass() = default;

void FWK::Graphics::PhysicsDebugPass::Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph& a_renderGraph)
{
	const auto& l_physicsManager = Physics::PhysicsManager::GetInstance();

	// 描画しないならreturn;
	if (l_physicsManager.GetVALIsDisableDebugDraw()) { return; }

	const auto& l_physicsDebugDrawRenderer = l_physicsManager.GetVALDebugRenderer().lock();

	FWK_ASSERT_RETURN_IF(!l_physicsDebugDrawRenderer, "PhysicsDebugDrawRendererが生成されておらず、PhysicsDebugPassの実行に失敗しました。");

	const auto& l_physicsDebugRendererQueue = l_physicsDebugDrawRenderer->GetREFPhysicsDebugRendererQueue();

	if (!l_physicsDebugRendererQueue.HasLineVertex()) { return; }

	const auto& l_lineVertexList = l_physicsDebugRendererQueue.GetREFLineVertexList();

	FWK_ASSERT_RETURN_IF(l_lineVertexList.size() > static_cast<std::size_t>(std::numeric_limits<UINT>::max()), "PhysicsDebugのLineVertex数がDrawInstancedで描画できる上限を超えており、PhysicsDebugPassの実行に失敗しました。");

	// パイプラインステートをセット
	const auto& l_rootSignature = SetupGraphicsRenderPipeline(a_renderer, Enum::PipelineStateType::PhysicsDebug).lock();

	FWK_ASSERT_RETURN_IF(!l_rootSignature, "PhysicsDebugPass用RootSignatureが無効のため、PhysicsDebugPassの実行に失敗しました。");

	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "現在のFrameResourceが無効のため、PhysicsDebugPassの実行に失敗しました。");

	// カメラ定数書き込み用Requestクラスを取得
	const auto& l_cameraPassDrawRequest = a_renderGraph.FindVALDrawRequestPass<CameraPassDrawRequest>().lock();

	FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest, "CameraPassDrawRequestが無効のため、PhysicsDebugPassの実行に失敗しました。");

	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	FWK_ASSERT_RETURN_IF(!l_cameraPassDrawRequest->SetupPassConstantBuffer(*l_rootSignature, l_directCommandList, *l_currentFrameResource), "PhysicsDebugPassでCameraPass定数バッファの設定に失敗しており、PhysicsDebugPassの実行に失敗しました。");

	// 頂点バッファーアップローダーを取得
	const auto& l_physicsDebugVertexBufferUploader = l_currentFrameResource->FindPTRDynamicBufferUploader<PhysicsDebugDynamicVertexBufferUploader>().lock();

	FWK_ASSERT_RETURN_IF(!l_physicsDebugVertexBufferUploader, "PhysicsDebugDynamicVertexBufferUploaderが無効のため、PhysicsDebugPassの実行に失敗しました。");

	// 頂点リストの数がバッファーアップローダーの作成数を超えていないかどうかを確認する
	FWK_ASSERT_RETURN_IF(l_lineVertexList.size() > static_cast<std::size_t>(l_physicsDebugVertexBufferUploader->GetREFCreateCount()), "PhysicsDebugの頂点数がPhysicsDebugDynamicVertexBufferUploaderの容量を超えています。");

	// バッファービューを取得
	const auto l_vertexBufferView = l_physicsDebugVertexBufferUploader->WriteVertexList(l_lineVertexList);

	FWK_ASSERT_RETURN_IF(l_vertexBufferView.BufferLocation == DynamicBufferUploaderBase::k_invalidGPUVirtualAddress, "PhysicsDebug用VertexBufferViewの作成に失敗しました。");

	// プリミティブと頂点バッファービューを設定
	l_directCommandList.SetupPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	l_directCommandList.SetupVertexBufferView (l_vertexBufferView);

	l_directCommandList.DrawInstanced(static_cast<UINT>(l_lineVertexList.size()),
									  k_lineInstanceCount,
									  k_startVertexLocation,
									  k_startInstanceLocation);
}