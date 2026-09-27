#pragma once

namespace FWK
{
    class GameObjectTransformComponentMatrixUpdateModeBase;
    class GameObject;
}

namespace FWK
{
    class GameObjectTransformComponent final
    {
    public:

         GameObjectTransformComponent() = default;
        ~GameObjectTransformComponent() = default;
 
        void Deserialize(const nlohmann::json& a_rootJson);

        void PostDeserialize();

        void PostLateUpdate();

        void EditInspector();

        nlohmann::json Serialize() const;

        std::shared_ptr<GameObjectTransformComponent> Clone() const;

        TypeAlias::Math::Matrix CalculateScaleMatrix      () const { return TypeAlias::Math::Matrix::CreateScale         (m_transform.m_scale); }
        TypeAlias::Math::Matrix CalculateRotationMatrix   () const { return TypeAlias::Math::Matrix::CreateFromQuaternion(m_transform.m_rotation); }
        TypeAlias::Math::Matrix CalculateTranslationMatrix() const { return TypeAlias::Math::Matrix::CreateTranslation   (m_transform.m_position); }

        void ApplyParent    ();
        void ApplyStandalone();
        
        void ApplyTransformScale   (const TypeAlias::Math::Vector3&    a_scale);
        void ApplyTransformRotation(const TypeAlias::Math::Quaternion& a_rotation);
        void ApplyTransformPosition(const TypeAlias::Math::Vector3&    a_position);

        void SetOwner(const std::weak_ptr<GameObject>& a_set) { m_owner = a_set; }

        void SetMatrix(TypeAlias::Math::Matrix&& a_set) { m_matrix = std::move(a_set); }

        void SetShouldUpdateMatrixDirty(const bool a_set) { m_shouldUpdateMatrixDirty = a_set; }

        const auto& GetREFMatrixUpdateMode() const { return m_matrixUpdateMode; }

        const auto& GetREFOwner() const { return m_owner; }

        const auto& GetREFMatrix() const { return m_matrix; }

        const auto& GetREFGameObjectTransformComponent() const { return m_transform; }

        const auto& GetREFTransform() const { return m_transform; }

        auto& GetMutableREFMatrixUpdateMode() { return m_matrixUpdateMode;; }

        auto& GetMutableREFTransform() { return m_transform; }

        auto& GetMutableREFMatrix() { return m_matrix; }

    private:
    
        void UpdateMatrix();
        
        void ConfirmMatrixFromRootToSelf() const;

        std::unique_ptr<GameObjectTransformComponentMatrixUpdateModeBase> m_matrixUpdateMode = nullptr;

        std::weak_ptr<GameObject> m_owner = {};

        GameObjectTransformComponentInspector m_inspector = {};

        Converter::GameObjectTransformComponentJsonConverter m_jsonConverter = {};

        TypeAlias::Math::Matrix m_matrix = TypeAlias::Math::Matrix::Identity;

        Struct::Transform m_transform = {};

        bool m_shouldUpdateMatrixDirty = false;

        FWK_DEFINE_TYPE_INFO_SINGLE(GameObjectTransformComponent)
    };
}