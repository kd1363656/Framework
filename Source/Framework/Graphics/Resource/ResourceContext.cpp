#include "ResourceContext.h"

void FWK::Graphics::ResourceContext::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::ResourceContext::PostDeserialize(const Device& a_device)
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_shaderCompiler.Create(), "ShaderCompilerの作成処理に失敗しました。", false);

    FWK_ASSERT_RETURN_VALUE_IF(!m_rtvDescriptorPool.Create      (a_device), "RTVDescriptorPoolの作成処理に失敗しました。",       false);
    FWK_ASSERT_RETURN_VALUE_IF(!m_cbvSRVUAVDescriptorPool.Create(a_device), "CBVSRVUAVDescriptorPoolの作成処理に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF(!m_dsvDescriptorPool.Create      (a_device), "DSVDescriptorPoolの作成処理に失敗しました。",       false);

    FWK_ASSERT_RETURN_VALUE_IF(!m_gpuMemoryAllocator.Create(a_device), "GPUMemoryAllocatorの作成処理に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF(!m_uploadSystem.Create(a_device),       "UploadSystemの作成処理に失敗しました。",       false);

    // 作成したGPUMemoryAllocatorとUploadSystemを使用する
    FWK_ASSERT_RETURN_VALUE_IF(!m_textureSystem.Create(a_device, m_gpuMemoryAllocator, m_cbvSRVUAVDescriptorPool), "TextureSystemの作成処理に失敗しました。",                false);
    FWK_ASSERT_RETURN_VALUE_IF(!m_staticModelSystem.Create(),                                                      "StaticModelSystemの作成処理に失敗しました。",            false);
    FWK_ASSERT_RETURN_VALUE_IF(!m_skeletalAnimationModelSystem.Create(),                                           "SkeletalAnimationModelSystemの作成処理に失敗しました。", false);

    // デフォルトテクスチャの登録をここで行う
    m_uploadSystem.SubmitPendingTextureCopyBatchIfNeededAndWait(m_textureSystem);
    m_textureSystem.RegisterPendingTextures                    ();

    return true;
}

void FWK::Graphics::ResourceContext::ProcessPendingTextureUploads()
{
    // TextureSystemにPending中のテクスチャがあれば、
    // UPLOADヒープ上の中間バッファからDEFAULTヒープ上のTextureResourceへコピーする。
    // コピーが完了した後、TextureRecordをTextureStorageへ正式登録する
    m_uploadSystem.SubmitPendingTextureCopyBatchIfNeededAndWait(m_textureSystem);
    m_textureSystem.RegisterPendingTextures                    ();
}
void FWK::Graphics::ResourceContext::ProcessPendingStaticModelUploads()
{
    // StaticModelSystemにPending中のスタティックモデルがあれば、
    // UPLOADヒープ上の中間バッファからDEFAULTヒープ上のBufferResourceへコピーする。
    // コピーが完了した後、StaticModelRecordをStaticModelStorageへ正式登録する
    m_uploadSystem.SubmitPendingStaticModelBatchIfNeededAndWait(m_staticModelSystem);
    m_staticModelSystem.RegisterPendingStaticModels            ();
}
void FWK::Graphics::ResourceContext::ProcessPendingSkeletalAnimationModelUploads()
{
    // SkeletalAnimationModelSystemにPending中のモデルがあれば、
    // UPLOADヒープからDEFAULTヒープへ各Bufferをコピーする
    m_uploadSystem.SubmitPendingSkeletalAnimationModelBatchIfNeededAndWait(m_skeletalAnimationModelSystem);

    // GPUコピー完了後、RecordをAssetStorageへ正式登録する
    m_skeletalAnimationModelSystem.RegisterPendingSkeletalAnimationModels();
}

void FWK::Graphics::ResourceContext::ReleaseCompletedDeferredResources(const TypeAlias::DirectCommandQueue & a_directCommandQueue)
{
    // 参照カウントが0になったRecordからQueueへ積まれたGPUResource/SRVを、
	// GPUのFence完了後に安全に解放する
    m_resourceReleaseContext.ReleaseAvailableDeferredResources(a_directCommandQueue,
                                                               m_rtvDescriptorPool,
                                                               m_cbvSRVUAVDescriptorPool,
                                                               m_dsvDescriptorPool);
}

nlohmann::json FWK::Graphics::ResourceContext::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}