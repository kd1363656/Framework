#pragma once

namespace FWK::Struct
{
	struct EventData final
	{
		bool m_hasPendingNotification        = false;
		bool m_isNotificationActiveThisFrame = false;
	};
}