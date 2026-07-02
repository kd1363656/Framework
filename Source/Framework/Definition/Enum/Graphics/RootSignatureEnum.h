#pragma once

namespace FWK::Enum
{
	enum class RootSignatureType
	{
		Invalid,
		SpriteScreen,
		StaticModelStandard,
		FinalColor,
		PhysicsDebug,
		FinalPresent,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RootSignatureType,
		FWK_JSON_ENUM_VALUE(RootSignatureType::Invalid),
		FWK_JSON_ENUM_VALUE(RootSignatureType::SpriteScreen),
		FWK_JSON_ENUM_VALUE(RootSignatureType::StaticModelStandard),
		FWK_JSON_ENUM_VALUE(RootSignatureType::FinalColor),
		FWK_JSON_ENUM_VALUE(RootSignatureType::PhysicsDebug),
		FWK_JSON_ENUM_VALUE(RootSignatureType::FinalPresent),
		FWK_JSON_ENUM_VALUE(RootSignatureType::Count),
	)
}