#include "ModelComponentBaseJsonConverter.h"

void FWK::Converter::ModelComponentBaseJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, ModelComponentBase& a_modelComponentBase) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_assetFilePathHelperJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		const auto& l_assetFilePathHelper = a_modelComponentBase.GetVALAssetFilePathHelper().lock();

		if (l_assetFilePathHelper)
		{
			l_assetFilePathHelper->Deserialize(l_json);
		}
	}
}

nlohmann::json FWK::Converter::ModelComponentBaseJsonConverter::SerializePrefab(const ModelComponentBase& a_modelComponentBase) const
{
	nlohmann::json l_rootJson = {};

	if (const auto& l_assetFilePathHelper = a_modelComponentBase.GetVALAssetFilePathHelper().lock();
		l_assetFilePathHelper)
	{
		l_rootJson[k_assetFilePathHelperJsonKey] = l_assetFilePathHelper->Serialize();
	}

	return l_rootJson;
}