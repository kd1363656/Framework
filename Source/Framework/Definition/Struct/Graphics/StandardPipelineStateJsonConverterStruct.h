#pragma once

namespace FWK::Struct
{
    struct StandardPipelineInputElement
    {
        std::string m_semanticName = {};

        D3D12_INPUT_ELEMENT_DESC m_inputElementDesc = {};
    };
}