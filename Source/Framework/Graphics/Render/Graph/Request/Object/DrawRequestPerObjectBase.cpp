#include "DrawRequestPerObjectBase.h"

FWK::TypeAlias::DescriptorIndex FWK::Graphics::DrawRequestPerObjectBase::FetchTextureSRVDescriptorIndex(const std::weak_ptr<TextureRecord>& a_textureRecord) const
{
	const auto& l_textureRecord = a_textureRecord.lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_textureRecord, "TextureRecordが無効なため、TextureSRVDescriptorIndexの取得に失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);

	return l_textureRecord->GetVALSRVDescriptorIndex();
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::DrawRequestPerObjectBase::FetchTextureSRVDescriptorIndex(const std::shared_ptr<Texture>& a_texture) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_texture, "Textureが無効なため、TextureSRVDescriptorIndexの取得に失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);

	return FetchTextureSRVDescriptorIndex(a_texture->GetREFTextureRecord());
}