#include "NodeEditorAllocator.h"

void FWK::NodeEditorAllocator::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::NodeEditorAllocator::Create()
{
	// 無効値を容量として指定された場合は作成失敗とする
	FWK_ASSERT_RETURN_VALUE_IF(m_nodeEditorIDCapacity == Constant::k_invalidNodeEditorID, "NodeEditorIDの割り当て可能数が無効値です、作成処理に失敗しました。",    false);
	FWK_ASSERT_RETURN_VALUE_IF(m_nodeEditorIDCapacity == k_invalidEditorIDCapacity,       "NodeEditorIDの割り当て可能数が0になっており、作成処理に失敗しました。", false);

	m_nextNodeEditorID = k_initialNextEditorID;

	// 全スロットを未使用状態で初期化する
	m_isAllocatedList.assign(static_cast<std::size_t>(m_nodeEditorIDCapacity), false);
	
	// キューも何も保持していない状態にする
	m_freeNodeEditorIDQueue = {};

	return true;
}

nlohmann::json FWK::NodeEditorAllocator::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

FWK::TypeAlias::NodeEditorID FWK::NodeEditorAllocator::Allocate()
{
	// 解放済みスロットがあればそれを優先再利用する
	if (!m_freeNodeEditorIDQueue.empty())
	{
		const int l_reuseStorageID = m_freeNodeEditorIDQueue.front();

		m_freeNodeEditorIDQueue.pop();

		// 有効なインデックスかどうかを確認
		FWK_ASSERT_RETURN_VALUE_IF(!IsValidNodeEditorID(l_reuseStorageID), "再利用しようとしたFreeNodeIDが確保範囲外となっており、アロケート処理に失敗しました。", Constant::k_invalidNodeEditorID);

		m_isAllocatedList[l_reuseStorageID] = true;

		return l_reuseStorageID;
	}

	// 未使用領域が残っているなら新規払い出しする
	if (IsValidNodeEditorID(m_nextNodeEditorID))
	{
		const auto l_allocateStorageID = m_nextNodeEditorID;

		++m_nextNodeEditorID;

		// 新規払い出しするインデックス番号は割り当て済みにする
		m_isAllocatedList[l_allocateStorageID] = true;

		return l_allocateStorageID;
	}

	FWK_ASSERT_RETURN_VALUE_IF(true, "NodeEditorIDの空きがなくなり、アロケート処理に失敗しました。", Constant::k_invalidNodeEditorID);
}

void FWK::NodeEditorAllocator::Release(const TypeAlias::NodeEditorID a_nodeEditorID)
{
	// 範囲外NodeEditorIDの解放は不正
	FWK_ASSERT_RETURN_IF(!IsValidNodeEditorID(a_nodeEditorID), "解放しようとしたNodeEditorIDが確保範囲外となっており、解放処理に失敗しました。");

	// 未使用スロットの二重解放を防ぐ
	FWK_ASSERT_RETURN_IF(!m_isAllocatedList[a_nodeEditorID], "未使用のNodeIDを解放しようとしており、解放処理に失敗しました。");

	m_isAllocatedList[m_nextNodeEditorID] = false;
	m_freeNodeEditorIDQueue.push(a_nodeEditorID);
}

bool FWK::NodeEditorAllocator::IsValidNodeEditorID(const TypeAlias::NodeEditorID a_nodeEditorID) const
{
	// 範囲外インデックスを指し示すならfalseを返す
	FWK_ASSERT_RETURN_VALUE_IF(a_nodeEditorID >= m_nodeEditorIDCapacity ||
							   a_nodeEditorID >= static_cast<TypeAlias::NodeEditorID>(m_isAllocatedList.size()),
							   "NodeEditorIDが無効な値であることを検知しました。",
							   false);

	return true;
}