#include "SkeletalAnimationModelSystemJsonConverter.h"

void FWK::Converter::SkeletalAnimationModelSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::SkeletalAnimationModelSystem& a_skeletalAnimationModelSystem) const
{
	if (a_rootJson.is_null()) { return; }

	if (const auto& l_json = a_rootJson.value(k_modelStorageJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_modelStorage = a_skeletalAnimationModelSystem.GetMutableREFModelStorage();

		l_modelStorage.Deserialize(l_json);
	}
}

nlohmann::json FWK::Converter::SkeletalAnimationModelSystemJsonConverter::Serialize(const Graphics::SkeletalAnimationModelSystem & a_skeletalAnimationModelSystem) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_modelStorage = a_skeletalAnimationModelSystem.GetREFModelStorage();

	l_rootJson[k_modelStorageJsonKey] = l_modelStorage.Serialize();

	return l_rootJson;
}