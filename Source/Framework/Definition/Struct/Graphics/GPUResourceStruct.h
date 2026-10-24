#pragma once

namespace FWK::Struct
{
    struct GPUResource final
    {
        TypeAlias::ComPtr<ID3D12Resource2> m_resource = nullptr;

        TypeAlias::ComPtr<D3D12MA::Allocation> m_allocation = nullptr;
    };
}