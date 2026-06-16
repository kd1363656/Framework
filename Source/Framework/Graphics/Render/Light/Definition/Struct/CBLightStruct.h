#pragma once

namespace FWK::Struct
{
	// Lit用ライト定数
	struct CBLightPass final
	{
		Struct::DirectionalLight m_directionalLight = {};
		float					 m_padding		    = 0.0F;

		Struct::AmbientLight m_ambientLight = {};
	};
}