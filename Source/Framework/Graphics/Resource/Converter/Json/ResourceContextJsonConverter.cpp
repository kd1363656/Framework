#include "ResourceContextJsonConverter.h"

void FWK::Converter::ResourceContextJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::ResourceContext& a_resourceContext) const
{
	if (a_rootJson.is_null()) { return; }

	auto& l_rtvDescriptorPool = a_resourceContext.GetMutableREFRTVDescriptorPool();

	const auto& l_rtvDescriptorPoolJson = a_rootJson.value(k_rtvDescriptorPoolJsonKey, nlohmann::json{});

	if (!l_rtvDescriptorPoolJson.is_null())
	{
		l_rtvDescriptorPool.Deserialize(l_rtvDescriptorPoolJson);
	}
}

nlohmann::json FWK::Converter::ResourceContextJsonConverter::Serialize(const Graphics::ResourceContext& a_resourceContext) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();

	// RTVディスクリプタプールのシリアライズ
	l_rootJson[k_rtvDescriptorPoolJsonKey] = l_rtvDescriptorPool.Serialize();

	return l_rootJson;
}