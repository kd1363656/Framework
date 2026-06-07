#include "AssetRecordBase.h"

void FWK::Graphics::AssetRecordBase::AddReferenceCount()
{
	++m_referenceCount;
}

bool FWK::Graphics::AssetRecordBase::SubtractReferenceCount()
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_referenceCount == Constant::k_emptyAssetReferenceCount, "参照数が0のAssetRecordに対して参照数の減算が行われました。", false);

	--m_referenceCount;

	return true;
}

bool FWK::Graphics::AssetRecordBase::IsUnused() const
{
	return m_referenceCount == Constant::k_emptyAssetReferenceCount;
}

void FWK::Graphics::AssetRecordBase::InvalidateStorageID()
{
	m_storageID = Constant::k_invalidStorageID;
}