#include "GraphicsManager.h"

void FWK::Graphics::GraphicsManager::INIT()
{
#if defined(_DEBUG)
	FWK_ASSERT_RETURN_IF_FAILED(!EnableDebugLayer(), "デバッグレイヤーの有効化に失敗しました。");
#endif

	m_renderer.INIT();
}
void FWK::Graphics::GraphicsManager::LoadCONFIG()
{
	const auto& l_rootJson = Utility::LoadJsonFile(k_configFileIOPath);

	if (l_rootJson.is_null()) { return; }

	m_graphicsManagerJsonConverter.Deserialize(l_rootJson, *this);
}
bool FWK::Graphics::GraphicsManager::PostLoadCONFIG()
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_factory.Create(),			"ファクトリーの作成に失敗しました。",   false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_device.Create(m_factory),	"デバイスの作成処理に失敗しました。",   false);
	
	m_renderer.PostDeserialize(m_device);

	return true;
}

void FWK::Graphics::GraphicsManager::BeginFrame()
{
	m_renderer.BeginFrame();
}
void FWK::Graphics::GraphicsManager::EndFrame()
{
	m_renderer.EndFrame();
}

void FWK::Graphics::GraphicsManager::SaveCONFIG() const
{
	const auto& l_rootJson = m_graphicsManagerJsonConverter.Serialize(*this);

	Utility::SaveJsonFile(l_rootJson, k_configFileIOPath);
}

#if defined(_DEBUG)
bool FWK::Graphics::GraphicsManager::EnableDebugLayer() const
{
	TypeAlias::ComPtr<ID3D12Debug5> l_debug = nullptr;

	// デバッグ機能を有効化するためのインターフェースを取得する関数
	// D3D12GetDebugInterface(受け取りたいCOMインターフェース型のID、
	//					      作成結果のポインタを書き込むアドレス);
	auto l_hr = D3D12GetDebugInterface(IID_PPV_ARGS(l_debug.ReleaseAndGetAddressOf()));

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "デバッグレイヤーの有効化に失敗しました。", false);

	l_debug->EnableDebugLayer();

	return true;
}
#endif