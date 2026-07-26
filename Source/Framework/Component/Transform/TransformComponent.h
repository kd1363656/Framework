#pragma once

namespace FWK
{
	class MatrixStrategyBase;
}

namespace FWK
{
	class TransformComponent final : public ComponentBase
	{
	public:

		struct Transform final
		{
			TypeAlias::Math::Vector3    m_scale    = TypeAlias::Math::Vector3::One;
			TypeAlias::Math::Quaternion m_rotation = TypeAlias::Math::Quaternion::Identity;
			TypeAlias::Math::Vector3    m_position = TypeAlias::Math::Vector3::Zero;

			std::unique_ptr<MatrixStrategyBase> m_matrixStrategy = nullptr;
		};

	public:

		 TransformComponent();
		~TransformComponent() override;

		void DeserializePrefabData(const nlohmann::json& a_rootJson) override;
		void DeserializeSpawnData (const nlohmann::json& a_rootJson) override;

		void PostDeserialize() override;
		
		void ConfrimMatrix();

		void EditInspector() override;

		nlohmann::json SerializeSpawnData () override;
		nlohmann::json SerializePrefabData() override;

		TypeAlias::Math::Matrix CalculateScaleMatrix      () const { return TypeAlias::Math::Matrix::CreateScale         (m_transform.m_scale);    }
		TypeAlias::Math::Matrix CalculateRotationMatrix   () const { return TypeAlias::Math::Matrix::CreateFromQuaternion(m_transform.m_rotation); }
		TypeAlias::Math::Matrix CalculateTranslationMatrix() const { return TypeAlias::Math::Matrix::CreateTranslation   (m_transform.m_position); }
	
		void ApplyParentTransformComponent(const std::weak_ptr<TransformComponent>& a_parentTransformComponent);

		void SetMatrix(TypeAlias::Math::Matrix&& a_set) { m_matrix = std::move(a_set); }

		const auto& GetREFParentTransformComponent() const { return m_parentTransformComponent; }

	private:

		void ConfrimMatrixStrategy();

		void           Deserialize(const nlohmann::json& a_rootJson);
		nlohmann::json Serialize  ();

		static constexpr std::string_view k_initialScaleJsonKey          = "InitialScale";
		static constexpr std::string_view k_initialRotationJsonKey       = "InitialRotation";
		static constexpr std::string_view k_initialPositionJsonKey       = "InitialPosition";
		static constexpr std::string_view k_initialMatrixStrategyJsonKey = "InitialMatrixStrategy";
		static constexpr std::string_view k_matrixStrategySelectorLabel  = "MatrixStrategySelector";

		std::weak_ptr<TransformComponent> m_parentTransformComponent;

		TypeAlias::Math::Matrix m_matrix = TypeAlias::Math::Matrix::Identity;

		Transform m_transform;
		Transform m_initialSettingTransform;

		FWK_DEFINE_TYPE_INFO(TransformComponent, ComponentBase)
	};
}