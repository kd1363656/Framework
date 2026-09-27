#pragma once

namespace FWK::Struct
{
    struct GameObjectPrefabChildHierarchy final
    {
        UUIDRegistry<std::weak_ptr<GameObject>> m_uuidRegistry = {};

        std::unordered_set<boost::uuids::uuid> m_removedChildUUIDSet = {};
    };
}