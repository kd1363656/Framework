#include "SkeletalAnimationModelSystemJsonConverter.h"

void FWK::Converter::SkeletalAnimationModelSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::SkeletalAnimationModelSystem& a_skeletalAnimationModelSystem) const
{
	if (a_rootJson.is_null()) { return; }

	if (a_rootJson.contains(k_modelStorageJsonKey))
	{
		auto& l_staticModelSystem = a_skeletalAnimationModelSystem.GetMutableREFModelStorage();

		l_staticModelSystem.Deserialize(a_rootJson[k_modelStorageJsonKey]);
	}
}

nlohmann::json FWK::Converter::SkeletalAnimationModelSystemJsonConverter::Serialize(const Graphics::SkeletalAnimationModelSystem & a_skeletalAnimationModelSystem) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_staticModelSystem = a_staticModelSystem.GetREFStaticModelStorage();

	l_rootJson[k_modelStorageJsonKey] = l_staticModelSystem.Serialize();

	return l_rootJson;
}