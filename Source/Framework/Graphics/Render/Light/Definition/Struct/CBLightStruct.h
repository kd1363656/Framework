#pragma once

namespace FWK::Struct
{
	// Lit用ライト定数
	struct CBLightPass final
	{
		Struct::DirectionalLight m_directionalLight = {};
		float					 m_padding		    = {};

		Struct::AmbientLight m_ambientLight = {};
	};
}