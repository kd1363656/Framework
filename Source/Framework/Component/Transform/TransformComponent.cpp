#include "TransformComponent.h"

FWK::TransformComponent::TransformComponent() :
	m_parentTransformComponent({}),

	m_matrix(),

	m_transform              (),
	m_initialSettingTransform()
{}
FWK::TransformComponent::~TransformComponent() = default;

void FWK::TransformComponent::DeserializePrefab(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	Deserialize(a_rootJson);
}
void FWK::TransformComponent::DeserializeScene(const nlohmann::json& a_rootJson)
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

nlohmann::json FWK::TransformComponent::SerializeScene()
{
	return Serialize();
}
nlohmann::json FWK::TransformComponent::SerializePrefab()
{
	return Serialize();
}

void FWK::TransformComponent::ApplyParent(const std::weak_ptr<GameObject>& a_parentObject)
{
	const auto& l_parent = a_parentObject.lock();

	FWK_ASSERT_RETURN_IF(!l_parent, "親GameObjectが無効なため、Transformの親を適用できませんでした。");

	m_parentTransformComponent = l_parent->GetVALTransformComponent();

	// 親が存在するということは追従する可能性が高いため、自動的に親に追従するように行列を掛ける
	m_matrixStrategy = std::make_unique<HierarchicalMatrixStrartegy>();

	m_initialMatrixStrategyTypeName = std::string(HierarchicalMatrixStrartegy::GetREFTypeINFO().k_name);

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
	
	m_initialMatrixStrategyTypeName = std::string(StandaloneMatrixStrategy::GetREFTypeINFO().k_name);

	ConfrimMatrixStrategy();
}

void FWK::TransformComponent::ConfrimMatrixStrategy()
{
	if (!m_matrixStrategy) { return; }

	m_matrixStrategy->Execute(*this);
}

void FWK::TransformComponent::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_initialSettingTransform.m_scale    = Utility::DeserializeVector3   (a_rootJson, k_initialScaleJsonKey);
	m_initialSettingTransform.m_rotation = Utility::DeserializeQuaternion(a_rootJson, k_initialRotationJsonKey);
	
	m_initialMatrixStrategyTypeName = a_rootJson.value(k_initialMatrixStrategyTypeNameJsonKey, std::string{});

	if (m_initialMatrixStrategyTypeName.empty())
	{
		FWK_ADD_LOG("TransformComponetnのストラテジー初期化用文字列が空になっており、ストラテジーの初期化に失敗しました。");

		return;
	}

	const auto& l_factory = TypeAlias::MatrixStrategyUniqueFactory::GetInstance();

	m_matrixStrategy = l_factory.Create(m_initialMatrixStrategyTypeName);

	m_transform.m_scale    = m_initialSettingTransform.m_scale;
	m_transform.m_rotation = m_initialSettingTransform.m_rotation;

	// 外部でInitialTransform.m_positionはデシリアライズを行う(プレハブは座標データをプレハブとして保持しなくてよいから)
	m_transform.m_position = m_initialSettingTransform.m_position;
}

nlohmann::json FWK::TransformComponent::Serialize()
{
	nlohmann::json l_rootJson = {};

	Utility::UpdateJson(l_rootJson, Utility::SerializeVector3(m_initialSettingTransform.m_scale,       k_initialScaleJsonKey));
	Utility::UpdateJson(l_rootJson, Utility::SerializeQuaternion(m_initialSettingTransform.m_rotation, k_initialRotationJsonKey));
	Utility::UpdateJson(l_rootJson, Utility::SerializeVector3(m_initialSettingTransform.m_position,    k_initialPositionJsonKey));

	l_rootJson[k_initialMatrixStrategyTypeNameJsonKey] = m_initialMatrixStrategyTypeName;

	return l_rootJson;
}