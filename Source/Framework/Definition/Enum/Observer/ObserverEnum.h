#pragma once

namespace FWK::Enum
{
	enum class EventLane : std::uint32_t
	{
		Invalid            = 0U,
		TriggeredThisFrame = 1U << 0U,
	};
	
	enum class SceneShiftEvent
	{
		Invalid,

		Title,
		Game,
		Gameover,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		SceneShiftEvent, 
		FWK_JSON_ENUM_VALUE(SceneShiftEvent::Invalid),
		FWK_JSON_ENUM_VALUE(SceneShiftEvent::Title),
		FWK_JSON_ENUM_VALUE(SceneShiftEvent::Game),
		FWK_JSON_ENUM_VALUE(SceneShiftEvent::Gameover),
	)

	enum class ComponentEvent
	{
		Invalid,

		InputMouseRight,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		ComponentEvent, 
		FWK_JSON_ENUM_VALUE(ComponentEvent::Invalid),
		FWK_JSON_ENUM_VALUE(ComponentEvent::InputMouseRight),
	)
}

FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::EventLane, FWK::Enum::EventLane::Invalid)
FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::EventLane, FWK::Enum::EventLane::TriggeredThisFrame)

FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::ComponentEvent, FWK::Enum::ComponentEvent::Invalid)
FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::ComponentEvent, FWK::Enum::ComponentEvent::InputMouseRight)