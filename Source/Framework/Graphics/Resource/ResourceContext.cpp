#include "ResourceContext.h"

void FWK::Graphics::ResourceContext::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::ResourceContext::PostDeserialize(const Device& a_device)
{
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_shaderCompiler.Create(), "ShaderCompilerの作成処理に失敗しました。", false);

    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_rtvDescriptorPool.Create(a_device), "RTVDescriptorPoolの作成処理に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_srvDescriptorPool.Create(a_device), "SRVDescriptorPoolの作成処理に失敗しました。", false);

    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_gpuMemoryAllocator.Create(a_device), "GPUMemoryAllocatorの作成処理に失敗しました。",   false);
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_uploadSystem.Create(a_device),       "アップロードシステムの作成処理に失敗しました。", false);

    // 作成したGPUMemoryAllocatorとUploadSystemを使用する
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_textureSystem.Create(a_device, m_gpuMemoryAllocator, m_srvDescriptorPool), "テクスチャシステムの作成処理に失敗しました。", false);

    // デフォルトテクスチャの登録をここで行う
    m_uploadSystem.SubmitPendingTextureCopyBatchIfNeededAndWait(m_textureSystem);
    m_textureSystem.RegisterPendingTextures                    ();

    return true;
}

void FWK::Graphics::ResourceContext::BeginFrame(const DirectCommandQueue& a_directCommandQueue)
{
    // TextureSystemにPending中のテクスチャがあれば、
    // UPLOADヒープ上の中間バッファからDEFAULTヒープ上のTextureResourceへコピーする。
    // コピーが完了した後、TextureRecordをTextureStorageへ正式登録する
    m_uploadSystem.SubmitPendingTextureCopyBatchIfNeededAndWait(m_textureSystem);
    m_textureSystem.RegisterPendingTextures                    ();

    // 参照カウントが0になったRecordからQueueへ積まれたGPUResource/SRVを、
	// GPUのFence完了後に安全に解放する
    m_resourceReleaseContext.ReleaseAvailableDeferredResources(a_directCommandQueue, m_srvDescriptorPool);
}

nlohmann::json FWK::Graphics::ResourceContext::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}