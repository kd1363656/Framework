#pragma once

namespace FWK
{
    class GameObjectCameraComponent final : public GameObjectComponentBase
    {
    public:

         GameObjectCameraComponent()          = default;
        ~GameObjectCameraComponent() override = default;

        void Deserialize(const nlohmann::json& a_rootJson) override;

        void PostDeserialize() override;

        void EarlyUpdate   () override;
        void PostLateUpdate() override;

        void EditInspector() override;

        nlohmann::json Serialize() const override;

        std::shared_ptr<GameObjectComponentBase> Clone() const override;

        const auto& GetREFCamera() const { return m_camera; }

        auto& GetMutableREFCamera() { return m_camera; }

    private:

        Graphics::Camera m_camera = {};

        GameObjectCameraComponentInspector m_inspector = {};

        Converter::GameObjectCameraComponentJsonConverter m_jsonConverter = {};

        FWK_DEFINE_TYPE_INFO(GameObjectCameraComponent, GameObjectComponentBase)
    };
}

FWK_REGISTER_FACTORY_METHOD                      (FWK::TypeAlias::GameObjectComponentSharedFactory, FWK::GameObjectCameraComponent)
FWK_REGISTER_TAGGED_GAME_OBJECT_COMPONENT_FACTORY(FWK::Constant::k_gameObjectComponentTagCamera,    FWK::GameObjectCameraComponent)