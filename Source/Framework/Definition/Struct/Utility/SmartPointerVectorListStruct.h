#pragma once

namespace FWK::Struct
{
    template <typename Type>
    struct SmartPointerVectorListArrayElementData final
    {
        static constexpr std::uintptr_t k_initialTypeAddress = 0ULL;

        Type m_type = {};

        std::uintptr_t m_typeAddress = k_initialTypeAddress;
    };
}