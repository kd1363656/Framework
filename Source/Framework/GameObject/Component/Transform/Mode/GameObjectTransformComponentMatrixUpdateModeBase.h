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

        virtual nlohmann::json Serialize() const { /*必要に応じてオーバーライドしてください*/ };

        virtual void PreserveWorldMatrix(const TypeAlias::Math::Matrix& a_parentWorldMatrix, GameObjectTransformComponent& a_gameObjectTransformComponent, TypeAlias::Math::Matrix& a_previousWorldMatrix) = 0;
    };
}