#pragma once

namespace FWK::Graphics
{
	// SRVのインデックスなどを扱うためコピー禁止
	class AssetRecordBase
	{
	public:

				 AssetRecordBase() = default;
		virtual ~AssetRecordBase() = default;

		AssetRecordBase(const AssetRecordBase&)			  = delete;
		AssetRecordBase(	  AssetRecordBase&&) noexcept = default;

		AssetRecordBase& operator=(const AssetRecordBase&)			 = delete;
		AssetRecordBase& operator=(		 AssetRecordBase&&) noexcept = default;

		void AddReferenceCount();

		bool SubtractReferenceCount();

		bool IsUnused() const;

		virtual bool ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext) = 0;

		void SetFilePath(const std::wstring& a_set) { m_filePath = a_set; }

		void SetStorageID(const TypeAlias::StorageID& a_set) { m_storageID = a_set; }

		void SetReferenceCount(const std::uint32_t a_set) { m_referenceCount = a_set; };

		const auto& GetREFFilePath() const { return m_filePath; }

		TypeAlias::StorageID GetVALStorageID() const { return m_storageID; }

		std::uint32_t GetVALReferenceCount() const { return m_referenceCount; }

	private:

		std::wstring m_filePath = {};

		TypeAlias::StorageID m_storageID = Constant::k_invalidStorageID;

		std::uint32_t m_referenceCount = Constant::k_emptyAssetReferenceCount;
	};
}