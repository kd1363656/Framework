//#pragma once
//
//namespace FWK::Converter
//{
//	class SkeletalAnimationModelBinaryConverter final : public BinaryFileConverterBase
//	{
//	private:
//
//		struct SkeletalAnimationModelBinaryHeader final
//		{
//			std::uint64_t m_fileSize    = k_emptyAssetFileSize;
//			std::uint16_t m_version     = k_skeletalAnimationModelAssetVersion;
//			std::uint16_t m_assetTypeID = k_skeletalAnimationModelAssetTypeID;
//
//			std::uint64_t m_boneCount           = k_emptyBoneCount;
//			std::uint64_t m_motionSequenceCount = k_emptyMotionSequenceCount;
//		};
//
//		struct SkeletalAnimationModelBoneBinaryHeader final
//		{
//			std::uint64_t m_boneNameSize = k_emptyBoneNameSize;
//
//			std::uint32_t m_parentBoneIndex = Graphics::SkeletalAnimationModelRecord::k_invalidBoneIndex;
//		};
//
//		struct SkeletalMotionSequenceBinaryHeader final
//		{
//			float m_durationSecond = Graphics::SkeletalAnimationModelRecord::k_initialAnimationDurationSecond;
//			float m_frameRate      = Graphics::SkeletalAnimationModelRecord::k_defaultAnimationFrameRate;
//
//			std::uint64_t m_motionNameSize = k_emptyMotionNameSize;
//
//			std::uint64_t m_boneMotionTrackCount = k_emptyBoneMotionTrackCount;
//		};
//
//		struct SkeletalAnimationBoneMotionTrackBinaryHeader final
//		{
//			std::uint64_t m_keyFrameCount = k_emptyKeyFrameCount;
//
//			std::uint32_t m_boneIndex = Graphics::SkeletalAnimationModelRecord::k_invalidBoneIndex;
//		};
//
//	public:
//
//		 SkeletalAnimationModelBinaryConverter()          = default;
//		~SkeletalAnimationModelBinaryConverter() override = default;
//
//		bool LoadSkeletalAnimationModelAsset(const std::filesystem::path& a_filePath, Graphics::SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord);
//		bool SaveSkeletalAnimationModelAsset(const std::filesystem::path& a_filePath, Graphics::SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord);
//
//		static constexpr std::uint64_t k_emptyBoneCount            = 0ULL;
//		static constexpr std::uint64_t k_emptyMotionSequenceCount  = 0ULL;
//		static constexpr std::uint64_t k_emptyBoneMotionTrackCount = 0ULL;
//		static constexpr std::uint64_t k_emptyKeyFrameCount        = 0ULL;
//
//	private:
//
//		bool CanLoadSkeletalAnimationModelAsset(const std::filesystem::path& a_filePath) const;
//
//		void FailLoadSkeletalAnimationModelAsset(Graphics::SkeletalAnimationModelRecord::SkeletalAnimationModelData& a_skeletalAnimationModelData, const std::uint64_t& a_fileSize) const;
//
//		SkeletalAnimationModelBinaryHeader CreateSkeletalAnimationModelBinaryHeader(const Graphics::SkeletalAnimationModelRecord::SkeletalAnimationModelData& a_skeletalAnimationModelData, const std::uint64_t& a_fileSize) const;
//
//		SkeletalAnimationModelBoneBinaryHeader CreateSkeletalAnimationMoelBoneBinaryHeader(const Graphics::SkeletalAnimationModelRecord::SkeletalAnimationModelBone& a_skeletalAnimationModelBone) const;
//
//		SkeletalAnimationBoneMotionTrackBinaryHeader CreateSkeletalAnimationModelBoneMotionTrackBinaryHeader(const Graphics::SkeletalAnimationModelRecord::SkeletalAnimationModelBoneMotionTrack& a_boneMotionTrack) const;
//
//		std::uint64_t CalculateSkeletalAnimationModelAssetFileSize(const Graphics::SkeletalAnimationModelRecord::SkeletalAnimationModelData& a_skeletalAnimationData) const;
//
//		static constexpr std::uint64_t k_emptyBoneNameSize   = 0ULL;
//		static constexpr std::uint64_t k_emptyMotionNameSize = 0ULL;
//
//		// 'S' = 0x53、'A' = 0x41のため、0x5341で"SA"を表す
//		static constexpr std::uint16_t k_skeletalAnimationModelAssetTypeID = 0x5341U;
//
//		static constexpr std::uint16_t k_skeletalAnimationModelAssetVersion = 1U;
//	};
//}