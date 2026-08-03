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
	Utility::FactoryRadioButtonSelector<TypeAlias::MatrixStrategyUniqueFactory>(k_matrixStrategySelectorLabel, m_matrixStrategy);
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
	ConfrimMatrixStrategy();
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

	ConfrimMatrixStrategy();
}

void FWK::TransformComponent::ConfrimMatrixStrategy()
{
	if (!m_matrixStrategy) { return; }

	m_matrixStrategy->Execute(*this);
}