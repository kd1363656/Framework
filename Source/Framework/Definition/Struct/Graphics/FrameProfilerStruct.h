#pragma once

namespace FWK::Struct
{
    // 計測1回分の結果
    // 「どの区間(m_name)」の処理に「何ミリ秒(m_elapsedMS)」かかったかを表す
    struct FrameProfileSample final
    {
        static constexpr double k_initialElapsedMS = 0.0;

        std::string_view m_name = {};

        double m_elapsedMS = k_initialElapsedMS;
    };
}