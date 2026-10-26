#pragma once

namespace FWK
{
    class GameObjectTransformComponentMatrixUpdateModeBase;
}

namespace FWK
{
    // 全てのGameObjectが必ず1つ持ち、コンポーネント一覧の先頭に固定されるコンポーネント
    // (削除・並べ替え不可。先頭固定かどうかはGameObjectComponentContainerがTypeINFOで判定する)
    class GameObjectTransformComponent final : public GameObjectComponentBase
    {
    public:

         GameObjectTransformComponent()          = default;
        ~GameObjectTransformComponent() override = default;
 
        void Deserialize(const nlohmann::json& a_rootJson) override;

        void PostDeserialize() override;

        void PostLateUpdate() override;

        void EditInspector() override;

        nlohmann::json Serialize() const override;

        std::shared_ptr<GameObjectComponentBase> Clone() const override;

        TypeAlias::Math::Matrix CalculateScaleMatrix      () const { return TypeAlias::Math::Matrix::CreateScale         (m_transform.m_scale); }
        TypeAlias::Math::Matrix CalculateRotationMatrix   () const { return TypeAlias::Math::Matrix::CreateFromQuaternion(m_transform.m_rotation); }
        TypeAlias::Math::Matrix CalculateTranslationMatrix() const { return TypeAlias::Math::Matrix::CreateTranslation   (m_transform.m_position); }

        void ApplyParent    ();
        void ApplyStandalone();
        
        void ApplyTransformScale   (const TypeAlias::Math::Vector3&    a_scale);
        void ApplyTransformRotation(const TypeAlias::Math::Quaternion& a_rotation);
        void ApplyTransformPosition(const TypeAlias::Math::Vector3&    a_position);

        void SetMatrix(TypeAlias::Math::Matrix&& a_set) { m_matrix = std::move(a_set); }

        void SetShouldUpdateMatrixDirty(const bool a_set) { m_shouldUpdateMatrixDirty = a_set; }

        const auto& GetREFMatrixUpdateMode() const { return m_matrixUpdateMode; }

        const auto& GetREFMatrix() const { return m_matrix; }

        const auto& GetREFGameObjectTransformComponent() const { return m_transform; }

        const auto& GetREFTransform() const { return m_transform; }

        auto& GetMutableREFMatrixUpdateMode() { return m_matrixUpdateMode;; }

        auto& GetMutableREFTransform() { return m_transform; }

        auto& GetMutableREFMatrix() { return m_matrix; }

    private:
    
        void UpdateMatrix();

        void MarkChildrenMatrixDirty    () const;
        void ConfirmMatrixFromRootToSelf() const;

        std::unique_ptr<GameObjectTransformComponentMatrixUpdateModeBase> m_matrixUpdateMode = nullptr;

        GameObjectTransformComponentInspector m_inspector = {};

        Converter::GameObjectTransformComponentJsonConverter m_jsonConverter = {};

        TypeAlias::Math::Matrix m_matrix = TypeAlias::Math::Matrix::Identity;

        Struct::Transform m_transform = {};

        bool m_shouldUpdateMatrixDirty = false;

        FWK_DEFINE_TYPE_INFO(GameObjectTransformComponent, GameObjectComponentBase)
    };
}

// シーン読み込み時にComponentTypeの名前から生成できるようファクトリーへ登録する
// Add Componentメニューから手動で追加させないため、TypeINFOのタグには登録しない
FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::GameObjectComponentSharedFactory, FWK::GameObjectTransformComponent)