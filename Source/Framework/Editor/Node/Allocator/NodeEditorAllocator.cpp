#include "NodeEditorAllocator.h"

void FWK::NodeEditorAllocator::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);

	// IsAllocatedListをDeserializeした後にQueueもそのDeserialize結果を受けて再構築
	RebuildFreeNodeEditorIDQueue();
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
		const int l_reuseNodeEditorID = m_freeNodeEditorIDQueue.front();

		m_freeNodeEditorIDQueue.pop();

		// 有効なインデックスかどうかを確認
		FWK_ASSERT_RETURN_VALUE_IF(l_reuseNodeEditorID >= static_cast<TypeAlias::NodeEditorID>(m_isAllocatedList.size()), "再利用しようとしたReuseNodeEditorIDが有効範囲外となっている。", Constant::k_invalidNodeEditorID);
		FWK_ASSERT_RETURN_VALUE_IF(m_isAllocatedList[l_reuseNodeEditorID],                                                "使用中のReuseNodeEditorIDを再利用しようとしています。",         Constant::k_invalidNodeEditorID);

		m_isAllocatedList[l_reuseNodeEditorID] = true;

		return l_reuseNodeEditorID;
	}

	const auto l_newNodeEditorID = static_cast<TypeAlias::NodeEditorID>(m_isAllocatedList.size());

	//　新しい番号尾使用ちゅう状態として追加する
	m_isAllocatedList.emplace_back(true);

	return l_newNodeEditorID;
}

void FWK::NodeEditorAllocator::Release(const TypeAlias::NodeEditorID a_nodeEditorID)
{
	FWK_ASSERT_RETURN_IF(a_nodeEditorID >= static_cast<TypeAlias::NodeEditorID>(m_isAllocatedList.size()), "解放しようとしたNodeEditorIDが有効範囲外となっています。");
	FWK_ASSERT_RETURN_IF(!m_isAllocatedList[a_nodeEditorID],                                               "未使用のNodeEditorIDを二重に解放しようとしています。");

	// 削除されたPrefabインスタンスの名前を未使用状態へ戻す
	m_isAllocatedList[a_nodeEditorID] = false;

	// 次回のAllocate()で再利用できるようにする
	m_freeNodeEditorIDQueue.push(a_nodeEditorID);
}

void FWK::NodeEditorAllocator::RebuildFreeNodeEditorIDQueue()
{
	std::queue<TypeAlias::NodeEditorID> l_emptyQueue = {};

	m_freeNodeEditorIDQueue.swap(l_emptyQueue);

	for (std::size_t l_index = 0ULL; l_index < m_isAllocatedList.size(); ++l_index)
	{
		if (m_isAllocatedList[l_index]) { continue; }

		m_freeNodeEditorIDQueue.push(static_cast<TypeAlias::NodeEditorID>(l_index));
	}
}