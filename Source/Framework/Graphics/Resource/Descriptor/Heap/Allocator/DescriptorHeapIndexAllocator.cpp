#include "DescriptorHeapIndexAllocator.h"

void FWK::Graphics::DescriptorHeapIndexAllocator::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::DescriptorHeapIndexAllocator::Create()
{
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_indexCapacity == Constant::k_invalidDescriptorIndex, "無効なIndexを割り当てようとしており作成処理に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_indexCapacity == Constant::k_emptyDescriptorNUM,     "無効なIndexを割り当てようとしており作成処理に失敗しました。", false);
    
    m_nextAllocateIndex = k_initialNextAllocateIndex;

    // 全スロットを未使用状態で初期化する
    m_isAllocatedList.assign(m_indexCapacity, false);

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
    // 範囲外Indexの解放は不正
    FWK_ASSERT_RETURN_IF_FAILED(!IsValidIndex(a_index), "解放しようとしたIndexが確保範囲外となっており、解放処理に失敗しました。。");

	// 未使用スロットの二重解放を防ぐ
	FWK_ASSERT_RETURN_IF_FAILED(!m_isAllocatedList[a_index], "未使用のIndexを解放しようとしており、解放処理に失敗しました。。");

    m_isAllocatedList[a_index] = false;
    m_reusableIndexQueue.push(a_index);
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::DescriptorHeapIndexAllocator::Allocate()
{
    // 解放済みスロットがあればそれを優先再利用する
    if (!m_reusableIndexQueue.empty())
    {
        const TypeAlias::DescriptorIndex l_reusableIndex = m_reusableIndexQueue.front();

        m_reusableIndexQueue.pop();

        // 有効なインデックスかどうかを確認
        FWK_ASSERT_RETURN_VALUE_IF_FAILED(!IsValidIndex(l_reusableIndex), "再利用しようとしたIndexが確保範囲外で、Indexの確保に失敗しました。", Constant::k_invalidDescriptorIndex);

        m_isAllocatedList[l_reusableIndex] = true;

        return l_reusableIndex;
    }

    // 未使用領域が残っているなら新規払い出しする
    if (IsValidIndex(m_nextAllocateIndex))
    {
        const auto l_allocateIndex = m_nextAllocateIndex;

        ++m_nextAllocateIndex;

        // 新規払い出しするインデックス番号は割り当て済みとして扱う
        m_isAllocatedList[l_allocateIndex] = true;

        return l_allocateIndex;
    }

    FWK_ASSERT_RETURN_VALUE("Indexの空きがなくなり、割り当てに失敗しました。", Constant::k_invalidDescriptorIndex);
}

bool FWK::Graphics::DescriptorHeapIndexAllocator::IsValidIndex(const TypeAlias::DescriptorIndex a_index) const
{
    // 範囲外インデックスを指し示すならfalseを返す
    if (a_index >= m_indexCapacity ||
        a_index >= static_cast<TypeAlias::DescriptorIndex>(m_isAllocatedList.size())) 
    {
        return false; 
    }

    return true;
}