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

	FWK_JSON_SERIALIZE_ENUM
	(
		ComponentEvent, 
		FWK_JSON_ENUM_VALUE(ComponentEvent::Invalid)
	)
}