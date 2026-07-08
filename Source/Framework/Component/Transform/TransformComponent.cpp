#include "TransformComponent.h"

FWK::TransformComponent::TransformComponent() :
	m_parentTransformComponent({}),

	m_matrix(),

	m_transform              (),
	m_initialSettingTransform()
{}
FWK::TransformComponent::~TransformComponent() = default;

void FWK::TransformComponent::DeserializePrefabData(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	Deserialize(a_rootJson);
}
void FWK::TransformComponent::DeserializeSpawnData(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	Deserialize(a_rootJson);
}

void FWK::TransformComponent::PostDeserialize()
{
	const auto& l_owner = GetREFOwner().lock();

	if (!l_owner) { return; }

	// 親が存在するなら親のTransformComponentをキャッシュする
	if (const auto& l_parent = l_owner->GetREFParent().lock())
	{
		m_parentTransformComponent = l_parent->GetVALREFTransformComponent();
	}

	ConfrimMatrixStrategy();
}

void FWK::TransformComponent::ConfrimMatrix()
{
	ConfrimMatrixStrategy();
}

void FWK::TransformComponent::EditInspector()
{
	Utility::FactoryRadioButtonSelector<TypeAlias::MatrixStrategyUniqueFactory>(k_matrixStrategySelectorLabel, m_transform.m_matrixStrategy);
}

nlohmann::json FWK::TransformComponent::SerializeSpawnData()
{
	return Serialize();
}
nlohmann::json FWK::TransformComponent::SerializePrefabData()
{
	return Serialize();
}

void FWK::TransformComponent::ApplyParentTransformComponent(const std::weak_ptr<TransformComponent>& a_parentTransformComponent)
{
	m_parentTransformComponent = a_parentTransformComponent;

	// セットした後にダーティーフラグで行列の更新が妨げられてもいいように
	// ここで一度だけ行列を更新しておく
	ConfrimMatrixStrategy();
}

void FWK::TransformComponent::ConfrimMatrixStrategy()
{
	if (!m_transform.m_matrixStrategy) { return; }

	m_transform.m_matrixStrategy->Execute(*this);
}

void FWK::TransformComponent::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_initialSettingTransform.m_scale    = Utility::DeserializeVector3   (a_rootJson, k_initialScaleJsonKey);
	m_initialSettingTransform.m_rotation = Utility::DeserializeQuaternion(a_rootJson, k_initialRotationJsonKey);
	m_initialSettingTransform.m_position = Utility::DeserializeVector3   (a_rootJson, k_initialPositionJsonKey);

	Utility::DeserializeInstanceType<TypeAlias::MatrixStrategyUniqueFactory>(a_rootJson, k_initialMatrixStrategyJsonKey, m_initialSettingTransform.m_matrixStrategy);	

	m_transform.m_scale    = m_initialSettingTransform.m_scale;
	m_transform.m_rotation = m_initialSettingTransform.m_rotation;
	m_transform.m_position = m_initialSettingTransform.m_position;

	Utility::DeserializeInstanceType<TypeAlias::MatrixStrategyUniqueFactory>(a_rootJson, k_initialMatrixStrategyJsonKey, m_transform.m_matrixStrategy);
}

nlohmann::json FWK::TransformComponent::Serialize()
{
	nlohmann::json l_rootJson = {};

	Utility::UpdateJson(l_rootJson, Utility::SerializeVector3(m_initialSettingTransform.m_scale,       k_initialScaleJsonKey));
	Utility::UpdateJson(l_rootJson, Utility::SerializeQuaternion(m_initialSettingTransform.m_rotation, k_initialRotationJsonKey));
	Utility::UpdateJson(l_rootJson, Utility::SerializeVector3(m_initialSettingTransform.m_position,    k_initialPositionJsonKey));

	Utility::UpdateJson(l_rootJson, Utility::SerializeInstanceType(m_initialSettingTransform.m_matrixStrategy, k_initialMatrixStrategyJsonKey));

	return l_rootJson;
}