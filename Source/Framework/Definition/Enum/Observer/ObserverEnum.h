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

FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::EventLane, FWK::Enum::EventLane::Invalid)
FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::EventLane, FWK::Enum::EventLane::TriggeredThisFrame)

FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::ComponentEvent, FWK::Enum::ComponentEvent::Invalid)