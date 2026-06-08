#pragma once

namespace FWK::Constant
{
	inline constexpr TypeAlias::Math::Color k_colorLess = 
	{
		0.0F,
		0.0F,
		0.0F,
		1.0F
	};

	inline constexpr std::string_view k_colorRJsonKey = "R";
	inline constexpr std::string_view k_colorGJsonKey = "G";
	inline constexpr std::string_view k_colorBJsonKey = "B";
	inline constexpr std::string_view k_colorAJsonKey = "A";
}