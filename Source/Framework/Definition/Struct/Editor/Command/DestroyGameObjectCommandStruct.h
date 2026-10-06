#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK::Struct
{
    struct DestroyedGameObjectRecord final
    {
        std::vector<std::shared_ptr<GameObject>> m_detachedSubtreeGameObjectList = {};

        boost::uuids::uuid m_parentUUID = {};
    };
}