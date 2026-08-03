#include "TransformComponentJsonConverter.h"

void FWK::Converter::TransformComponentJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, TransformComponent& a_transformComponent) const
{
	if (a_rootJson.is_null()) { return; }

	CommonDeserialize(a_rootJson, a_transformComponent);
}
void FWK::Converter::TransformComponentJsonConverter::DeserializeScene(const nlohmann::json& a_rootJson, TransformComponent& a_transformComponent) const
{
	if (a_rootJson.is_null()) { return; }

	CommonDeserialize(a_rootJson, a_transformComponent);
}

nlohmann::json FWK::Converter::TransformComponentJsonConverter::SerializePrefab(const TransformComponent& a_transformComponent) const
{
	return CommonSerialize(a_transformComponent);
}
nlohmann::json FWK::Converter::TransformComponentJsonConverter::SerializeScene(const TransformComponent& a_transformComponent) const
{
	return CommonSerialize(a_transformComponent);
}

void FWK::Converter::TransformComponentJsonConverter::CommonDeserialize(const nlohmann::json& a_rootJson, TransformComponent& a_transformComponent) const
{
	if (a_rootJson.is_null()) { return; }

	const auto& l_scale    = Utility::DeserializeVector3   (a_rootJson, k_initialScaleJsonKey);
	const auto& l_rotation = Utility::DeserializeQuaternion(a_rootJson, k_initialRotationJsonKey);
	const auto& l_position = Utility::DeserializeVector3   (a_rootJson, k_initialPositionJsonKey);

	a_transformComponent.SetInitialSettingTransformScale   (l_scale);
	a_transformComponent.SetInitialSettingTransformRotation(l_rotation);
	a_transformComponent.SetInitialSettingTransformPosition(l_position);
	
	// 行列合成用クラスを作成するための文字列を取得 
	const auto& l_matrixStrategyTypeName = a_rootJson.value(k_initialMatrixStrategyTypeNameJsonKey, std::string{});

	if (l_matrixStrategyTypeName.empty())
	{
		FWK_ADD_LOG("TransformComponetnのストラテジー初期化用文字列が空になっており、ストラテジーの初期化に失敗しました。");

		return;
	}

	const auto& l_factory = TypeAlias::MatrixStrategyUniqueFactory::GetInstance();

	auto l_matrixStrategy = l_factory.Create(l_matrixStrategyTypeName);

	if (!l_matrixStrategy)
	{
		FWK_ADD_LOG("TransformComponentのMatrixStrategyがうまく作成されていません、jsonファイルを確認してください");
	}

	a_transformComponent.SetInitializeMatrixStrategyTypeName(l_matrixStrategyTypeName);
	a_transformComponent.SetMatrixStrategy                  (std::move(l_matrixStrategy));

	a_transformComponent.SetTransformScale   (l_scale);
	a_transformComponent.SetTransformRotation(l_rotation);
	a_transformComponent.SetTransformPosition(l_position);
}

nlohmann::json FWK::Converter::TransformComponentJsonConverter::CommonSerialize(const TransformComponent& a_transformComponent) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_initialSettingTransform = a_transformComponent.GetREFInitialSettingTransform();

	Utility::UpdateJson(l_rootJson, Utility::SerializeVector3(l_initialSettingTransform.m_scale,       k_initialScaleJsonKey));
	Utility::UpdateJson(l_rootJson, Utility::SerializeQuaternion(l_initialSettingTransform.m_rotation, k_initialRotationJsonKey));
	Utility::UpdateJson(l_rootJson, Utility::SerializeVector3(l_initialSettingTransform.m_position,    k_initialPositionJsonKey));

	l_rootJson[k_initialMatrixStrategyTypeNameJsonKey] = a_transformComponent.GetREFInitialMatrixStrategyName();

	return l_rootJson;
}