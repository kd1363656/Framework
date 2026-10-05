#pragma once

namespace FWK::Enum
{
    enum class ApplyCalculateWorldMatrixBitShiftFlag : std::uint32_t
    {
        Invalid  = 0U,
        Scale    = 1U << 0U,
        Rotation = 1U << 1U,
        Position = 1U << 2U,
    };
 
    FWK_JSON_SERIALIZE_ENUM
    (
        ApplyCalculateWorldMatrixBitShiftFlag,
        FWK_JSON_ENUM_VALUE(ApplyCalculateWorldMatrixBitShiftFlag::Invalid),
        FWK_JSON_ENUM_VALUE(ApplyCalculateWorldMatrixBitShiftFlag::Scale),
        FWK_JSON_ENUM_VALUE(ApplyCalculateWorldMatrixBitShiftFlag::Rotation),
        FWK_JSON_ENUM_VALUE(ApplyCalculateWorldMatrixBitShiftFlag::Position),
    )
}

FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::ApplyCalculateWorldMatrixBitShiftFlag, FWK::Enum::ApplyCalculateWorldMatrixBitShiftFlag::Scale)
FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::ApplyCalculateWorldMatrixBitShiftFlag, FWK::Enum::ApplyCalculateWorldMatrixBitShiftFlag::Rotation)
FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(FWK::Enum::ApplyCalculateWorldMatrixBitShiftFlag, FWK::Enum::ApplyCalculateWorldMatrixBitShiftFlag::Position)