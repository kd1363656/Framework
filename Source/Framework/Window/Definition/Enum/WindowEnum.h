#pragma once

namespace FWK::Enum
{
	enum class WindowStyle
	{
		None,
		Normal,
		BorderlessFullScreen,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		WindowStyle, 
		FWK_JSON_ENUM_VALUE(WindowStyle::None),
		FWK_JSON_ENUM_VALUE(WindowStyle::Normal),
		FWK_JSON_ENUM_VALUE(WindowStyle::BorderlessFullScreen)
	)
}