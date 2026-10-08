#pragma once

namespace FWK
{
    class GameObject;
    class GameObjectTransformComponent;
}

namespace FWK::Utility
{
    class FetchSelfGameObjectTransformComponentHelper final
    {
    public:

         FetchSelfGameObjectTransformComponentHelper() = default;
        ~FetchSelfGameObjectTransformComponentHelper() = default;

        void PostDeserialize(const std::weak_ptr<GameObject>& a_selfGameObject);

        const auto& GetREFTransformComponent() const { return m_transformComponent; }

    private:

        std::weak_ptr<GameObjectTransformComponent> m_transformComponent = {};
    };
}