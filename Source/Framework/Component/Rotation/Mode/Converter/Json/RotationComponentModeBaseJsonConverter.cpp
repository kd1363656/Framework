#include "RotationComponentModeBaseJsonConverter.h"

void FWK::Converter::RotationComponentModeBaseJsonConverter::Deserialize(const nlohmann::json& a_rootJson, RotationComponentModeBase& a_rotationComponentModeBase) const
{
	if (a_rootJson.is_null()) { return; }

	// 回転を適用する軸のリストを復元する
	if (const auto& l_json = a_rootJson.value(k_canApplyRotationAxisListJsonKey, nlohmann::json{});
		!l_json.is_null() &&
		Utility::IsJsonArray(l_json))
	{
		DeserializeCanApplyRotationAxisList(l_json, a_rotationComponentModeBase);
	}
}

nlohmann::json FWK::Converter::RotationComponentModeBaseJsonConverter::Serialize(const RotationComponentModeBase& a_rotationComponentModeBase) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_canApplyRotationAxisListJsonKey] = SerializeCanApplyRotationAxisList(a_rotationComponentModeBase);

	return l_rootJson;
}

void FWK::Converter::RotationComponentModeBaseJsonConverter::DeserializeCanApplyRotationAxisList(const nlohmann::json& a_rootJson, RotationComponentModeBase& a_rotationComponentModeBase) const
{
	if (a_rootJson.is_null() ||
		!Utility::IsJsonArray(a_rootJson))
	{
		return;
	}

	for (const auto& l_json : a_rootJson)
	{
		if (l_json.is_null()) { continue; }

		const auto l_canApplyRotationAxis = l_json.value(k_canApplyRotationAxisJsonKey, Enum::Axis::Invalid);

		if (l_canApplyRotationAxis == Enum::Axis::Invalid) { continue; }

		a_rotationComponentModeBase.AddCanApplyRotationAxis(l_canApplyRotationAxis);
	}
}

nlohmann::json FWK::Converter::RotationComponentModeBaseJsonConverter::SerializeCanApplyRotationAxisList(const RotationComponentModeBase& a_rotationComponentModeBase) const
{
		  auto  l_rootJsonArray            = nlohmann::json::array                                     ();
	const auto& l_canApplyRotationAxisList = a_rotationComponentModeBase.GetREFCanApplyRotationAxisList();

	for (const auto& l_canApplyRotationAxis : l_canApplyRotationAxisList)
	{
		if (l_canApplyRotationAxis == Enum::Axis::Invalid) { continue; }

		nlohmann::json l_json = {};

		l_json[k_canApplyRotationAxisJsonKey] = l_canApplyRotationAxis;

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}