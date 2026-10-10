#pragma once

namespace FWK::Struct
{
    struct ModelDrawItem final
    {
        Struct::RCModelDrawItem m_rootConstant = {};

        D3D12_DISPATCH_MESH_ARGUMENTS m_dispatchMeshArguments = {};
    };
}