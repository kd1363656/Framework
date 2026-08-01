#pragma once

namespace FWK::Utility
{
	inline constexpr auto& BoolToString(const bool a_flag)
	{
		return a_flag ? Constant::k_isTrueString : Constant::k_isFalseString;
	};
}