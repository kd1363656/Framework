#include "DynamicBufferUploaderJsonConverter.h"

void FWK::Converter::DynamicBufferUploaderBaseJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::DynamicBufferUploaderBase& a_dynamicBufferUploaderBase) const
{
	if (a_rootJson.is_null()) { return; }

	const auto& l_createCount = a_rootJson.value(k_createCountJsonKey, a_dynamicBufferUploaderBase.k_invalidCreateCount);

	a_dynamicBufferUploaderBase.SetCreateCount(l_createCount);
}

nlohmann::json FWK::Converter::DynamicBufferUploaderBaseJsonConverter::Serialize(const Graphics::DynamicBufferUploaderBase& a_dynamicBufferUploaderBase) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_createCountJsonKey] = a_dynamicBufferUploaderBase.GetREFCreateCount();

	return l_rootJson;
}