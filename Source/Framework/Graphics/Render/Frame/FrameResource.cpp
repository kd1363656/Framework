#include "FrameResource.h"

void FWK::Graphics::FrameResource::INIT()
{
	if (!m_directCommandAllocator)
	{
		m_directCommandAllocator = std::make_shared<DirectCommandAllocator>();
	}
}
bool FWK::Graphics::FrameResource::Create(const Device& a_device)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_directCommandAllocator,				       "ダイレクトコマンドアロケータが無効です。",				 false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_directCommandAllocator->Create(a_device), "ダイレクトコマンドアロケータの作成処理に失敗しました。", false);

	// 定数バッファの作成
	for (const auto& l_constantBuffer : m_constantBufferUploaderList)
	{
		if (!l_constantBuffer) { continue; }

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_constantBuffer->Create(a_device), "定数バッファ作成処理に失敗しました。", false);
	}

	return true;
}
void FWK::Graphics::FrameResource::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Graphics::FrameResource::BeginFrame()
{
	RemoveExpiredConstantBufferUploaderList();
	RemoveExpiredConstantBufferUploaderMap ();

	// 定数バッファのインデックスのリセット処理などを行う
	for (const auto& l_constantBuffer : m_constantBufferUploaderList)
	{
		if (!l_constantBuffer) { continue; }

		l_constantBuffer->BeginFrame();
	}
}

nlohmann::json FWK::Graphics::FrameResource::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::FrameResource::AddConstantBufferUploader(const std::shared_ptr<ConstantBufferUploaderBase>& a_constantBufferUploader)
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_constantBufferUploader, "ConstantBufferUploaderが無効のため、追加に失敗しました。");

	const auto l_staticTypeID = a_constantBufferUploader->GetREFRuntimeTypeINFO().k_staticTypeID;

	// すでに同じ型のUploaderが登録されている場合は再度追加しない
	if (m_constantBufferUploaderMap.contains(l_staticTypeID)) { return; }

	m_constantBufferUploaderList.emplace_back(a_constantBufferUploader);
	m_constantBufferUploaderMap.try_emplace  (l_staticTypeID, a_constantBufferUploader);
}

void FWK::Graphics::FrameResource::RemoveExpiredConstantBufferUploaderList()
{
	std::size_t l_index = 0ULL;

	while (l_index < m_constantBufferUploaderList.size())
	{
		if (m_constantBufferUploaderList[l_index]) 
		{
			++l_index;
			continue;
		}

		// 解放順は考慮しなくてよいのでpop_backする
		std::swap							 (m_constantBufferUploaderList[l_index], m_constantBufferUploaderList.back());
		m_constantBufferUploaderList.pop_back();
	}
}
void FWK::Graphics::FrameResource::RemoveExpiredConstantBufferUploaderMap()
{
	auto l_itr = m_constantBufferUploaderMap.begin();

	while (l_itr != m_constantBufferUploaderMap.end())
	{
		if (!l_itr->second.expired())
		{
			++l_itr;
			continue;
		}

		l_itr = m_constantBufferUploaderMap.erase(l_itr);
	}
}