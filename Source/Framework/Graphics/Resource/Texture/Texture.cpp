#include "Texture.h"

FWK::Graphics::Texture::Texture() : 
	m_textureRecord({}),
	m_storageID    (Constant::k_invalidStorageID)
{}
FWK::Graphics::Texture::Texture(const Texture & a_other) : 
	m_textureRecord(a_other.m_textureRecord),
	m_storageID    (a_other.m_storageID)
{
	AddTextureReferenceCount();
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
	SubtractTextureReferenceCount();;
}

FWK::Graphics::Texture& FWK::Graphics::Texture::operator=(const Texture& a_other)
{
	if (this == &a_other) { return *this; }

	// 所持しているテクスチャを破棄
	SubtractTextureReferenceCount();;

	// コピー元と同じTextureRecordを参照する
	m_storageID     = a_other.m_storageID;
	m_textureRecord = a_other.m_textureRecord;

	// 参照数の加算
	AddTextureReferenceCount();

	return *this;
}
FWK::Graphics::Texture& FWK::Graphics::Texture::operator=(Texture&& a_other) noexcept
{
	if (this == &a_other) { return *this; }

	// 所持しているテクスチャを破棄
	SubtractTextureReferenceCount();

	// ムーブでは参照数を増やさず、参照先だけ移す
	m_storageID     = a_other.m_storageID;
	m_textureRecord = std::move(a_other.m_textureRecord);

	// 参照元のストレージIDを無効化
	a_other.m_storageID = Constant::k_invalidStorageID;
	a_other.m_textureRecord.reset();

	return *this;
}

bool FWK::Graphics::Texture::Load(const std::filesystem::path& a_filePath, Enum::TextureLoadType a_loadType, const Enum::DefaultTextureType a_defaultTextureType)
{
	// 既に別のStorageIDを持っている場合は先に参照を外す
	SubtractTextureReferenceCount();

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
																				l_srvDescriptorPool,
																				a_loadType);

	// テクスチャの登録がデフォルトテクスチャをセットしてreturn
	if (l_textureLoadResult.m_storageID == Constant::k_invalidStorageID ||
		l_textureLoadResult.m_textureRecord.expired())
	{
		SetupDefauldtTexture(a_defaultTextureType);
		return false; 
	}
	
	m_storageID     = l_textureLoadResult.m_storageID;
	m_textureRecord = l_textureLoadResult.m_textureRecord;

	return true;
}

void FWK::Graphics::Texture::SetupDefauldtTexture(const Enum::DefaultTextureType a_defaultTextureType)
{
	const auto& l_graphicsManager = FWK::Graphics::GraphicsManager::GetInstance();
	const auto& l_resourceContext = l_graphicsManager.GetREFResourceContext    ();
	const auto& l_textureSystem   = l_resourceContext.GetREFTextureSystem      ();
	
	const auto& l_defaultTextureRecord = l_textureSystem.FetchVALDefaultTextureRecord(a_defaultTextureType);
	const auto& l_teextureRecord       = l_defaultTextureRecord.lock				 ();
	
	FWK_ASSERT_RETURN_IF_FAILED(!l_teextureRecord, "DefaultTextureRecordが無効のため、デフォルトテクスチャ設定に失敗しました。");

	// デフォルトテクスチャはAssetStorage管理ではないため、StorageIDは無効値のままにする
	m_storageID     = l_teextureRecord->GetVALStorageID();
	m_textureRecord = l_defaultTextureRecord;
}

void FWK::Graphics::Texture::AddTextureReferenceCount() const
{
	if (m_storageID == Constant::k_invalidStorageID) { return; }

	auto& l_graphicsManager = FWK::Graphics::GraphicsManager::GetInstance();

	auto& l_resourceContext = l_graphicsManager.GetMutableREFResourceContext();
	auto& l_textureSystem   = l_resourceContext.GetMutableREFTextureSystem  ();

	FWK_ASSERT_RETURN_IF_FAILED(!l_textureSystem.AddTextureReferenceCount(m_textureRecord), "テクスチャ参照数加算に失敗しました。");
}
void FWK::Graphics::Texture::SubtractTextureReferenceCount()
{
	if (m_storageID == Constant::k_invalidStorageID) 
	{
		m_textureRecord.reset();
		return; 
	}

	auto& l_graphicsManager = FWK::Graphics::GraphicsManager::GetInstance();

	const auto& l_renderer		     = l_graphicsManager.GetREFRenderer   ();
	const auto& l_directCommandQueue = l_renderer.GetREFDirectCommandQueue();

	auto& l_resourceContext = l_graphicsManager.GetMutableREFResourceContext();
	auto& l_textureSystem   = l_resourceContext.GetMutableREFTextureSystem  ();
	
	// 参照カウントを減らす
	auto& l_resourceReleaseContext = l_resourceContext.GetMutableREFResourceReleaseContext();

	FWK_ASSERT_RETURN_IF_FAILED(!l_textureSystem.SubtractTextureReferenceCount(m_textureRecord, l_directCommandQueue, l_resourceReleaseContext), "テクスチャ参照数解放に失敗しました。");

	m_storageID = Constant::k_invalidStorageID;
	m_textureRecord.reset();
}