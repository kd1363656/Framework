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

	// ライト方向はシェーダー側で内積計算に使うため、長さ1に正規化しておく
	// 正規化していないと、方向ベクトルの長さによって明るさまで変わってしまうため
	l_directionalLight.m_direction.Normalize();

	auto& l_ambientLight = m_cbLightPass->m_ambientLight;

	l_ambientLight.m_color	   = Constant::k_defaultAmbientLightColor;
	l_ambientLight.m_intensity = Constant::k_defaultAmbientLightIntensity;
}