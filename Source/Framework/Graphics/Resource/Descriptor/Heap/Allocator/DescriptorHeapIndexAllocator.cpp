#include "DescriptorHeapIndexAllocator.h"

void FWK::Graphics::DescriptorHeapIndexAllocator::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::DescriptorHeapIndexAllocator::Create()
{
    FWK_ASSERT_RETURN_VALUE_IF(m_capacity == DescriptorHeap::k_invalidDescriptorIndex, "無効なIndexを割り当てようとしており作成処理に失敗しました。",        false);
    FWK_ASSERT_RETURN_VALUE_IF(m_capacity == DescriptorHeap::k_invalidDescriptorNUM,   "ディスクリプタの作成数が無効となっており、作成処理に失敗しました。", false);
    
    m_nextIndex = k_initialNextIndex;

    // 全スロットを未使用状態で初期化する
    m_isAllocatedIndexList.assign(m_capacity, false);

    // キューも何も保持していない状態にする
    m_reusableIndexQueue = {};

	return true;
}

nlohmann::json FWK::Graphics::DescriptorHeapIndexAllocator::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::DescriptorHeapIndexAllocator::Release(const TypeAlias::DescriptorIndex a_index)
{
    // 無効なIndexの解放は不正とみなす
    FWK_ASSERT_RETURN_IF(IsInValidIndex(a_index), "解放しようとしたIndexが確保範囲外となっており、解放処理に失敗しました。。");

    // アロケートリストの容量を超えていたらreturn
    if (a_index > m_isAllocatedIndexList.size()) { return; }
    
	// 未使用スロットの二重解放を防ぐ
    FWK_ASSERT_RETURN_IF(!m_isAllocatedIndexList[a_index], "未使用のIndexを解放しようとしており、解放処理に失敗しました。。");
    
    m_isAllocatedIndexList[a_index] = false;

    m_reusableIndexQueue.push(a_index);
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::DescriptorHeapIndexAllocator::Allocate()
{
    FWK_ASSERT_RETURN_VALUE_IF(m_isAllocatedIndexList.empty(), "m_isAllocatedListの容量を超えており、DescriptorIndexのAllocateに失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);

    // 解放済みスロットがあればそれを優先再利用する
    if (!m_reusableIndexQueue.empty())
    {
        const TypeAlias::DescriptorIndex l_reusableIndex = m_reusableIndexQueue.front();

        m_reusableIndexQueue.pop();

        // 有効なインデックスかどうかを確認
        FWK_ASSERT_RETURN_VALUE_IF(IsInValidIndex(l_reusableIndex), "再利用しようとしたIndexが確保範囲外で、Indexの確保に失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);

        m_isAllocatedIndexList[l_reusableIndex] = true;

        return l_reusableIndex;
    }

    // 無効なインデックスならassert
    FWK_ASSERT_RETURN_VALUE_IF(IsInValidIndex(m_nextIndex), "Indexの空きがなくなり、割り当てに失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);

    // 未使用領域が残っているなら新規払い出しする
    const auto l_allocatedIndex = m_nextIndex;

    ++m_nextIndex;

    // 新規払い出しするインデックス番号は割り当て済みとして扱う
    m_isAllocatedIndexList[l_allocatedIndex] = true;

    return l_allocatedIndex;
}

bool FWK::Graphics::DescriptorHeapIndexAllocator::IsInValidIndex(const TypeAlias::DescriptorIndex a_index) const
{
    // 範囲外インデックスを指し示すならfalseを返す
    if (a_index >= m_capacity ||
        a_index >= static_cast<TypeAlias::DescriptorIndex>(m_isAllocatedIndexList.size())) 
    {
        return true; 
    }

    return false;
}