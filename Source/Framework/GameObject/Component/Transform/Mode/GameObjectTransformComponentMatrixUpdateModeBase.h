#pragma once

namespace FWK
{
    class GameObjectTransformComponent;
}

namespace FWK
{
    class GameObjectTransformComponentMatrixUpdateModeBase
    {
    public:

                 GameObjectTransformComponentMatrixUpdateModeBase() = default;
        virtual ~GameObjectTransformComponentMatrixUpdateModeBase() = default;

        virtual void Deserialize(const nlohmann::json&) { /*必要に応じてオーバーライドしてください*/ };

        virtual void PostDeserialize(const GameObjectTransformComponent&) { /*必要に応じてオーバーライドしてください*/ };

        virtual void UpdateMatrix(GameObjectTransformComponent&) = 0;

        virtual nlohmann::json Serialize() const { return {}; };

        virtual void PreserveWorldMatrix(const TypeAlias::Math::Matrix& a_parentWorldMatrix, GameObjectTransformComponent& a_transformComponent, TypeAlias::Math::Matrix& a_previousWorldMatrix) = 0;

        FWK_DEFINE_TYPE_INFO_ROOT(GameObjectTransformComponentMatrixUpdateModeBase)
    };
}