#pragma once

namespace FWK::Utility
{
    // アニメーションを一定間隔で取り出すときの、取り出す時刻(秒)の一覧を作る
    // ボーンのキーフレームとモーフのウェイトのキーフレームを、同じ時刻で取り出すために使う
    //
    // 例 : 再生時間1.0秒、30FPSなら 0/30, 1/30, … , 30/30 秒の31個
    //      最後の時刻が再生時間を超える場合は、再生時間ちょうどにそろえる
    inline std::vector<double> CreateAnimationSampleTimeSecondList(const double a_durationSecond, const double a_frameRate)
    {
        if (a_frameRate <= Constant::k_invalidAnimationSampleFrameRate) { return {}; }

        const auto l_frameCount = static_cast<std::uint64_t>(std::ceil(a_durationSecond * a_frameRate));
        const std::uint64_t l_sampleCount = l_frameCount + Constant::k_animationTerminalSampleCount;

        std::vector<double> l_sampleTimeSecondList = {};

        l_sampleTimeSecondList.reserve(l_sampleCount);

        for (std::uint64_t l_sampleIndex = 0ULL; l_sampleIndex < l_sampleCount; ++l_sampleIndex)
        {
            const double l_sampleTimeSecond = static_cast<double>(l_sampleIndex) / a_frameRate;

            l_sampleTimeSecondList.emplace_back(std::min(l_sampleTimeSecond, a_durationSecond));
        }

        return l_sampleTimeSecondList;
    }
}