#include "PhysicsDebugPass.h"

FWK::Graphics::PhysicsDebugPass::PhysicsDebugPass()
{
	SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::Debug);

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

void FWK::Graphics::PhysicsDebugPass::Execute(Renderer& a_renderer, RenderGraph& a_renderGraph)
{

}