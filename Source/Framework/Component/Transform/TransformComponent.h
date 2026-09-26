#pragma once

namespace FWK
{
    class MatrixStrategyBase;
    class GameObject;
}

namespace FWK
{
    class TransformComponent final
    {
    public:

         TransformComponent() = default;
        ~TransformComponent() = default;
 
        void Deserialize(const nlohmann::json& a_rootJson);

        void PostDeserialize();

        void PostLateUpdate();

        void EditInspector();

        nlohmann::json Serialize();

        TypeAlias::Math::Matrix CalculateScaleMatrix      () const { return TypeAlias::Math::Matrix::CreateScale         (m_transform.m_scale); }
        TypeAlias::Math::Matrix CalculateRotationMatrix   () const { return TypeAlias::Math::Matrix::CreateFromQuaternion(m_transform.m_rotation); }
        TypeAlias::Math::Matrix CalculateTranslationMatrix() const { return TypeAlias::Math::Matrix::CreateTranslation   (m_transform.m_position); }

        void ApplyParent    (const std::weak_ptr<GameObject>& a_parentGameObject);
        void ApplyStandalone();

        void ApplyTransformScale   (const TypeAlias::Math::Vector3&    a_scale);
        void ApplyTransformRotation(const TypeAlias::Math::Quaternion& a_rotation);
        void ApplyTransformPosition(const TypeAlias::Math::Vector3&    a_position);

        void SetMatrix(TypeAlias::Math::Matrix&& a_set) { m_matrix = std::move(a_set); }

        void SetShouldUpdateMatrixDirty(const bool a_set) { m_shouldUpdateMatrixDirty = a_set; }

        const auto& GetREFMatrixStrategy() const { return m_matrixStrategy; }

        const auto& GetREFParentGameObject() const { return m_parentGameObject; }

        const auto& GetREFMatrix() const { return m_matrix; }

        const auto& GetREFTransformComponent() const { return m_transform; }

        const auto& GetREFTransform() const { return m_transform; }

        auto& GetMutableREFMatrixStrategy() { return m_matrixStrategy; }

        auto& GetMutableREFTransform() { return m_transform; }

    private:
    
        void ConfirmMatrix();

        std::unique_ptr<MatrixStrategyBase> m_matrixStrategy = nullptr;

        std::weak_ptr<GameObject> m_owner            = {};
        std::weak_ptr<GameObject> m_parentGameObject = {};

        Converter::TransformComponentJsonConverter m_jsonConverter = {};
        TransformComponentInspector                m_inspector     = {};

        TypeAlias::Math::Matrix m_matrix = TypeAlias::Math::Matrix::Identity;

        Struct::Transform m_transform = {};

        bool m_shouldUpdateMatrixDirty = false;

        FWK_DEFINE_TYPE_INFO_SINGLE(TransformComponent)
    };
}