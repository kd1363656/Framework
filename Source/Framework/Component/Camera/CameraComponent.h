#pragma once

namespace FWK
{
    class CameraComponent final : public ComponentBase
    {
    public:

         CameraComponent()          = default;
        ~CameraComponent() override = default;
    
        void Deserialize(const nlohmann::json& a_rootJson) override;

        void PostDeserialize() override;

        void EarlyUpdate   () override;
        void PostLateUpdate() override;

        void EditInspector() override;

        nlohmann::json Serialize() const override;

        std::shared_ptr<ComponentBase> Clone() const override;

        const auto& GetREFCamera() const { return m_camera; }

        auto& GetMutableREFCamera() { return m_camera; }

    private:
    
        Graphics::Camera m_camera = {};


        FWK_DEFINE_TYPE_INFO(CameraComponent, ComponentBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ComponentSharedFactory, FWK::CameraComponent)