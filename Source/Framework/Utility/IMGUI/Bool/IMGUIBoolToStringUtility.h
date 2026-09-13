#pragma once

namespace FWK::Utility
{
    inline constexpr auto& IMGUIBoolToString(const bool a_flag)
    {
        return a_flag ? Constant::k_imguiIsTrueString : Constant::k_imguiIsFalseString;
    };
}