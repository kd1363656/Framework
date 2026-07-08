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
	DeserializeCommon(a_rootJson);
}
void FWK::TransformComponent::DeserializeSpawnData(const nlohmann::json& a_rootJson)
{
	DeserializeCommon(a_rootJson);
}

void FWK::TransformComponent::ConfrimMatrix()
{
	ConfrimMatrixStrategy();
}

void FWK::TransformComponent::EditInspector()
{

}

nlohmann::json FWK::TransformComponent::SerializeSpawnData()
{
	return SerializeCommon();
}
nlohmann::json FWK::TransformComponent::SerializePrefabData()
{
	return SerializeCommon();
}

void FWK::TransformComponent::ApplyParentTransformComponent(const std::weak_ptr<TransformComponent>& a_parentTransformComponent)
{
	m_parentTransformComponent = a_parentTransformComponent;

	// セットした後にダーティーフラグで行列の更新が妨げられてもいいように
	// ここで一度だけ行列を更新しておく
	ConfrimMatrixStrategy();
}

void FWK::TransformComponent::DeserializeCommon(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	DeserializeTransform(a_rootJson, m_initialSettingTransform);
	DeserializeTransform(a_rootJson, m_transform);

	// 初期設定から行列を作成
	ConfrimMatrixStrategy();
}
void FWK::TransformComponent::DeserializeTransform(const nlohmann::json& a_rootJson, Struct::Transform& a_transform)
{
	if (a_rootJson.is_null()) { return; }

	Struct::Transform l_transform = {};

	l_transform.m_scale    = Utility::DeserializeVector3   (a_rootJson, k_initialScaleJsonKey);
	l_transform.m_rotation = Utility::DeserializeQuaternion(a_rootJson, k_initialRotationJsonKey);
	l_transform.m_position = Utility::DeserializeVector3   (a_rootJson, k_initialPositionJsonKey);

	Utility::DeserializeInstanceType<TypeAlias::MatrixStrategyUniqueFactory>(a_rootJson, k_initialMatrixStrategyJsonKey, l_transform.m_matrixStrategy);

	a_transform = std::move(l_transform);
}

nlohmann::json FWK::TransformComponent::SerializeCommon() const
{
	nlohmann::json l_rootJson = {};

	// 現在の位置ではなく初期位置をシリアライズする(キャラクターが移動したりした時に元の位置に戻す、そうしなければキャラクターの位置がそのシーンの最初になっていてほしい位置、回転、拡大にならない)
	Utility::UpdateJson(l_rootJson, Utility::SerializeVector3   (m_initialSettingTransform.m_scale,    k_initialScaleJsonKey));
	Utility::UpdateJson(l_rootJson, Utility::SerializeQuaternion(m_initialSettingTransform.m_rotation, k_initialRotationJsonKey));
	Utility::UpdateJson(l_rootJson, Utility::SerializeVector3   (m_initialSettingTransform.m_position, k_initialPositionJsonKey));

	Utility::UpdateJson(l_rootJson, Utility::SerializeInstanceType(m_initialSettingTransform.m_matrixStrategy, k_initialMatrixStrategyJsonKey));

	return l_rootJson;
}
void FWK::TransformComponent::ConfrimMatrixStrategy()
{
	if (!m_transform.m_matrixStrategy) { return; }

	m_transform.m_matrixStrategy->Execute(*this);
}