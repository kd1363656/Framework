#pragma once

namespace FWK::Enum
{
    enum class ComponentEvent
    {
        Invalid,

        InputMouseRight,

        CanRotation,
    };

    FWK_JSON_SERIALIZE_ENUM
    (
        ComponentEvent,
        FWK_JSON_ENUM_VALUE(ComponentEvent::Invalid),

        FWK_JSON_ENUM_VALUE(ComponentEvent::InputMouseRight),

        FWK_JSON_ENUM_VALUE(ComponentEvent::CanRotation),
    )
}

FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::ComponentEvent, FWK::Enum::ComponentEvent::Invalid)
FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::ComponentEvent, FWK::Enum::ComponentEvent::InputMouseRight)
FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::ComponentEvent, FWK::Enum::ComponentEvent::CanRotation)