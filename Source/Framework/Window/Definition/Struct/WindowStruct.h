#pragma once

namespace FWK::Struct
{
	struct ClientSize
	{
		UINT m_width  = Constant::k_defaultWindowWidth;
		UINT m_height = Constant::k_defaultWindowHeight;
	};

	struct WindowResizeRequest final
	{
		ClientSize m_clientSize = {};

		bool m_isRequested = false;
		bool m_isMinimized = false;
	};
}