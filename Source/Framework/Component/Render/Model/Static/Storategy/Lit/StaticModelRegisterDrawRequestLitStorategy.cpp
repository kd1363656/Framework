#include "StaticModelRegisterDrawRequestLitStorategy.h"

void FWK::StaticModelRegisterDrawRequestLitStorategyBase::Execute(StaticModelComponent& a_staticModelComponent)
{
	const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
	const auto& l_renderer        = l_graphicsManager.GetREFRenderer      ();
	const auto& l_renderGraph     = l_renderer.GetREFRenderGraph          ();

	const auto& l_drawRequestPass = l_renderGraph.FindVALDrawRequestPerObject<Graphics::StaticModelStandardLitPerObjectDrawRequest>().lock();

	FWK_ASSERT_RETURN_IF(!l_drawRequestPass, "レンダーグラフのStaticモデル用Litシェーダーのパスの取得に失敗しました。");

	// Lit用パスにDrawRequestDataを格納する
	l_drawRequestPass->AddDrawRequest(a_staticModelComponent.GetREFDrawRequestData());
}