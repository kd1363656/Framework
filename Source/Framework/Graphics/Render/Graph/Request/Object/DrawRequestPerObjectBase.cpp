#include "DrawRequestPerObjectBase.h"

void FWK::Graphics::DrawRequestPerObjectBase::SetupSRVDescriptorHeap(const DirectCommandList& a_directCommandList, const TypeAlias::SRVDescriptorPool& a_srvDescriptorPool) const
{
	// SRVディスクリプタヒープをセット
	a_directCommandList.SetupDescriptorHeap(a_srvDescriptorPool);
}

std::weak_ptr<FWK::Graphics::RootSignature> FWK::Graphics::DrawRequestPerObjectBase::SetupRenderPipeline(const Renderer& a_renderer, const Enum::PipelineStateType a_pipelineStateType) const
{
	const auto& l_pipelineStateWeak = a_renderer.FindVALPipelineState(a_pipelineStateType);
	const auto& l_pipelineState     = l_pipelineStateWeak.lock       ();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_pipelineState, "指定されたPipelineStateTypeに対応するPipelineStateが無効になっており。", {});

	const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

	l_directCommandList.SetupRenderPipeline(l_pipelineState);

	// 使用するルートシグネチャを戻り値にセット
	return l_pipelineState->GetREFUseRootSignature();
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::DrawRequestPerObjectBase::FetchVALTextureSRVDescriptorIndex(const std::shared_ptr<Texture>& a_texture, const TextureSystem& a_textureSystem, const Enum::DefaultTextureType a_defaultTextureType) const
{
	if (a_texture)
	{
		// テクスチャIDが無効でなければその値を返す
		const auto& l_textureRecord = a_texture->GetREFTextureRecord().lock();

		if (l_textureRecord &&
			l_textureRecord->GetVALSRVDescriptorIndex() != Constant::k_invalidDescriptorIndex)
		{
			return l_textureRecord->GetVALSRVDescriptorIndex();
		}
	}

	// SRVがTextureRecordから取得できない場合デフォルトテクスチャのSRVStorageIDを返す
	const auto& l_defaultTextureRecord = a_textureSystem.FetchVALDefaultTextureRecord(a_defaultTextureType).lock();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_defaultTextureRecord, "DefaultTextureRecordの取得に失敗しており、SRV用ディスクリプタインデックスの取得に失敗しました。", Constant::k_invalidDescriptorIndex);

	const auto l_defaultTextureSRVIndex = l_defaultTextureRecord->GetVALSRVDescriptorIndex();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_defaultTextureSRVIndex == Constant::k_invalidDescriptorIndex, "DefaultTextureRecordのSRVIndexが無効になっており、SRV用ディスクリプタインデックスの取得に失敗しました。", Constant::k_invalidDescriptorIndex);

	return l_defaultTextureSRVIndex;
}
FWK::TypeAlias::DescriptorIndex FWK::Graphics::DrawRequestPerObjectBase::FetchVALTextureSRVDescriptorIndex(const std::shared_ptr<TextureRecord>& a_textureRecord, const TextureSystem& a_textureSystem, const Enum::DefaultTextureType a_defaultTextureType) const
{
	if (a_textureRecord)
	{
		// テクスチャSRVDescriptorIndexが無効でなければその値を返す
		if (const auto l_srvIndex = a_textureRecord->GetVALSRVDescriptorIndex();
			l_srvIndex != Constant::k_invalidDescriptorIndex)
		{
			return l_srvIndex;
		}
	}

	// SRVDescriptorIndexがTextureRecordから取得できない場合デフォルトテクスチャのSRVDescriptorIndexを返す
	const auto& l_defaultTextureRecord = a_textureSystem.FetchVALDefaultTextureRecord(a_defaultTextureType).lock();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_defaultTextureRecord, "DefaultTextureRecordの取得に失敗しており、SRV用ディスクリプタインデックスの取得に失敗しました。", Constant::k_invalidDescriptorIndex);

	const auto l_defaultTextureSRVIndex = l_defaultTextureRecord->GetVALSRVDescriptorIndex();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_defaultTextureSRVIndex == Constant::k_invalidDescriptorIndex, "DefaultTexturerecordのSRVIndexが無効値です、SRV用ディスクリプタインデックスの取得に失敗しました。", Constant::k_invalidDescriptorIndex);

	return l_defaultTextureSRVIndex;
}