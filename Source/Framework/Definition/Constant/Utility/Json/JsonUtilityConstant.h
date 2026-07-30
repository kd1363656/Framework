#pragma once

namespace FWK::Constant
{
	inline constexpr std::string_view k_xJsonKey = "X";
	inline constexpr std::string_view k_yJsonKey = "Y";
	inline constexpr std::string_view k_zJsonKey = "Z";
	inline constexpr std::string_view k_wJsonKey = "W";

	inline constexpr std::string_view k_colorRJsonKey = "R";
	inline constexpr std::string_view k_colorGJsonKey = "G";
	inline constexpr std::string_view k_colorBJsonKey = "B";
	inline constexpr std::string_view k_colorAJsonKey = "A";

	inline constexpr std::string_view k_uuidJsonKey = "UUID";

	inline constexpr TypeAlias::Math::Color k_whiteColor = { 1.0F, 1.0F, 1.0F, 1.0F };
}