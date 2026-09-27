#pragma once

namespace FWK
{
    class GameObjectTransformComponentMatrixUpdateStandaloneMode final : public GameObjectTransformComponentMatrixUpdateModeBase
    {
    public:

         GameObjectTransformComponentMatrixUpdateStandaloneMode()          = default;
        ~GameObjectTransformComponentMatrixUpdateStandaloneMode() override = default;
    
        void UpdateMatrix(GameObjectTransformComponent& a_gameObjectTransformComponent) override;

        void PreserveWorldMatrix(const TypeAlias::Math::Matrix& a_parentWorldMatrix, GameObjectTransformComponent& a_gameObjectTransformComponent, TypeAlias::Math::Matrix& a_previousWorldMatrix) override;
    };
}