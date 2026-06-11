#include "ConstantBufferUploaderJsonConverter.h"

void FWK::Converter::ConstantBufferUploaderBaseJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::ConstantBufferUploaderBase& a_constantBufferUploaderBase) const
{
	if (a_rootJson.is_null()) { return; }

	const auto& l_createConstantBufferCount = a_rootJson.value(k_createConstantBufferCountJsonKey, a_constantBufferUploaderBase.GetREFInvalidCreateConsntantBufferCount());

	a_constantBufferUploaderBase.SetCreateConstantBufferCount(l_createConstantBufferCount);
}

nlohmann::json FWK::Converter::ConstantBufferUploaderBaseJsonConverter::Serialize(const Graphics::ConstantBufferUploaderBase& a_constantBufferUploaderBase) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_createConstantBufferCountJsonKey] = a_constantBufferUploaderBase.GetREFCreateConstantBufferCount();

	return l_rootJson;
}