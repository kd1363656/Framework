#pragma once

namespace FWK::Struct
{
    struct GameObjectPlacement final
    {
        bool operator==(const GameObjectPlacement&) const = default;

        std::vector<boost::uuids::uuid> m_siblingUUIDList = {};

        boost::uuids::uuid m_parentUUID = {};
    };
}