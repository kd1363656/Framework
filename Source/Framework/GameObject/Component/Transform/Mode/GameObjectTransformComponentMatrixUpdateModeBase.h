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

        virtual void Deserialize(const nlohmann::json& a_rootJson);

        virtual void PostDeserialize(const GameObjectTransformComponent&) { /*必要に応じてオーバーライドしてください*/ };

        virtual void UpdateMatrix(GameObjectTransformComponent&) = 0;

        virtual void EditInspector();

        virtual nlohmann::json Serialize() const;

        virtual void PreserveWorldMatrix(const TypeAlias::Math::Matrix& a_parentWorldMatrix, GameObjectTransformComponent& a_transformComponent, TypeAlias::Math::Matrix& a_previousWorldMatrix) = 0;

        void SetIsRotateAroundPosition(const bool a_set) { m_isRotateAroundPosition = a_set; }

        bool GetVALIsRotateAroundPosition() const { return m_isRotateAroundPosition; }

    private:

        Converter::GameObjectTransformComponentMatrixUpdateModeBaseJsonConverter m_jsonConverter = {};

        GameObjectTransformComponentMatrixUpdateModeBaseInspector m_inspector = {};

        bool m_isRotateAroundPosition = Constant::k_gameObjectTransformComponentMatrixUpdateModeBaseInitialVALIsRotateAroundPosition;

        FWK_DEFINE_TYPE_INFO_ROOT(GameObjectTransformComponentMatrixUpdateModeBase)
    };
}