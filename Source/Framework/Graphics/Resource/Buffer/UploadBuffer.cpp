#include "UploadBuffer.h"

FWK::Graphics::UploadBuffer::UploadBuffer() : 
	m_uploadBuffer(nullptr),

	m_mappedData(nullptr)
{}
FWK::Graphics::UploadBuffer::~UploadBuffer()
{
	// MapしたデータをUnMap
	Release();
}

bool FWK::Graphics::UploadBuffer::Create(const Device& a_device, const UINT64& a_bufferSize)
{
	// 二重Mapを防ぐためリリース
	Release();

	const auto& l_device = a_device.GetREFDevice();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_device, "デバイスが作成されておらず、UploadBufferの作成に失敗しました。", false);

	// サイズ0のバッファは作成する意味がないため失敗扱いにする
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_bufferSize == Constant::k_invalidBufferSize, "UploadBufferの作成サイズが0のため、UploadBufferの作成に失敗しました。", false);

	constexpr auto l_nodeMask = Constant::k_defaultGPUNodeMask;

	// D3D12_HEAP_PROPERTIESについての説明
	// Type                 : どの種類のヒープにリソースを置くか
	//		                  D3D12_HEAP_TYPE_UPLOADなのでCPUからMapして書き込むことを想定したアップロード用ヒープ
	// CPUPageProperty		: CPUページ属性(既定値を使うため内部的にUNKNOWN相当)
	// MemoryPoolPreference : メモリプール指定(既定値を使うため内部的にUNKNOWN相当)
	// CreationNodeMask		: 作製対象GPUノード
	// VisibleNodeMask	    : 可視対象GPUノード
	// UploadBufferはCPUでデータを書き込み、
	// その後CopyBufferRegionやCopyTextureRegionで本番リソースへ転送するため、
	// D3D12_HEAP_TYPE_UPLOADで作成する
	auto l_heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD, l_nodeMask, l_nodeMask);

	// D3D12_RESOURCE_DESCについての説明
	// Buffer(作成するバッファサイズ)
	// 今回はテクスチャではなく単純な線形メモリとして扱うアップロード用バッファを作成する。
	auto l_resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(a_bufferSize);

	// CreateCommittedResource(ヒープ設定、
	//						   ヒープ追加フラグ、
	//						   リソース設定、
	//						   初期リソース状態、
	//						   ClearValue(nullptrなので未使用)、
	//						   受け取りたいインターフェース型、
	//						   作成結果のポインタ書き込み先);
	// CommittedResourceはヒープとリソースをまとめて一つで作成する方式
	// UploadBufferは転送用の一時バッファであり、管理を複雑にしなくてよいためこの方式で十分
	// 初期状態にD3D12_RESOURCE_STATE_GENERIC_READを指定している理由
	// UploadHeap上のリソースはCPUから書き込み、GPUから読み取ってコピー元として使う想定のため
	const auto l_hr = l_device->CreateCommittedResource(&l_heapProperties,
														D3D12_HEAP_FLAG_NONE,
													    &l_resourceDesc,
														D3D12_RESOURCE_STATE_GENERIC_READ,
														nullptr,
														IID_PPV_ARGS(m_uploadBuffer.ReleaseAndGetAddressOf()));

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "UploadBufferの作成に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Map(),       "UploadBufferのMapに失敗しました。",  false);

	return true;
}

void FWK::Graphics::UploadBuffer::Release()
{
	// Map済みなら先にUnMapする
	// UnMap後はm_mappedDataをnullptrに戻しご使用を防ぐ
	UnMap();
}

std::uint8_t* FWK::Graphics::UploadBuffer::FetchPTRMappedData() const
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_mappedData, "UploadBufferがMapされておらず、書き込み先ポインタの取得に失敗しました。", nullptr);

	return m_mappedData;
}

bool FWK::Graphics::UploadBuffer::Map()
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_uploadBuffer, "UploadBufferが作成されておらず、Mapに失敗しました。", false);

	// 既にMap済みなら成功扱いにする
	// UploadBufferは永続Mapする方針なので、二重マップはしない。
	if (m_mappedData) { return true; }

	// Map成功時にCPUから書き込める先頭アドレスを受け取る
	void* l_mappedData = nullptr;
	
	// Map(対象サブリソース番号、
	//	   CPUが読む範囲情報(nullptrなので範囲指定なし)、
	//	   マップした先頭アドレスの受取先);
	// Bufferリソースなのでサブリソースは0固定で扱う
	// UploadBufferはCPUから書き込みたい用途なのでMapして生ポインタを取得する
	const auto l_hr = m_uploadBuffer->Map(Constant::k_firstSubresourceIndex, nullptr, &l_mappedData);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "UploadBufferのMapに失敗しました。", false);

	m_mappedData = static_cast<std::uint8_t*>(l_mappedData);

	return true;
}

void FWK::Graphics::UploadBuffer::UnMap()
{
	if (!m_uploadBuffer || !m_mappedData) { return; }

	m_uploadBuffer->Unmap(Constant::k_firstSubresourceIndex, nullptr);

	m_mappedData = nullptr;
}