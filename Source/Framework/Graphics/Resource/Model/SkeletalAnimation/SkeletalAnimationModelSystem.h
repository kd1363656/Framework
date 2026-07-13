#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModelSystem final
	{
	public:

		 SkeletalAnimationModelSystem() = default;
		~SkeletalAnimationModelSystem() = default;

		bool LoadSkeletalAnimationModelAsset(const std::filesystem::path& a_filePath, SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord);

	private:

		bool BuildSkeletalAnimationModelAssetData(const std::filesystem::path& a_filePath, SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord);

		SkeletalAnimationModelFBXLoader                                  m_loader                        = {};
		ModelMaterialRuntimeTextureBuilder<SkeletalAnimationModelRecord> m_materialRuntimeTextureBuilder = {};

		ModelMeshOptimizer<SkeletalAnimationModelRecord>  m_meshOptimizer  = {};
		ModelMeshletBuilder<SkeletalAnimationModelRecord> m_meshletBuilder = {};

		Converter::SkeletalAnimationModelBinaryConverter m_binaryConverter = {};
	};
}