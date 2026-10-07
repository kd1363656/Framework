#pragma once

namespace FWK::Struct
{
    struct ReparentGameObjectState final
    {
        std::vector<boost::uuids::uuid> m_siblingUUIDList = {};

        boost::uuids::uuid m_parentUUID = {};
    };
}