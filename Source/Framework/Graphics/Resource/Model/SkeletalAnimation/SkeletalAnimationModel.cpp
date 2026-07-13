#include "SkeletalAnimationModel.h"

FWK::Graphics::SkeletalAnimationModel::SkeletalAnimationModel() : 
	m_skeletalAnimationModelRecord(),
	m_storageID                   (AssetRecordBase::k_invalidStorageID)
{

}
FWK::Graphics::SkeletalAnimationModel::SkeletalAnimationModel(const SkeletalAnimationModel& a_other) : 
	m_skeletalAnimationModelRecord(a_other.m_skeletalAnimationModelRecord),
	m_storageID                   (a_other.m_storageID)
{
	AddReferenceCount();
}
FWK::Graphics::SkeletalAnimationModel::SkeletalAnimationModel(SkeletalAnimationModel&& a_other) noexcept :
	m_skeletalAnimationModelRecord(std::move(a_other.m_skeletalAnimationModelRecord)),
	m_storageID                   (a_other.m_storageID)
{
	a_other.m_storageID = AssetRecordBase::k_invalidStorageID;

	a_other.m_skeletalAnimationModelRecord.reset();
}
FWK::Graphics::SkeletalAnimationModel::~SkeletalAnimationModel()
{
	SubtractReferenceCount();
}

FWK::Graphics::SkeletalAnimationModel& FWK::Graphics::SkeletalAnimationModel::operator=(const SkeletalAnimationModel& a_other)
{
	if (this == &a_other) { return *this; }

	// 現在参照しているSkeletalAnimationModelの参照数を減らす
	SubtractReferenceCount();

	// コピー元と同じSkeletalAnimationModelRecordを参照する
	m_storageID = a_other.m_storageID;

	m_skeletalAnimationModelRecord = a_other.m_skeletalAnimationModelRecord;

	// 新しく参照するSkeletalAnimationModelの参照数を増やす
	AddReferenceCount();

	return *this;
}
FWK::Graphics::SkeletalAnimationModel& FWK::Graphics::SkeletalAnimationModel::operator=(SkeletalAnimationModel&& a_other) noexcept
{
	if (this == &a_other) { return *this; }

	// 現在参照しているSkeletalAnimationModelの参照数を減らす
	SubtractReferenceCount();

	// Move出は参照数を増やさず、所有している参照だけを移動する
	m_storageID = a_other.m_storageID;

	m_skeletalAnimationModelRecord = std::move(a_other.m_skeletalAnimationModelRecord);

	// Move元が参照数を減らさないように無効化する
	a_other.m_storageID = AssetRecordBase::k_invalidStorageID;
	a_other.m_skeletalAnimationModelRecord.reset();

	return *this;
}

bool FWK::Graphics::SkeletalAnimationModel::Load(const std::filesystem::path& a_filePath)
{
	// 既に別のSkeletalAnimationModelを参照している場合は、
	// 新しいモデルを読み込む前に現在の参照を外す
	SubtractReferenceCount();

	      auto& l_graphicsManager              = GraphicsManager::GetInstance                               ();
	      auto& l_resourceContext              = l_graphicsManager.GetMutableREFResourceContext             ();
	      auto& l_skeletalAnimationModelSystem = l_resourceContext.GetMutableREFSkeletalAnimationModelSystem();
	const auto& l_device		               = l_graphicsManager.GetREFDevice		                        ();
	const auto& l_gpuMemoryAllocator           = l_resourceContext.GetREFGPUMemoryAllocator                 ();
		  auto& l_cbvSRVUAVDescriptorPool      = l_resourceContext.GetMutableREFCBVSRVUAVDescriptorPool     ();

	const auto& l_skeletalAnimationModelLoadResult = l_skeletalAnimationModelSystem.LoadSkeletalAnimationModelForBatchUpload(l_device,
																															 l_gpuMemoryAllocator,
																															 a_filePath,
																															 l_cbvSRVUAVDescriptorPool);
	
	FWK_ASSERT_RETURN_VALUE_IF(l_skeletalAnimationModelLoadResult.m_storageID == AssetRecordBase::k_invalidStorageID, "SkeletalAnimationModelの読み込みに失敗しました。",                                           false);
	FWK_ASSERT_RETURN_VALUE_IF(l_skeletalAnimationModelLoadResult.m_skeletalAnimationModelRecord.expired(),           "SkeletalAnimationModelRecordが無効のため、SkeletalAnimationModelの読み込みに失敗しました。", false);

	m_storageID                    = l_skeletalAnimationModelLoadResult.m_storageID;
	m_skeletalAnimationModelRecord = l_skeletalAnimationModelLoadResult.m_skeletalAnimationModelRecord;

	return true;
}

bool FWK::Graphics::SkeletalAnimationModel::IsValid() const
{
	if (m_storageID == AssetRecordBase::k_invalidStorageID ||
		m_skeletalAnimationModelRecord.expired())
	{
		return false;
	}

	return true;
}

void FWK::Graphics::SkeletalAnimationModel::AddReferenceCount() const
{
	if (m_storageID == AssetRecordBase::k_invalidStorageID) { return; }

	auto& l_graphicsManager              = GraphicsManager::GetInstance                               ();
	auto& l_resourceContext              = l_graphicsManager.GetMutableREFResourceContext             ();
	auto& l_skeletalAnimationModelSystem = l_resourceContext.GetMutableREFSkeletalAnimationModelSystem();

	FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelSystem.AddSkeletalAnimationModelReferenceCount(m_skeletalAnimationModelRecord), "SkeletalAnimationModelの参照数加算に失敗しました。");
}

void FWK::Graphics::SkeletalAnimationModel::SubtractReferenceCount()
{
	if (m_storageID == AssetRecordBase::k_invalidStorageID) { return; }

	      auto& l_graphicsManager              = GraphicsManager::GetInstance                               ();
	      auto& l_resourceContext              = l_graphicsManager.GetMutableREFResourceContext             ();
	const auto& l_renderer                     = l_graphicsManager.GetREFRenderer                           ();
	const auto& l_directCommandQueue           = l_renderer.GetREFDirectCommandQueue                        ();
	      auto& l_skeletalAnimationModelSystem = l_resourceContext.GetMutableREFSkeletalAnimationModelSystem();
		  auto& l_resourceReleaseContext       = l_resourceContext.GetMutableREFResourceReleaseContext      ();

	FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelSystem.SubtractSkeletalAnimationModelReferenceCount(m_skeletalAnimationModelRecord, l_directCommandQueue, l_resourceReleaseContext), "SkeletalAnimationModelの参照数減算に失敗しました。");

	m_storageID = AssetRecordBase::k_invalidStorageID;

	m_skeletalAnimationModelRecord.reset();
}