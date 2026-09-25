#pragma once

namespace FWK::Enum
{
    enum class EventLaneBitShiftFlag : std::uint32_t
    {
        Invalid            = 0U,
        TriggeredThisFrame = 1U << 0U,
        KeepFrame          = 2U << 0U,
    };
}

FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::EventLaneBitShiftFlag, FWK::Enum::EventLaneBitShiftFlag::Invalid)
FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::EventLaneBitShiftFlag, FWK::Enum::EventLaneBitShiftFlag::TriggeredThisFrame)
FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::EventLaneBitShiftFlag, FWK::Enum::EventLaneBitShiftFlag::KeepFrame)