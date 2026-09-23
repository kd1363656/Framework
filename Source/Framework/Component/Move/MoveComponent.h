#pragma once

namespace FWK
{
    class MoveComponent final : public ComponentBase
    {
    public:

         MoveComponent() = default;
        ~MoveComponent() = default;

        void DeserializePrefab(const nlohmann::json& a_rootJson) override;

        void PostDeserialize() override;

        void Update() override;

        void EditInspector() override;

        nlohmann::json SerializePrefab() override;

        const auto& GetREFMoveMode() const { return m_moveMode; }

        std::weak_ptr<MoveComponentModeBase> GetVALMoveMode() const { return m_moveMode; }

    private:
        
        std::shared_ptr<MoveComponentModeBase> m_moveMode = nullptr;

        MoveComponentInspector m_inspector = {};

        Converter::MoveComponentJsonConverter m_jsonConverter = {};

        FWK_DEFINE_TYPE_INFO(MoveComponent, ComponentBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ComponentSharedFactory, FWK::MoveComponent)