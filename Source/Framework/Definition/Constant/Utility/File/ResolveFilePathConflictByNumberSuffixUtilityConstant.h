#pragma once

namespace FWK::Constant
{
    inline constexpr std::string_view k_numberSuffixOpenStringForFilePathConflict  = " (";
    inline constexpr std::string_view k_numberSuffixCloseStringForFilePathConflict = ")";

    inline constexpr std::size_t k_initialNumberSuffixForFilePathConflict = 1ULL;

    inline constexpr std::size_t k_nextNumberSuffixOffsetForFilePathConflict = 1ULL;
}