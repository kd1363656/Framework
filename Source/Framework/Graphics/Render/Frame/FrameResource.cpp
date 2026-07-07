#include "FrameResource.h"

void FWK::Graphics::FrameResource::INIT()
{
	if (!m_directCommandAllocator)
	{
		m_directCommandAllocator = std::make_shared<DirectCommandAllocator>();
	}

	if (!m_computeCommandAllocator)
	{
		m_computeCommandAllocator = std::make_shared<ComputeCommandAllocator>();
	}
}
bool FWK::Graphics::FrameResource::Create(const Device&			    a_device, 
										  const GPUMemoryAllocator& a_gpuMemoryAllocator,
										  const Struct::ClientSize& a_clientSize, 
											    ResourceContext&    a_resourceContext)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_directCommandAllocator,				     "ダイレクトコマンドアロケータが無効です。",				     false);
	FWK_ASSERT_RETURN_VALUE_IF(!m_directCommandAllocator->Create(a_device),  "ダイレクトコマンドアロケータの作成処理に失敗しました。",   false);
	FWK_ASSERT_RETURN_VALUE_IF(!m_computeCommandAllocator,                   "コンピュートコマンドアロケータが無効です。",               false);
	FWK_ASSERT_RETURN_VALUE_IF(!m_computeCommandAllocator->Create(a_device), "コンピュートコマンドアロケータの作成処理に失敗しました。", false);

	// 定数バッファの作成
	for (const auto& l_dynamicBuffer : m_dynamicBufferUploaderList)
	{
		if (!l_dynamicBuffer) { continue; }

		FWK_ASSERT_RETURN_VALUE_IF(!l_dynamicBuffer->Create(a_device), "定数バッファ作成処理に失敗しました。", false);
	}

	FWK_ASSERT_RETURN_VALUE_IF(!m_renderGraphFrameResource.Create(a_device,
																  a_gpuMemoryAllocator,
																  a_clientSize,	
																  a_resourceContext),
																  "RenderGraphFrameResourceの作成処理に失敗しました。",
																  false);

	return true;
}
bool FWK::Graphics::FrameResource::Resize(const Device&			    a_device, 
										  const GPUMemoryAllocator& a_gpuMemoryAllocator, 
										  const Struct::ClientSize& a_clientSize,
										  const UINT64&				a_retiredFenceValue,
												ResourceContext&    a_resourceContext) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_renderGraphFrameResource.Resize(a_device,
																  a_gpuMemoryAllocator,
																  a_clientSize,
																  a_retiredFenceValue,
																  a_resourceContext),
																  "RenderGraphFrameResourceのリサイズ処理に失敗しました。",
																  false);

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
	for (const auto& l_dynamicBuffer : m_dynamicBufferUploaderList)
	{
		if (!l_dynamicBuffer) { continue; }

		l_dynamicBuffer->BeginFrame();
	}
}

nlohmann::json FWK::Graphics::FrameResource::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::FrameResource::AddDynamicBufferUploader(const std::shared_ptr<DynamicBufferUploaderBase>& a_dynamicBufferUploader)
{
	FWK_ASSERT_RETURN_IF(!a_dynamicBufferUploader, "ConstantBufferUploaderが無効のため、追加に失敗しました。");

	const auto l_staticTypeID = a_dynamicBufferUploader->GetREFRuntimeTypeINFO().k_staticTypeID;

	// すでに同じ型のUploaderが登録されている場合は再度追加しない
	if (m_dynamicBufferUploaderMap.contains(l_staticTypeID)) { return; }

	m_dynamicBufferUploaderList.emplace_back(a_dynamicBufferUploader);
	m_dynamicBufferUploaderMap.try_emplace  (l_staticTypeID, a_dynamicBufferUploader);
}

void FWK::Graphics::FrameResource::RemoveExpiredConstantBufferUploaderList()
{
	std::size_t l_index = 0ULL;

	while (l_index < m_dynamicBufferUploaderList.size())
	{
		if (m_dynamicBufferUploaderList[l_index]) 
		{
			++l_index;
			continue;
		}

		// 解放順は考慮しなくてよいのでpop_backする
		std::swap							(m_dynamicBufferUploaderList[l_index], m_dynamicBufferUploaderList.back());
		m_dynamicBufferUploaderList.pop_back();
	}
}
void FWK::Graphics::FrameResource::RemoveExpiredConstantBufferUploaderMap()
{
	auto l_itr = m_dynamicBufferUploaderMap.begin();

	while (l_itr != m_dynamicBufferUploaderMap.end())
	{
		if (!l_itr->second.expired())
		{
			++l_itr;
			continue;
		}

		l_itr = m_dynamicBufferUploaderMap.erase(l_itr);
	}
}