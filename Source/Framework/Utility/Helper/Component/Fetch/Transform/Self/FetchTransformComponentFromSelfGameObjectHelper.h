#pragma once

namespace FWK::Utility
{
    class FetchTransformComponentFromSelfGameObjectHelper final
    {
    public:

         FetchTransformComponentFromSelfGameObjectHelper() = default;
        ~FetchTransformComponentFromSelfGameObjectHelper() = default;

        void PostDeserialize(const GameObject& a_self);

        const auto& GetREFFetchedTransformComponent() const { return m_fetchedTransformComponent; }

    private:

        std::weak_ptr<TransformComponent> m_fetchedTransformComponent = {};
    };
}