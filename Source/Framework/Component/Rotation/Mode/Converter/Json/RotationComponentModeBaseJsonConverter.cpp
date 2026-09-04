#include "RotationComponentModeBaseJsonConverter.h"

void FWK::Converter::RotationComponentModeBaseJsonConverter::Deserialize(const nlohmann::json& a_rootJson, RotationComponentModeBase& a_rotationComponentModeBase) const
{
	if (a_rootJson.is_null()) { return; }

	// 回転を適用する軸のリストを復元する
	if (const auto& l_json = a_rootJson.value(k_rotationApplyAxisListJsonKey, nlohmann::json{});
		!l_json.is_null() &&
		Utility::IsJsonArray(l_json))
	{
		DeserializeRotationApplyAxisList(l_json, a_rotationComponentModeBase);
	}
}

nlohmann::json FWK::Converter::RotationComponentModeBaseJsonConverter::Serialize(const RotationComponentModeBase& a_rotationComponentModeBase) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_rotationApplyAxisListJsonKey] = SerializeRotationApplyAxisList(a_rotationComponentModeBase);

	return l_rootJson;
}

void FWK::Converter::RotationComponentModeBaseJsonConverter::DeserializeRotationApplyAxisList(const nlohmann::json& a_rootJson, RotationComponentModeBase& a_rotationComponentModeBase) const
{
	if (a_rootJson.is_null() ||
		!Utility::IsJsonArray(a_rootJson))
	{
		return;
	}

	for (const auto& l_json : a_rootJson)
	{
		if (l_json.is_null()) { continue; }

		const auto l_rotationApplyAxis = l_json.value(k_rotationApplyAxisJsonKey, Enum::Axis::Invalid);

		if (l_rotationApplyAxis == Enum::Axis::Invalid) { continue; }

		a_rotationComponentModeBase.AddRotationApplyAxis(l_rotationApplyAxis);
	}
}

nlohmann::json FWK::Converter::RotationComponentModeBaseJsonConverter::SerializeRotationApplyAxisList(const RotationComponentModeBase& a_rotationComponentModeBase) const
{
	      auto  l_rootJsonArray         = nlohmann::json::array                                  ();
	const auto& l_rotationApplyAxisList = a_rotationComponentModeBase.GetREFRotationApplyAxisList();

	for (const auto& l_rotationApplyAxis : l_rotationApplyAxisList)
	{
		if (l_rotationApplyAxis == Enum::Axis::Invalid) { continue; }

		nlohmann::json l_json = {};

		l_json[k_rotationApplyAxisJsonKey] = l_rotationApplyAxis;

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}