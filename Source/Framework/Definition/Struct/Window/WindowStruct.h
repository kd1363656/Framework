#pragma once

namespace FWK::Struct
{
    struct WindowClientSize final
    {
        UINT m_width  = Constant::k_defaultWindowWidth;
        UINT m_height = Constant::k_defaultWindowHeight;
    };

    struct WindowResizeRequest final
    {
        WindowClientSize m_clientSize = { Constant::k_invalidClientWidth, Constant::k_invalidClientHeight };

        bool m_isRequested = false;
        bool m_isMinimized = false;
    };
}