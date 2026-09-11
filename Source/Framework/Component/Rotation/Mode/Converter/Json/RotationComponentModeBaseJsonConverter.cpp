#include "RotationComponentModeBaseJsonConverter.h"

void FWK::Converter::RotationComponentModeBaseJsonConverter::Deserialize(const nlohmann::json& a_rootJson, RotationComponentModeBase& a_moveComponentModeBase) const
{
	if (a_rootJson.is_null()) { return; }

	// 回転を適用する軸のリストを復元する
	if (const auto& l_json = a_rootJson.value(k_canApplyRotationAxisBitShiftFlagListJsonKey, nlohmann::json{});
		!l_json.is_null() &&
		Utility::IsJsonArray(l_json))
	{
		DeserializeCanApplyRotationAxisBitShiftFlagList(l_json, a_moveComponentModeBase);
	}
}

nlohmann::json FWK::Converter::RotationComponentModeBaseJsonConverter::Serialize(const RotationComponentModeBase& a_moveComponentModeBase) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_canApplyRotationAxisBitShiftFlagListJsonKey] = SerializeCanApplyRotationAxisBitShiftFlagList(a_moveComponentModeBase);

	return l_rootJson;
}

void FWK::Converter::RotationComponentModeBaseJsonConverter::DeserializeCanApplyRotationAxisBitShiftFlagList(const nlohmann::json& a_rootJson, RotationComponentModeBase& a_moveComponentModeBase) const
{
	if (a_rootJson.is_null() ||
		!Utility::IsJsonArray(a_rootJson))
	{
		return;
	}

	for (const auto& l_json : a_rootJson)
	{
		if (l_json.is_null()) { continue; }

		const auto l_canApplyRotationAxis = l_json.value(k_canApplyRotationAxisJsonKey, Enum::AxisBitShiftFlag::Invalid);

		if (l_canApplyRotationAxis == Enum::AxisBitShiftFlag::Invalid) { continue; }

		a_moveComponentModeBase.AddCanApplyRotationAxisBitShiftFlag(l_canApplyRotationAxis);
	}
}

nlohmann::json FWK::Converter::RotationComponentModeBaseJsonConverter::SerializeCanApplyRotationAxisBitShiftFlagList(const RotationComponentModeBase& a_moveComponentModeBase) const
{
		  auto  l_rootJsonArray                        = nlohmann::json::array                                             ();
	const auto& l_canApplyRotationAxisBitShiftFlagList = a_moveComponentModeBase.GetREFCanApplyRotationAxisBitShiftFlagList();

	for (const auto& l_canApplyRotationAxisBitShiftFlag : l_canApplyRotationAxisBitShiftFlagList)
	{
		if (l_canApplyRotationAxisBitShiftFlag == Enum::AxisBitShiftFlag::Invalid) { continue; }

		nlohmann::json l_json = {};

		l_json[k_canApplyRotationAxisJsonKey] = l_canApplyRotationAxisBitShiftFlag;

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}