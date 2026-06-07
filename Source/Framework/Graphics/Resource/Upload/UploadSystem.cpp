#include "UploadSystem.h"

void FWK::Graphics::UploadSystem::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
bool FWK::Graphics::UploadSystem::Create(const Device& a_device)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_copyCommandQueue.Create(a_device), "コピーコマンドキュー作成処理に失敗しました。", false);

	// Deserializeでコピーコマンドアロケータの数を設定している、もしコマンドアロケータ数が0なら
	// リストが空の場合は、UploadSystemJsonConverterのDeserialize処理を確認すること
	for (const auto& l_copyCommandAllocator : m_copyCommandAllocatorList)
	{
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_copyCommandAllocator,				     "コマンドアロケーターが無効のため、作成処理に失敗しました。", false);
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_copyCommandAllocator->Create(a_device), "コピーコマンドアロケータ作成処理に失敗しました。",		       false);
	}

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_copyCommandList.Create(a_device), "コピーコマンドリスト作成処理に失敗しました。", false);

	return true;
}

nlohmann::json FWK::Graphics::UploadSystem::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::UploadSystem::AddCommandAllocator(const std::shared_ptr<CopyCommandAllocator>& a_copyCommandAllocator)
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_copyCommandAllocator, "無効なコピーコマンドアロケーターです、コピーコマンドアロケーター追加処理に失敗しました。");

	m_copyCommandAllocatorList.emplace_back(a_copyCommandAllocator);
}

std::weak_ptr<FWK::Graphics::CopyCommandAllocator> FWK::Graphics::UploadSystem::FetchMutablePTRCopyCommandAllocator()
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_copyCommandAllocatorList.empty(),									"コピーコマンドアロケータリストが空のため、コピーコマンドアロケータ取得処理に失敗しました。",					       {});
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_currentCopyCommandAllocatorIndex >= m_copyCommandAllocatorList.size(), "コピーコマンドアロケータリストの容量を超えたインデックスのため、コピーコマンドアロケータ取得処理に失敗しました。", {});

	const auto& l_copyCommandAllocator = m_copyCommandAllocatorList[m_currentCopyCommandAllocatorIndex];

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_copyCommandAllocator, "コピーコマンドアロケータが無効のため、コピーコマンドアロケータ取得処理に失敗しました。", {});

	// もしWaitが必要なコマンドアロケータならWaitする
	m_copyCommandQueue.EnsureAllocatorAvailable(*l_copyCommandAllocator);

	// 次のコピーコマンドアロケータを使用するようにインデックスを更新
	m_currentCopyCommandAllocatorIndex = (m_currentCopyCommandAllocatorIndex + k_copyCommandAllocatorIndexIncrement) % m_copyCommandAllocatorList.size();

	return l_copyCommandAllocator;
}