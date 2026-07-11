#pragma once

namespace FWK::Struct
{
	// Lit用ライト定数
	struct CBLightPass final
	{
		DirectionalLight m_directionalLight = {};
		float			 m_padding          = {};

		AmbientLight m_ambientLight = {};
	};
}