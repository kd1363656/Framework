#pragma once

namespace FWK
{
    class GameObjectTransformComponent;
}

namespace FWK
{
    class GameObjectTransformComponentMatrixUpdateHierarchicalMode final : public GameObjectTransformComponentMatrixUpdateModeBase
    {
    public:

         GameObjectTransformComponentMatrixUpdateHierarchicalMode()          = default;
        ~GameObjectTransformComponentMatrixUpdateHierarchicalMode() override = default;
    
        void Deserialize(const nlohmann::json& a_rootJson);

        void PostDeserialize(const GameObjectTransformComponent& a_transformComponent) override;

        void UpdateMatrix(GameObjectTransformComponent& a_transformComponent) override;

        nlohmann::json Serialize() const;

        void PreserveWorldMatrix(const TypeAlias::Math::Matrix& a_parentWorldMatrix, GameObjectTransformComponent& a_transformComponent, TypeAlias::Math::Matrix& a_previousWorldMatrix) override;

        void EnableAllApplyCalculateParentWorldMatrixFlag();;

        const auto& GetREFCalculateParentWorldMatrixEnumBitShift() const { return m_calculateParentWorldMatrixEnumBitShift; }

        auto& GetMutableREFCalculateParentWorldMatrixEnumBitShift() { return m_calculateParentWorldMatrixEnumBitShift; }

    private:

        TypeAlias::Math::Matrix CalculateParentWorldMatrix() const;

        std::weak_ptr<GameObjectTransformComponent> m_parentTransformComponent = {};

        Utility::EnumBitShift<Enum::ApplyCalculateWorldMatrixBitShiftFlag> m_calculateParentWorldMatrixEnumBitShift = {};

        GameObjectTransformComponentMatrixUpdateHierarchicalModeInspector m_inspector = {};

        Converter::GameObjectTransformComponentMatrixUpdateHierarchicalModeJsonConverter m_jsonConverter = {};

        FWK_DEFINE_TYPE_INFO(GameObjectTransformComponentMatrixUpdateHierarchicalMode, GameObjectTransformComponentMatrixUpdateModeBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::GameObjectTransformComponentMatrixUpdateModeUniqueFactory, FWK::GameObjectTransformComponentMatrixUpdateHierarchicalMode)