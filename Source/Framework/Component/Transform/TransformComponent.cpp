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

	m_initialSettingTransform.m_position = Utility::DeserializeVector3(a_rootJson, k_initialPositionJsonKey);

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
	// 初期スポーン位置はエディターでドラッグしたときのみ決まる
	// 拡大率
	if (ImGui::DragFloat3("拡大率", &m_transform.m_scale.x, Constant::k_imguiDefaultDragValue))
	{
		m_initialSettingTransform.m_scale = m_transform.m_scale;
	}

	// 回転
	if (auto l_euler = FWK::Utility::QuaternionToEuler(m_transform.m_rotation);
		ImGui::DragFloat3("回転", &l_euler.x, Constant::k_imguiDefaultDragValue))
	{
		// オイラー角に変換していたクオータニオンを元に戻して格納
		auto l_dragResult = Utility::EulerToQuaternion(l_euler);

		m_transform.m_rotation               = l_dragResult;
		m_initialSettingTransform.m_rotation = l_dragResult;
	}

	// 座標
	if (ImGui::DragFloat3("座標", &m_transform.m_position.x, Constant::k_imguiDefaultDragValue))
	{
		m_initialSettingTransform.m_scale = m_transform.m_scale;
	}

	// 行列の計算方法を選択することができるラジオボタンリスト
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
	
	Utility::DeserializeInstanceType<TypeAlias::MatrixStrategyUniqueFactory>(a_rootJson, k_initialMatrixStrategyJsonKey, m_initialSettingTransform.m_matrixStrategy);	

	m_transform.m_scale    = m_initialSettingTransform.m_scale;
	m_transform.m_rotation = m_initialSettingTransform.m_rotation;

	// 外部でInitialTransform.m_positionはデシリアライズを行う(プレハブは座標データをプレハブとして保持しなくてよいから)
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