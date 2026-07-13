#pragma once

namespace FWK::Graphics
{
	class SkeletalAnimationModel final
	{
	public:

         SkeletalAnimationModel();
		 SkeletalAnimationModel(const SkeletalAnimationModel&  a_other);
		 SkeletalAnimationModel(      SkeletalAnimationModel&& a_other) noexcept;
		~SkeletalAnimationModel();

		SkeletalAnimationModel& operator=(const SkeletalAnimationModel&  a_other);
		SkeletalAnimationModel& operator=(      SkeletalAnimationModel&& a_other) noexcept;

		bool Load(const std::filesystem::path& a_filePath);

		bool IsValid() const;

		const auto& GetREFSkeletalAnimationModelRecord() const { return m_skeletalAnimationModelRecord; }

		auto GetVALStorageID() const { return m_storageID; }

	private:

		void AddReferenceCount() const;

		void SubtractReferenceCount();

		std::weak_ptr<Graphics::SkeletalAnimationModelRecord> m_skeletalAnimationModelRecord;

		TypeAlias::StorageID m_storageID;
	};
}