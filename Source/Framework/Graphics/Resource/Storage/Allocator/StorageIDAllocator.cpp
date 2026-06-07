#include "StorageIDAllocator.h"

void FWK::Graphics::StorageIDAllocator::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }
	m_storageAllocatorJsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::StorageIDAllocator::Create()
{
	// 無効値を容量として指定された場合は作成失敗とする
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_storageIDCapacity == Constant::k_invalidStorageIDCapacity, "ストレージIDの割り当て可能数が0になっており、作成処理に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_storageIDCapacity == Constant::k_invalidStorageID,         "StorageIDの割り当て可能数が無効値です、作成処理に失敗しました。",       false);

	m_nextStorageID = k_initialNextStorageID;

	// 全スロットを未使用状態で初期化する
	m_isAllocatedList.assign(m_storageIDCapacity, false);

	// キューも何も保持していない状態にする
	m_freeStorageIDQueue = {};

	return true;
}

nlohmann::json FWK::Graphics::StorageIDAllocator::Serialize() const
{
	return m_storageAllocatorJsonConverter.Serialize(*this);
}

void FWK::Graphics::StorageIDAllocator::Release(const TypeAlias::StorageID a_storageID)
{
	// 範囲外StorageIDの解放は不正
	FWK_ASSERT_RETURN_IF_FAILED(!IsValidStorageID(a_storageID), "解放しようとしたStorageIDが確保範囲外となっており、解放処理に失敗しました。");

	// 未使用スロットの二重解放を防ぐ
	FWK_ASSERT_RETURN_IF_FAILED(!m_isAllocatedList[a_storageID], "未使用のStorageIDを解放しようとしており、解放処理に失敗しました。");

	m_isAllocatedList[a_storageID] = false;
	m_freeStorageIDQueue.push(a_storageID);
}

FWK::TypeAlias::StorageID FWK::Graphics::StorageIDAllocator::Allocate()
{
	// 解放済みスロットがあればそれを優先再利用する
	if (!m_freeStorageIDQueue.empty())
	{
		const TypeAlias::StorageID l_reuseStorageID = m_freeStorageIDQueue.front();

		m_freeStorageIDQueue.pop();

		// 有効なインデックスかどうかを確認
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!IsValidStorageID(l_reuseStorageID), "再利用しようとしたストレージIDが確保範囲外となっており、アロケート処理に失敗しました。", Constant::k_invalidStorageID);

		m_isAllocatedList[l_reuseStorageID] = true;

		return l_reuseStorageID;
	}

	// 未使用領域が残っているなら新規払い出しする
	if (IsValidStorageID(m_nextStorageID))
	{
		const auto l_allocateStorageID = m_nextStorageID;

		++m_nextStorageID;

		// 新規払い出しするインデックス番号は割り当て済みにする
		m_isAllocatedList[l_allocateStorageID] = true;

		return l_allocateStorageID;
	}

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(true, "StorageIDの空きがなくなり、アロケート処理に失敗しました。", Constant::k_invalidStorageID);
}

bool FWK::Graphics::StorageIDAllocator::IsValidStorageID(const TypeAlias::StorageID a_storageID) const
{
	// 範囲外インデックスを指し示すならfalseを返す
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_storageID >= m_storageIDCapacity ||
									  a_storageID >= static_cast<TypeAlias::StorageID>(m_isAllocatedList.size()),
									  "ストレージIDが無効な値であることを検知しました。",
									  false);

	return true;
}