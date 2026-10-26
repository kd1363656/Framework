#pragma once

namespace FWK::Utility
{
    // a_value ÷ a_divisorを切り上げる(ComputeShaderのスレッド数からGroup数を求めるときなどに使う)
    // 「値 + 割る数 - 1」を割る数で割ると、余りがあるときだけ1つ増える
    inline UINT DivideRoundUp(const UINT a_value, const UINT a_divisor)
    {
        if (a_divisor == Constant::k_invalidDivisor) { return a_value; }

        return (a_value + a_divisor - Constant::k_divideRoundUpOffset) / a_divisor;
    }
}