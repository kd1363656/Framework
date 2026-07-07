#include "RenderGraphPassBase.h"

void FWK::Graphics::RenderGraphPassBase::WriteBackBuffer(const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage)
{
	AddResourceAccess(true,
					  Enum::RenderGraphRenderTargetType::None,
					  Enum::RenderGraphDepthStencilType::None,
					  Enum::RenderGraphAccessType::Write,
					  a_beforeUsage,
					  a_afterUsage);
}

void FWK::Graphics::RenderGraphPassBase::ReadRenderTarget(const Enum::RenderGraphRenderTargetType a_renderTargetType, const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage)
{
	AddResourceAccess(false,
					  a_renderTargetType,
					  Enum::RenderGraphDepthStencilType::None,
					  Enum::RenderGraphAccessType::Read,
					  a_beforeUsage,
					  a_afterUsage);
}
void FWK::Graphics::RenderGraphPassBase::ReadDepthStencil(const Enum::RenderGraphDepthStencilType a_depthStencilType, const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage)
{
	AddResourceAccess(false,
					  Enum::RenderGraphRenderTargetType::None,
					  a_depthStencilType,
					  Enum::RenderGraphAccessType::Read,
					  a_beforeUsage,
					  a_afterUsage);
}

void FWK::Graphics::RenderGraphPassBase::WriteDepthStencil(const Enum::RenderGraphDepthStencilType a_depthStencilType, const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage)
{
	AddResourceAccess(false,
					  Enum::RenderGraphRenderTargetType::None,
					  a_depthStencilType,
					  Enum::RenderGraphAccessType::Write,
					  a_beforeUsage,
					  a_afterUsage);
}
void FWK::Graphics::RenderGraphPassBase::WriteRenderTarget(const Enum::RenderGraphRenderTargetType a_renderTargetType, const Enum::RenderGraphResourceUsage a_beforeUsage, const Enum::RenderGraphResourceUsage a_afterUsage)
{
	AddResourceAccess(false,
					  a_renderTargetType,
					  Enum::RenderGraphDepthStencilType::None,
					  Enum::RenderGraphAccessType::Write,
					  a_beforeUsage,
					  a_afterUsage);
}

std::weak_ptr<FWK::Graphics::RootSignature> FWK::Graphics::RenderGraphPassBase::SetupGraphicsRenderPipeline(Renderer& a_renderer, const Enum::PipelineStateType a_pipelineStateType) const
{
	const auto& l_pipelineStateWeak = a_renderer.FindVALPipelineState<Graphics::GraphicsPipelineStateBase>(a_pipelineStateType);
	const auto& l_pipelineState     = l_pipelineStateWeak.lock                                            ();

	FWK_ASSERT_RETURN_VALUE_IF(!l_pipelineState, "指定されたPipelineStateTypeに対応するPipelineStateが無効になっており、レンダーパイプラインのセットに失敗しました。", {});

	auto& l_directCommandList = a_renderer.GetMutableREFDirectCommandList();

	l_directCommandList.SetupRenderPipeline(l_pipelineStateWeak);

	// 使用するルートシグネチャを戻り値にセット
	return l_pipelineState->GetREFUseRootSignature();
}

void FWK::Graphics::RenderGraphPassBase::SetupExecutionLayer(const Enum::RenderGraphPassExecutionLayer a_executionLayer)
{
	FWK_ASSERT_RETURN_IF(a_executionLayer == Enum::RenderGraphPassExecutionLayer::Invalid ||
						 a_executionLayer == Enum::RenderGraphPassExecutionLayer::Count,
						 "RenderGraphPassExecutionLayerが無効となっており、RenderGraphPassExecutionLayerのセットに失敗しました。");

	m_executionLayer = a_executionLayer;
}

void FWK::Graphics::RenderGraphPassBase::AddResourceAccess(const bool						       a_isBackBuffer,
														   const Enum::RenderGraphRenderTargetType a_renderTargetType,
														   const Enum::RenderGraphDepthStencilType a_depthStencilType,
														   const Enum::RenderGraphAccessType       a_accessType,
														   const Enum::RenderGraphResourceUsage    a_beforeUsage,
														   const Enum::RenderGraphResourceUsage    a_afterUsage)
{
	FWK_ASSERT_RETURN_IF(a_renderTargetType == Enum::RenderGraphRenderTargetType::Invalid ||
						 a_renderTargetType == Enum::RenderGraphRenderTargetType::Count,
						 "RenderGraphRenderTargetTypeが無効です、ResourceAccessの追加に失敗しました。");

	FWK_ASSERT_RETURN_IF(a_depthStencilType == Enum::RenderGraphDepthStencilType::Invalid ||
						 a_depthStencilType == Enum::RenderGraphDepthStencilType::Count,
						 "RenderDepthStencilTypeが無効です、ResourceAccessの追加に失敗しました。");


	FWK_ASSERT_RETURN_IF(a_accessType == Enum::RenderGraphAccessType::Invalid ||
						 a_accessType == Enum::RenderGraphAccessType::Count,
						 "RenderGraphAccessTypeが無効です、ResourceAccessの追加に失敗しました。");

	// beforeUsageはPass実行前に必ず使うStateなので、Noneは禁止
	FWK_ASSERT_RETURN_IF(a_beforeUsage == Enum::RenderGraphResourceUsage::Invalid ||
						 a_beforeUsage == Enum::RenderGraphResourceUsage::None    ||
						 a_beforeUsage == Enum::RenderGraphResourceUsage::Count,
						 "BeforeUsageが無効です、ResourceAccessの追加に失敗しました。");

	// afterUsageはNoneを許可する
	// Noneは「Pass実行後は状態遷移しない」という意味。
	FWK_ASSERT_RETURN_IF(a_afterUsage == Enum::RenderGraphResourceUsage::Invalid ||
						 a_afterUsage == Enum::RenderGraphResourceUsage::Count,
						 "AfterUsageが無効です、ResourceAccessの追加に失敗しました。");

	Struct::RenderGraphResourceAccess l_resourceAccess = {};

	l_resourceAccess.m_isBackBuffer     = a_isBackBuffer;
	l_resourceAccess.m_renderTargetType = a_renderTargetType;
	l_resourceAccess.m_depthStencilType = a_depthStencilType;
	l_resourceAccess.m_accessType		= a_accessType;
	l_resourceAccess.m_beforeUsage		= a_beforeUsage;
	l_resourceAccess.m_afterUsage		= a_afterUsage;

	m_resourceAccessList.emplace_back(l_resourceAccess);
}