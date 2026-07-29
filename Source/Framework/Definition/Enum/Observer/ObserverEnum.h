#pragma once

namespace FWK::Enum
{
	enum class EventLane : std::uint32_t
	{
		Invalid            = 0U,
		TriggeredThisFrame = 1U << 0U,
	};
	
	enum class ComponentEvent
	{
		Invalid,
	};
}