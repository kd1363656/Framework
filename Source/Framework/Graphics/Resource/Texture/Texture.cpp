#include "Texture.h"

FWK::Graphics::Texture::Texture() : 
	m_textureRecord({}),
	m_storageID    (Constant::k_invalidStorageID)
{}
FWK::Graphics::Texture::Texture(const Texture & a_other) : 
	m_textureRecord(a_other.m_textureRecord),
	m_storageID    (a_other.m_storageID)
{
	AddReferenceCount();
}
FWK::Graphics::Texture::Texture(Texture&& a_other) noexcept : 
	m_textureRecord(std::move(a_other.m_textureRecord)),
	m_storageID    (a_other.m_storageID)
{
	a_other.m_storageID = Constant::k_invalidStorageID;
	a_other.m_textureRecord.reset();
}
FWK::Graphics::Texture::~Texture()
{
	SubtractReferenceCount();;
}

FWK::Graphics::Texture& FWK::Graphics::Texture::operator=(const Texture& a_other)
{
	if (this == &a_other) { return *this; }

	// 所持しているテクスチャをの参照数を減算
	SubtractReferenceCount();;

	// コピー元と同じTextureRecordを参照する
	m_storageID     = a_other.m_storageID;
	m_textureRecord = a_other.m_textureRecord;

	// 参照数の加算
	AddReferenceCount();

	return *this;
}
FWK::Graphics::Texture& FWK::Graphics::Texture::operator=(Texture&& a_other) noexcept
{
	if (this == &a_other) { return *this; }

	// 所持しているテクスチャを破棄
	SubtractReferenceCount();

	// ムーブでは参照数を増やさず、参照先だけ移す
	m_storageID     = a_other.m_storageID;
	m_textureRecord = std::move(a_other.m_textureRecord);

	// 参照元のストレージIDを無効化
	a_other.m_storageID = Constant::k_invalidStorageID;
	a_other.m_textureRecord.reset();

	return *this;
}

bool FWK::Graphics::Texture::Load(const std::filesystem::path& a_filePath, const Enum::TextureLoadColorSpace a_textureLoadColorSpace, const Enum::DefaultTextureType a_defaultTextureType)
{
	// 既に別のStorageIDを持っている場合は先に参照を外す
	SubtractReferenceCount();

	auto& l_graphicsManager = FWK::Graphics::GraphicsManager::GetInstance();

	const auto& l_device = l_graphicsManager.GetREFDevice();

		  auto& l_resourceContext    = l_graphicsManager.GetMutableREFResourceContext  ();
	const auto& l_gpuMemoryAllocator = l_resourceContext.GetREFGPUMemoryAllocator      ();
	      auto& l_textureSystem      = l_resourceContext.GetMutableREFTextureSystem    ();
	      auto& l_srvDescriptorPool  = l_resourceContext.GetMutableREFSRVDescriptorPool();
	
	// ロードタイプに応じたテクスチャの読み込みを行い
	// テクスチャのGPUリソース作成の一括登録申請用の処理を行う
	const auto& l_textureLoadResult = l_textureSystem.LoadTextureForBatchUpload(l_device,
																				l_gpuMemoryAllocator,
																				a_filePath,
																				a_textureLoadColorSpace,
																				a_defaultTextureType,
																				l_srvDescriptorPool);

	// デフォルトテクスチャをセットしてreturn
	if (l_textureLoadResult.m_storageID == Constant::k_invalidStorageID ||
		l_textureLoadResult.m_textureRecord.expired())
	{
		return false; 
	}
	
	m_storageID     = l_textureLoadResult.m_storageID;
	m_textureRecord = l_textureLoadResult.m_textureRecord;

	return l_textureLoadResult.m_isLoadSuccess;
}

void FWK::Graphics::Texture::AddReferenceCount() const
{
	if (m_storageID == Constant::k_invalidStorageID) { return; }

	auto& l_graphicsManager = FWK::Graphics::GraphicsManager::GetInstance();

	auto& l_resourceContext = l_graphicsManager.GetMutableREFResourceContext();
	auto& l_textureSystem   = l_resourceContext.GetMutableREFTextureSystem  ();

	FWK_ASSERT_RETURN_IF(!l_textureSystem.AddTextureReferenceCount(m_textureRecord), "テクスチャ参照数加算に失敗しました。");
}
void FWK::Graphics::Texture::SubtractReferenceCount()
{
	if (m_storageID == Constant::k_invalidStorageID) 
	{
		m_textureRecord.reset();

		return; 
	}

	auto& l_graphicsManager = FWK::Graphics::GraphicsManager::GetInstance();

	const auto& l_renderer		     = l_graphicsManager.GetREFRenderer   ();
	const auto& l_directCommandQueue = l_renderer.GetREFDirectCommandQueue();

	auto& l_resourceContext        = l_graphicsManager.GetMutableREFResourceContext       ();
	auto& l_textureSystem          = l_resourceContext.GetMutableREFTextureSystem         ();
	auto& l_resourceReleaseContext = l_resourceContext.GetMutableREFResourceReleaseContext();
	
	// 参照カウントを減らす
	FWK_ASSERT_RETURN_IF(!l_textureSystem.SubtractTextureReferenceCount(m_textureRecord, l_directCommandQueue, l_resourceReleaseContext), "テクスチャ参照数解放に失敗しました。");

	m_storageID = Constant::k_invalidStorageID;
	m_textureRecord.reset();
}