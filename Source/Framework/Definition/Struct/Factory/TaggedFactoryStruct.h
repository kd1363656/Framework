#pragma once

namespace FWK::Struct
{
    struct TaggedFactoryTagGroup final
    {
        std::vector<std::string_view> m_typeNameList = {};

        std::string_view m_tag = {};
    };
}