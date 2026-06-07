#include "ResourceContextJsonConverter.h"

void FWK::Converter::ResourceContextJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::ResourceContext& a_resourceContext) const
{
	if (a_rootJson.is_null()) { return; }

	// RTVDescriptorPoolのデシリアライズ、
	if (const auto& l_json = a_rootJson.value(k_rtvDescriptorPoolJsonKey, nlohmann::json{}); 
		!l_json.is_null())
	{
		auto& l_rtvDescriptorPool = a_resourceContext.GetMutableREFRTVDescriptorPool();

		l_rtvDescriptorPool.Deserialize(l_json);
	}

	// SRVDescriptorPoolのデシリアライズ、
	if (const auto& l_json = a_rootJson.value(k_srvDescriptorPoolJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_srvDescriptorPool = a_resourceContext.GetMutableREFSRVDescriptorPool();

		l_srvDescriptorPool.Deserialize(l_json);
	}

	// アップロードシステムのデシリアライズ、
	if (const auto& l_json = a_rootJson.value(k_uploadSystemJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_uploadSystem = a_resourceContext.GetMutableREFUploadSystem();

		l_uploadSystem.Deserialize(l_json);
	}

	// テクスチャシステムのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_textureSystemJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_textureSystem = a_resourceContext.GetMutableREFTextureSystem();

		l_textureSystem.Deserialize(l_json);
	}
}

nlohmann::json FWK::Converter::ResourceContextJsonConverter::Serialize(const Graphics::ResourceContext& a_resourceContext) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_rtvDescriptorPool = a_resourceContext.GetREFRTVDescriptorPool();
	const auto& l_srvDescriptorPool = a_resourceContext.GetREFSRVDescriptorPool();

	const auto& l_textureSystem = a_resourceContext.GetREFTextureSystem();
	const auto& l_uploadSystem  = a_resourceContext.GetREFUploadSystem ();

	// RTVディスクリプタプールのシリアライズ
	l_rootJson[k_rtvDescriptorPoolJsonKey] = l_rtvDescriptorPool.Serialize();

	// SRVディスクリプタプールのシリアライズ
	l_rootJson[k_srvDescriptorPoolJsonKey] = l_srvDescriptorPool.Serialize();

	// UploadSystemのシリアライズ
	l_rootJson[k_uploadSystemJsonKey] = l_uploadSystem.Serialize();

	// テクスチャシステムのシリアライズ
	l_rootJson[k_textureSystemJsonKey] = l_textureSystem.Serialize();

	return l_rootJson;
}