#include "LightSystem.h"

void FWK::Graphics::LightSystem::ApplyDefaultSettings()
{
	if (!m_cbLightPass) 
	{
		m_cbLightPass = std::make_shared<Struct::CBLightPass>();
	}

	auto& l_directionalLight = m_cbLightPass->m_directionalLight;

	l_directionalLight.m_direction = Constant::k_defaultDirectionalLightDirection;
	l_directionalLight.m_color     = Constant::k_defaultDirectionalLightColor;
	l_directionalLight.m_intensity = Constant::k_defaultDirectionalLightIntensity;

	l_directionalLight.m_direction.Normalize();

	// ライト方向はシェーダー側で内積計算に使うため、長さ1に正規化しておく
	// 正規化していないと、方向ベクトルの長さによって明るさまで変わってしまうため
	l_directionalLight.m_direction.Normalize();

	auto& l_ambientLight = m_cbLightPass->m_ambientLight;

	l_ambientLight.m_color	   = Constant::k_defaultAmbientLightColor;
	l_ambientLight.m_intensity = Constant::k_defaultAmbientLightIntensity;

	RegisterCBLightPass();
}

void FWK::Graphics::LightSystem::RegisterCBLightPass()
{
		  auto& l_graphicsManager  = FWK::Graphics::GraphicsManager::GetInstance  ();
	      auto& l_renderer		   = l_graphicsManager.GetMutableREFRenderer      ();
	const auto& l_renderGraph	   = l_renderer.GetREFRenderGraph			      ();
	      auto& l_shadowContext    = l_renderer.GetMutableREFShadowContext        ();
		  auto& l_cascadeShadowMap = l_shadowContext.GetMutableREFCascadeShadowMap();

	const auto& l_lightPassDrawRequest = l_renderGraph.FindVALDrawRequestPass<LightPassDrawRequest>().lock();

	if (!l_lightPassDrawRequest) { return; }

	// 定数バッファの変更を反映するためにカメラクラスの定数バッファデータを送信する
	l_lightPassDrawRequest->SetSourceConstantBuffer(m_cbLightPass);
	l_cascadeShadowMap.SetCBLightPass              (m_cbLightPass);
}