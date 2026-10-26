#pragma once

namespace FWK::Struct
{
    // ビューポートのTextureを表示している、画面上の範囲(ピクセル)
    struct ViewportScreenRect final
    {
        ImVec2 m_min  = {};
        ImVec2 m_size = {};
    };
}
