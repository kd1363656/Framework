#pragma once

namespace FWK
{
    class GameObjectTransformComponentMatrixUpdateStandaloneMode final : public GameObjectTransformComponentMatrixUpdateModeBase
    {
    public:

         GameObjectTransformComponentMatrixUpdateStandaloneMode()          = default;
        ~GameObjectTransformComponentMatrixUpdateStandaloneMode() override = default;
    
        void UpdateMatrix(GameObjectTransformComponent& a_transformComponent) override;

        void PreserveWorldMatrix(const TypeAlias::Math::Matrix&, GameObjectTransformComponent& a_transformComponent, TypeAlias::Math::Matrix& a_previousWorldMatrix) override;

        FWK_DEFINE_TYPE_INFO(GameObjectTransformComponentMatrixUpdateStandaloneMode, GameObjectTransformComponentMatrixUpdateModeBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::GameObjectTransformComponentMatrixUpdateModeUniqueFactory, FWK::GameObjectTransformComponentMatrixUpdateStandaloneMode)