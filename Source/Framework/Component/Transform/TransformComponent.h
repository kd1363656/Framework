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

		 TransformComponent()          = default;
		~TransformComponent() override = default;

		void DeserializePrefab(const nlohmann::json& a_rootJson) override;
		void DeserializeScene (const nlohmann::json& a_rootJson) override;

		void PostDeserialize() override;
		
		void ConfrimMatrix();

		void EditInspector() override;

		nlohmann::json SerializePrefab() override;
		nlohmann::json SerializeScene () override;

		TypeAlias::Math::Matrix CalculateScaleMatrix      () const { return TypeAlias::Math::Matrix::CreateScale         (m_transform.m_scale);    }
		TypeAlias::Math::Matrix CalculateRotationMatrix   () const { return TypeAlias::Math::Matrix::CreateFromQuaternion(m_transform.m_rotation); }
		TypeAlias::Math::Matrix CalculateTranslationMatrix() const { return TypeAlias::Math::Matrix::CreateTranslation   (m_transform.m_position); }
	
		void ApplyParent    (const std::weak_ptr<GameObject>& a_parentObject);
		void ApplyStandalone();
		
		void SetMatrix(TypeAlias::Math::Matrix&& a_set) { m_matrix = std::move(a_set); }

		void SetTransformScale   (const TypeAlias::Math::Vector3&    a_set) { m_transform.m_scale    = a_set; }
		void SetTransformRotation(const TypeAlias::Math::Quaternion& a_set) { m_transform.m_rotation = a_set; }
		void SetTransformPosition(const TypeAlias::Math::Vector3&    a_set) { m_transform.m_position = a_set; }

		void SetInitializeMatrixStrategyTypeName(const std::string& a_set) { m_initializeMatrixStrategyTypeName = a_set; }

		void SetInitialSettingTransformScale   (const TypeAlias::Math::Vector3&    a_set) { m_initialSettingTransform.m_scale    = a_set; }
		void SetInitialSettingTransformRotation(const TypeAlias::Math::Quaternion& a_set) { m_initialSettingTransform.m_rotation = a_set; }
		void SetInitialSettingTransformPosition(const TypeAlias::Math::Vector3&    a_set) { m_initialSettingTransform.m_position = a_set; }

		void SetMatrixStrategy(std::unique_ptr<MatrixStrategyBase>&& a_set) { m_matrixStrategy = std::move(a_set); }

		const auto& GetREFMatrix() const { return m_matrix; }

		const auto& GetREFParentTransformComponent() const { return m_parentTransformComponent; }

		const auto& GetREFInitialMatrixStrategyName() const { return m_initializeMatrixStrategyTypeName; }
		const auto& GetREFInitialSettingTransform  () const { return m_initialSettingTransform; }

	private:

		void ConfrimMatrixStrategy();

		static constexpr std::string_view k_matrixStrategySelectorLabel = "MatrixStrategySelector";

		std::unique_ptr<MatrixStrategyBase> m_matrixStrategy = nullptr;

		std::weak_ptr<TransformComponent> m_parentTransformComponent = {};

		Converter::TransformComponentJsonConverter m_jsonConverter = {};

		std::string m_initializeMatrixStrategyTypeName = {};

		TypeAlias::Math::Matrix m_matrix = TypeAlias::Math::Matrix::Identity;

		Struct::Transform m_transform               = {};
		Struct::Transform m_initialSettingTransform = {};

		FWK_DEFINE_TYPE_INFO(TransformComponent, ComponentBase)
	};
}