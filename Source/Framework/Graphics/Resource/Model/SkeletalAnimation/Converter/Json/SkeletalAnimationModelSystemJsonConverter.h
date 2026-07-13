#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelSystem;
}

namespace FWK::Converter
{
	class SkeletalAnimationModelSystemJsonConverter final
	{
	public:

		 SkeletalAnimationModelSystemJsonConverter() = default;
		~SkeletalAnimationModelSystemJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::SkeletalAnimationModelSystem& a_skeletalAnimationModelSystem) const;

		nlohmann::json Serialize(const Graphics::SkeletalAnimationModelSystem& a_skeletalAnimationModelSystem) const;

	private:

		static constexpr std::string_view k_modelStorageJsonKey = "ModelStorage";
	};
}