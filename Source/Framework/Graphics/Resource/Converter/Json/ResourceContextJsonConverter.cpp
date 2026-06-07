#include "ResourceContextJsonConverter.h"

void FWK::Converter::ResourceContextJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::ResourceContext& a_resourceContext) const
{
	if (a_rootJson.is_null()) { return; }

	auto& l_rtvDescriptorPool = a_resourceContext.GetMutableREFRTVDescriptorPool();
	auto& l_srvDescriptorPool = a_resourceContext.GetMutableREFSRVDescriptorPool();

	const auto& l_rtvDescriptorPoolJson = a_rootJson.value(k_rtvDescriptorPoolJsonKey, nlohmann::json{});
	const auto& l_srvDescriptorPoolJson = a_rootJson.value(k_srvDescriptorPoolJsonKey, nlohmann::json{});

	if (!l_rtvDescriptorPoolJson.is_null())
	{
		l_rtvDescriptorPool.Deserialize(l_rtvDescriptorPoolJson);
	}

	if (!l_srvDescriptorPoolJson.is_null())
	{
		l_srvDescriptorPool.Deserialize(l_rtvDescriptorPoolJson);
	}
}

nlohmann::json FWK::Converter::ResourceContextJsonConverter::Serialize(const Graphics::ResourceContext& a_resourceContext) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();
	const auto& l_srvDescriptorPool = a_resourceContext.GetREFSRVDescriptorPool();

	// RTVディスクリプタプールのシリアライズ
	l_rootJson[k_rtvDescriptorPoolJsonKey] = l_rtvDescriptorPool.Serialize();

	// SRVディスクリプタプールのシリアライズ
	l_rootJson[k_srvDescriptorPoolJsonKey] = l_srvDescriptorPool.Serialize();

	return l_rootJson;
}