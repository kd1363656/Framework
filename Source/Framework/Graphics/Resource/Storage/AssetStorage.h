#pragma once

namespace FWK::Graphics
{
	template <Concept::IsDerivedAssetRecordBaseConcept RecordType>
	class AssetStorage final
	{
	private:

		using RecordMap = std::unordered_map<std::wstring, std::shared_ptr<RecordType>>;

	public:

		 AssetStorage() = default;
		~AssetStorage() = default;

		void Deserialize(const nlohmann::json& a_rootJson)
		{
			if (a_rootJson.is_null()) { return; }
			m_jsonConverter.Deserialize(a_rootJson, *this);
		}
		bool Create()
		{
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_storageIDAllocator.Create(), "StorageIDAllocatorの作成に失敗したため、AssetStorageの作成処理に失敗しました。", false);

			return true;
		}

		nlohmann::json Serialize() const
		{
			return m_jsonConverter.Serialize(*this);
		}

		TypeAlias::StorageID AllocateStorageID()
		{
			return m_storageIDAllocator.Allocate();
		}

		void ReleaseStorageID(const TypeAlias::StorageID a_storageID)
		{
			m_storageIDAllocator.Release(a_storageID);
		}

		bool RegisterRecord(const std::shared_ptr<RecordType>& a_record, const std::wstring& a_filePath)
		{
			// レコード情報が無効じゃないか、無効なストレージIDでないかをチェック
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_record,												   "レコードのインスタンス化がされておらず、Recordの登録に失敗しました。",			 false);
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_filePath.empty(),										   "ファイルパスが空のため、Recordの登録に失敗しました。",							 false);
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_record->GetVALStorageID() == Constant::k_invalidStorageID, "StorageIDが無効のため、Recordの登録に失敗しました。",							 false);
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_recordMap.contains(a_filePath),							   "同じファイルパスのRecordが既に登録されているため、Recordの登録に失敗しました。", false);

			m_recordMap.try_emplace(a_filePath, a_record);

			return true;
		}

		bool AddReference(const std::weak_ptr<RecordType>& a_record)
		{
			const auto& l_record = a_record.lock();

			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_record, "指定されたStorageIDのRecordが見つからないため、参照数加算に失敗しました。", false);

			// 参照カウントを加算
			l_record->AddReferenceCount();

			return true;
		}

		bool ReleaseReference(const std::weak_ptr<RecordType>& a_record, const DirectCommandQueue& a_directCommandQueue, ResourceReleaseContext& a_resourceReleaseContext)
		{
			const auto& l_record = a_record.lock();

			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_record,						   "指定されたStorageIDのRecordが見つからないため、解放予約に失敗しました。", false);
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_record->SubtractReferenceCount(), "Recordの参照数減算に失敗ており、解放予約に失敗しました。",				  false);

			// まだ利用者が残っているなら何もしない
			if (!l_record->IsUnused()) { return true; }

			// Record自身に、GPUResourceやDescriptorIndexを遅延解放Queueへ積ませる
			const auto& l_lastSignaledFenceValue = a_directCommandQueue.FetchREFLastSignaledFenceValue();

			FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_record->ReserveRelease(l_lastSignaledFenceValue, a_resourceReleaseContext), "Record固有リソースの遅延解放登録に失敗しました。", false);

			const auto& l_filePath = l_record->GetREFFilePath ();
			
			// マップで管理していたレコードのストレージIDをリリース
			if (const auto  l_storageID = l_record->GetVALStorageID();
				l_storageID != Constant::k_invalidStorageID)
			{
				m_storageIDAllocator.Release(l_storageID);
			}

			// マップから削除
			m_recordMap.erase(l_filePath);

			return true;
		}

		TypeAlias::StorageID FindVALStorageIDFromFilePath(const std::wstring& a_filePath) const
		{
			// ファイルパスからストレージクラスを取得
			const auto& l_itr = m_recordMap.find(a_filePath);

			if (l_itr == m_recordMap.end()) { return Constant::k_invalidStorageID; }

			const auto& l_record = l_itr->second;

			if (!l_record) { return Constant::k_invalidStorageID; }

			// ストレージクラスからストレージIDを取得
			return l_record->GetVALStorageID();
		}

		std::weak_ptr<RecordType> FindVALRecord(const std::wstring& a_filePath) const
		{
			FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_filePath.empty(), "ファイルパスが空のため、Recordの取得に失敗しました。", {});

			const auto& l_itr = m_recordMap.find(a_filePath);

			if (l_itr == m_recordMap.end()) { return {}; }

			return l_itr->second;
		}

		const auto& GetREFStorageIDAllocator() const { return m_storageIDAllocator; }

		auto& GetMutableREFStorageIDAllocator() { return m_storageIDAllocator; }

	private:

		RecordMap m_recordMap = {};

		StorageIDAllocator m_storageIDAllocator = {};

		Converter::AssetStorageJsonConverter<RecordType> m_jsonConverter = {};
	};
}