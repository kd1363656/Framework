#include "RenderGraphPassBase.h"

void FWK::Graphics::RenderGraphPassBase::WriteBackBuffer(const Enum::RenderGraphResourceUsage a_usage)
{
	AddResourceAccess(true,
					  Enum::RenderGraphRenderTargetType::None,
					  Enum::RenderGraphDepthStencilType::None,
					  Enum::RenderGraphAccessType::Write,
					  a_usage);
}

void FWK::Graphics::RenderGraphPassBase::ReadRenderTarget(const Enum::RenderGraphRenderTargetType a_renderTargetType, const Enum::RenderGraphResourceUsage a_usage)
{
	AddResourceAccess(false,
					  a_renderTargetType,
					  Enum::RenderGraphDepthStencilType::None,
					  Enum::RenderGraphAccessType::Read,
					  a_usage);
}
void FWK::Graphics::RenderGraphPassBase::ReadDepthStencil(const Enum::RenderGraphDepthStencilType a_depthStencilType, const Enum::RenderGraphResourceUsage a_usage)
{
	AddResourceAccess(false,
					  Enum::RenderGraphRenderTargetType::None,
					  a_depthStencilType,
					  Enum::RenderGraphAccessType::Read,
					  a_usage);
}

void FWK::Graphics::RenderGraphPassBase::WriteRenderTarget(const Enum::RenderGraphRenderTargetType a_renderTargetType, const Enum::RenderGraphResourceUsage a_usage)
{
	AddResourceAccess(false,
					  a_renderTargetType,
					  Enum::RenderGraphDepthStencilType::None,
					  Enum::RenderGraphAccessType::Write,
					  a_usage);
}
void FWK::Graphics::RenderGraphPassBase::WriteDepthStencil(const Enum::RenderGraphDepthStencilType a_depthStencilType, const Enum::RenderGraphResourceUsage a_usage)
{
	AddResourceAccess(false,
					  Enum::RenderGraphRenderTargetType::None,
					  a_depthStencilType,
					  Enum::RenderGraphAccessType::Write,
					  a_usage);
}

std::weak_ptr<FWK::Graphics::RootSignature> FWK::Graphics::RenderGraphPassBase::SetupRenderPipeline(const Renderer& a_renderer, const Enum::PipelineStateType a_pipelineStateType) const
{
	const auto& l_pipelineStateWeak = a_renderer.FindVALPipelineState(a_pipelineStateType);
	const auto& l_pipelineState     = l_pipelineStateWeak.lock       ();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_pipelineState, "指定されたPipelineStateTypeに対応するPipelineStateが無効になっており、レンダーパイプラインのセットに失敗しました。", {});

	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	l_directCommandList.SetupRenderPipeline(l_pipelineState);

	// 使用するルートシグネチャを戻り値にセット
	return l_pipelineState->GetREFUseRootSignature();
}

void FWK::Graphics::RenderGraphPassBase::AddResourceAccess(const bool a_isBackBuffer, const Enum::RenderGraphRenderTargetType a_renderTargetType, const Enum::RenderGraphDepthStencilType a_depthStencilType, const Enum::RenderGraphAccessType a_accessType, const Enum::RenderGraphResourceUsage a_usage)
{
	FWK_ASSERT_RETURN_IF_FAILED(a_renderTargetType == Enum::RenderGraphRenderTargetType::Invalid ||
								a_renderTargetType == Enum::RenderGraphRenderTargetType::Count,
								"RenderGraphRenderTargetTypeが無効です、ResourceAccessの追加に失敗しました。");

	FWK_ASSERT_RETURN_IF_FAILED(a_depthStencilType == Enum::RenderGraphDepthStencilType::Invalid ||
								a_depthStencilType == Enum::RenderGraphDepthStencilType::Count,
								"RenderDepthStencilTypeが無効です、ResourceAccessの追加に失敗しました。");


	FWK_ASSERT_RETURN_IF_FAILED(a_accessType == Enum::RenderGraphAccessType::Invalid ||
								a_accessType == Enum::RenderGraphAccessType::Count,
								"RenderGraphAccessTypeが無効です、ResourceAccessの追加に失敗しました。");

	FWK_ASSERT_RETURN_IF_FAILED(a_usage == Enum::RenderGraphResourceUsage::Invalid ||
								a_usage == Enum::RenderGraphResourceUsage::Count,
								"RenderGraphResourceUsageが無効です、ResourceAccessの追加に失敗しました。");

	Struct::RenderGraphResourceAccess l_resourceAccess = {};

	l_resourceAccess.m_isBackBuffer     = a_isBackBuffer;
	l_resourceAccess.m_renderTargetType = a_renderTargetType;
	l_resourceAccess.m_depthStencilType = a_depthStencilType;
	l_resourceAccess.m_accessType		= a_accessType;
	l_resourceAccess.m_usage		    = a_usage;

	m_resourceAccessList.emplace_back(l_resourceAccess);
}