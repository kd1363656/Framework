#pragma once

namespace FWK::Constant
{
    inline constexpr std::string_view k_gameObjectPrefabUUIDJsonKey     = "PrefabUUID";
    inline constexpr std::string_view k_gameObjectPrefabNodeUUIDJsonKey = "PrefabNodeUUID";

    inline constexpr std::string_view k_gameObjectChildListJsonKey = "ChildList";
    inline constexpr std::string_view k_gameObjectTransformComponentJsonKey = "TransformComponent";

    inline constexpr std::string_view k_gameObjectComponentListJsonKey                 = "ComponentList";
    inline constexpr std::string_view k_gameObjectComponentTypeNameJsonKey             = "ComponentTypeName";
    inline constexpr std::string_view k_gameObjectComponentEventObserverJsonKey        = "ComponentEventObserver";
    inline constexpr std::string_view k_gameObjectComponentEventObserverRemovedJsonKey = "ComponentEventObserverRemoved";
}