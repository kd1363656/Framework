#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK::Converter
{
    class GameObjectJsonConverter final
    {
    public:

         GameObjectJsonConverter() = default;
        ~GameObjectJsonConverter() = default;
    
        void Deserialize(const nlohmann::json& a_rootJson, const std::weak_ptr<GameObject>& a_gameObject);

        nlohmann::json Serialize(const GameObject& a_gameObject) const;
    };
}