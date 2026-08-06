#include "TransformComponent.h"

void FWK::TransformComponent::DeserializePrefab(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.DeserializePrefab(a_rootJson, *this);
}
void FWK::TransformComponent::DeserializeScene(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.DeserializeScene(a_rootJson, *this);
}

void FWK::TransformComponent::PostDeserialize()
{
	const auto& l_owner = GetREFOwner().lock();

	if (!l_owner) { return; }

	// 親が存在するなら親のTransformComponentをキャッシュする
	if (const auto& l_parent = l_owner->GetREFParent().lock())
	{
		m_parentTransformComponent = l_parent->GetVALTransformComponent();
	}

	ConfrimMatrix();
}

void FWK::TransformComponent::PostLateUpdate()
{
	ConfrimMatrix();
}

void FWK::TransformComponent::EditInspector()
{
	m_inspector.EditInspector(*this);
}

nlohmann::json FWK::TransformComponent::SerializePrefab()
{
	return m_jsonConverter.SerializePrefab(*this);
}
nlohmann::json FWK::TransformComponent::SerializeScene()
{
	return m_jsonConverter.SerializeScene(*this);
}

void FWK::TransformComponent::ApplyParent(const std::weak_ptr<GameObject>& a_parentObject)
{
	const auto& l_parent = a_parentObject.lock();

	FWK_ASSERT_RETURN_IF(!l_parent, "親GameObjectが無効なため、Transformの親を適用できませんでした。");

	m_parentTransformComponent = l_parent->GetVALTransformComponent();

	// 親が存在するということは追従する可能性が高いため、自動的に親に追従するように行列を掛ける
	m_matrixStrategy = std::make_unique<HierarchicalMatrixStrartegy>();

	m_initializeMatrixStrategyTypeName = std::string(HierarchicalMatrixStrartegy::GetREFTypeINFO().k_name);

	// セットした後にダーティーフラグで行列の更新が妨げられてもいいように
	// ここで一度だけ行列を更新しておく
	ConfrimMatrix();
}
void FWK::TransformComponent::ApplyStandalone()
{
	const auto& l_position = m_matrix.Translation();

	m_parentTransformComponent.reset();

	// Transformに前の親の回転率、スケール、座標を考慮した行列を格納する
	m_transform.m_position = l_position;

	// Scene保存時にも解除後の位置を保存できるように
	// シリアライズ対象の初期Transformにも反映する
	m_initialSettingTransform.m_position = l_position;

	// 親から外れたので、
	// 単独GameObject用の行列計算方式へ戻す
	m_matrixStrategy = std::make_unique<StandaloneMatrixStrategy>();
	
	m_initializeMatrixStrategyTypeName = std::string(StandaloneMatrixStrategy::GetREFTypeINFO().k_name);

	ConfrimMatrix();
}

void FWK::TransformComponent::ConfrimMatrix()
{
	if (!m_matrixStrategy) { return; }

	m_matrixStrategy->Execute(*this);
}