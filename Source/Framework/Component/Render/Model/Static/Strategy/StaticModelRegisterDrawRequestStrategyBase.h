#pragma once

namespace FWK
{
	class StaticModelRegisterDrawRequestStrategyBase : public StrategyBase<StaticModelComponent>
	{
	public:

		 StaticModelRegisterDrawRequestStrategyBase()          = default;
		~StaticModelRegisterDrawRequestStrategyBase() override = default;

	protected:

		template <Concept::IsDerivedDrawRequestPerObjectBaseConcept Type>
		void RegisterDrawRequestData(StaticModelComponent& a_staticModelComponent)
		{
			const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
			const auto& l_renderer        = l_graphicsManager.GetREFRenderer      ();
			const auto& l_renderGraph     = l_renderer.GetREFRenderGraph          ();

			const auto& l_drawRequestPass = l_renderGraph.FindVALDrawRequestPerObject<Type>().lock();

			FWK_ASSERT_RETURN_IF(!l_drawRequestPass, "レンダーグラフのシェーダーパスの取得に失敗しました。");

			// Lit用パスにDrawRequestDataを格納する
			l_drawRequestPass->AddDrawRequest(a_staticModelComponent.GetREFDrawRequestData());
		}

		FWK_DEFINE_TYPE_INFO_ROOT(StaticModelRegisterDrawRequestStrategyBase)
	};
}