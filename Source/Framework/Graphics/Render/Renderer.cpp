#include "Renderer.h"

void FWK::Graphics::Renderer::INIT()
{
	for (std::size_t l_i = 0ULL; l_i < 3; ++l_i)
	{
		const auto& l_frameResource = std::make_shared<FrameResource>();

		l_frameResource->INIT();

		m_frameResourceList.emplace_back(l_frameResource);
	}

	for (const auto& l_frameResource : m_frameResourceList)
	{
		if (!l_frameResource) { continue; }

		l_frameResource->INIT();
	}
}
void FWK::Graphics::Renderer::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_rendererJsonConverter.Deserialize(a_rootJson, *this);
}
void FWK::Graphics::Renderer::PostDeserialize(const Device& a_device)
{
	// フレームリソースがないとコマンドアロケーターを使えないため"return"
	FWK_ASSERT_RETURN_IF_FAILED(m_frameResourceList.empty(), "フレームリソースリストがからになっており、フレームリソース作成処理に失敗しました。");

	for (const auto& l_frameResource : m_frameResourceList)
	{
		if (!l_frameResource) { continue; }

		FWK_ASSERT_RETURN_IF_FAILED(!l_frameResource->Create(a_device), "フレームリソースの作成処理に失敗しました。");
	}

	FWK_ASSERT_RETURN_IF_FAILED(!m_directCommandQueue.Create(a_device), "ダイレクトコマンドキューの作成処理に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!m_directCommandList.Create(a_device),  "ダイレクトコマンドリストの作成処理に失敗しました。");

	// 最初に使うフレームリソースをキャッシュ
	m_currentFrameResource = m_frameResourceList[k_initialFrameResourceIndex];
}

void FWK::Graphics::Renderer::BeginFrame()
{
	// 現在のフレームリソースの定数バッファのインデックスの初期化
	const auto& l_currentFrameResource = m_currentFrameResource.lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_currentFrameResource, "フレームリソースの取得に失敗しており、描画開始処理に失敗しました。");

	const auto& l_commandAllocator = l_currentFrameResource->GetREFDirectCommandAllocator();

	FWK_ASSERT_RETURN_IF_FAILED(!l_commandAllocator, "ダイレクトコマンドアロケータが無効になっており、描画開始処理に失敗しました。");
	
	// コマンドアロケータからGPU処理が終わっているかどうかを確かめGPUの処理が終わっていればWait
	m_directCommandQueue.EnsureAllocatorAvailable(*l_commandAllocator);

	// GPU同期処理が終わってからコマンドリスト、アロケータをリセット
	l_commandAllocator->Reset();
	m_directCommandList.Reset(*l_commandAllocator);
}
void FWK::Graphics::Renderer::EndFrame()
{
	const auto& l_currentFrameResource = m_currentFrameResource.lock();

	FWK_ASSERT_RETURN_IF_FAILED(!l_currentFrameResource, "フレームリソースの取得に失敗しており、描画開始処理に失敗しました。");

	const auto& l_commandAllocator = l_currentFrameResource->GetREFDirectCommandAllocator();

	FWK_ASSERT_RETURN_IF_FAILED(!l_commandAllocator, "ダイレクトコマンドアロケータが無効になっており、描画開始処理に失敗しました。");

	// コマンドリストへの命令記録を終了
	m_directCommandList.Close();

	// 描画命令を実行
	m_directCommandQueue.ExecuteCommandLists(m_directCommandList);

	// フェンス値を更新
	m_directCommandQueue.SignalAndTrackAllocator(*l_commandAllocator);
}

nlohmann::json FWK::Graphics::Renderer::Serialize() const
{
	return m_rendererJsonConverter.Serialize(*this);
}

void FWK::Graphics::Renderer::AddFrameResource(const std::shared_ptr<FrameResource>& a_frameResource)
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_frameResource, "FrameResourceが無効のため、FrameResourceListへの登録に失敗しました。");

	m_frameResourceList.emplace_back(a_frameResource);
}