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

	// CBVSRVUAVDescriptorPoolのデシリアライズ、
	if (const auto& l_json = a_rootJson.value(k_cbvSRVUAVDescriptorPoolJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_cbvSRVUAVDescriptorPool = a_resourceContext.GetMutableREFCBVSRVUAVDescriptorPool();

		l_cbvSRVUAVDescriptorPool.Deserialize(l_json);
	}

	// DSVDescriptorPoolのデシリアライズ、
	if (const auto& l_json = a_rootJson.value(k_dsvDescriptorPoolJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_dsvDescriptorPool = a_resourceContext.GetMutableREFDSVDescriptorPool();

		l_dsvDescriptorPool.Deserialize(l_json);
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

	// スタティックモデルシステムのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_staticModelSystemJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_staticModelSystem = a_resourceContext.GetMutableREFStaticModelSystem();

		l_staticModelSystem.Deserialize(l_json);
	}
	
	// スケルタルアニメーションモデルシステムのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_skeletalAnimationModelSystemJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_skeletalAnimationModelSystem = a_resourceContext.GetMutableREFSkeletalAnimationModelSystem();

		l_skeletalAnimationModelSystem.Deserialize(l_json);
	}
}

nlohmann::json FWK::Converter::ResourceContextJsonConverter::Serialize(const Graphics::ResourceContext& a_resourceContext) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_rtvDescriptorPool       = a_resourceContext.GetREFRTVDescriptorPool      ();
	const auto& l_cbvSRVUAVDescriptorPool = a_resourceContext.GetREFCBVSRVUAVDescriptorPool();
	const auto& l_dsvDescriptorPool       = a_resourceContext.GetREFDSVDescriptorPool      ();

	const auto& l_uploadSystem                 = a_resourceContext.GetREFUploadSystem                ();
	const auto& l_textureSystem                = a_resourceContext.GetREFTextureSystem               ();
	const auto& l_staticModelSystem            = a_resourceContext.GetREFStaticModelSystem           ();
	const auto& l_skeletalAnimationModelSystem = a_resourceContext.GetREFSkeletalAnimationModelSystem();

	// RTVディスクリプタプールのシリアライズ
	l_rootJson[k_rtvDescriptorPoolJsonKey] = l_rtvDescriptorPool.Serialize();

	// SRVディスクリプタプールのシリアライズ
	l_rootJson[k_cbvSRVUAVDescriptorPoolJsonKey] = l_cbvSRVUAVDescriptorPool.Serialize();

	// DSVディスクリプタプールのシリアライズ
	l_rootJson[k_dsvDescriptorPoolJsonKey] = l_dsvDescriptorPool.Serialize();

	// UploadSystemのシリアライズ
	l_rootJson[k_uploadSystemJsonKey] = l_uploadSystem.Serialize();

	// テクスチャシステムのシリアライズ
	l_rootJson[k_textureSystemJsonKey] = l_textureSystem.Serialize();

	// スタティックモデルシステムのシリアライズ
	l_rootJson[k_staticModelSystemJsonKey] = l_staticModelSystem.Serialize();

	// スケルタルアニメーションシステムのシリアライズ
	l_rootJson[k_skeletalAnimationModelSystemJsonKey] = l_skeletalAnimationModelSystem.Serialize();

	return l_rootJson;
}