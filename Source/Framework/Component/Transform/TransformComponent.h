#pragma once

namespace FWK
{
	class TransformComponent final : public ComponentBase
	{
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

		TypeAlias::Math::Matrix CalcScaleMatrix      () const { return TypeAlias::Math::Matrix::CreateScale         (m_transform.m_scale);    }
		TypeAlias::Math::Matrix CalcRotationMatrix   () const { return TypeAlias::Math::Matrix::CreateFromQuaternion(m_transform.m_rotation); }
		TypeAlias::Math::Matrix CalcTranslationMatrix() const { return TypeAlias::Math::Matrix::CreateTranslation   (m_transform.m_position); }
	
		void ApplyParentTransformComponent(const std::weak_ptr<TransformComponent>& a_parentTransformComponent);

		const auto& GetREFParentTransformComponent() const { return m_parentTransformComponent; }

		void SetMatrix(TypeAlias::Math::Matrix&& a_set) { m_matrix = std::move(a_set); }

	private:

		void ConfrimMatrixStrategy();

		void           Deserialize(const nlohmann::json& a_rootJson);
		nlohmann::json Serialize  ();

		static constexpr std::string_view k_initialScaleJsonKey          = "InitialScale";
		static constexpr std::string_view k_initialRotationJsonKey       = "InitialRotation";
		static constexpr std::string_view k_initialPositionJsonKey       = "InitialPosition";
		static constexpr std::string_view k_initialMatrixStrategyJsonKey = "InitialMatrixStrategy";

		std::weak_ptr<TransformComponent> m_parentTransformComponent;

		TypeAlias::Math::Matrix m_matrix = TypeAlias::Math::Matrix::Identity;

		Struct::Transform m_transform;
		Struct::Transform m_initialSettingTransform;

		FWK_DEFINE_TYPE_INFO(TransformComponent, ComponentBase)
	};
}