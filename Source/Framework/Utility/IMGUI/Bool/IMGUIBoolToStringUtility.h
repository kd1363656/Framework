#pragma once

namespace FWK::Utility
{
    inline constexpr auto& IMGUIBoolToString(const bool a_isTrue)
    {
        return a_isTrue ? Constant::k_imguiIsTrueString : Constant::k_imguiIsFalseString;
    };
}