#pragma once

namespace FWK::Graphics
{
    class ResourceContext final
    {
    public:

         ResourceContext() = default;
        ~ResourceContext() = default;

        void Deserialize    (const nlohmann::json& a_rootJson);
        bool PostDeserialize(const Device&         a_device);

        void ProcessPendingTextureUploads               ();
        void ProcessPendingStaticModelUploads           ();
        void ProcessPendingSkeletalAnimationModelUploads();

        void ProcessPendingMaterialUploads(const Device& a_device, const TypeAlias::DirectCommandQueue& a_directCommandQueue);

        void ReleaseCompletedDeferredResources(const TypeAlias::DirectCommandQueue& a_directCommandQueue);

        nlohmann::json Serialize() const;

        const auto& GetREFShaderCompiler() const { return m_shaderCompiler; }

        const auto& GetREFRTVDescriptorPool      () const { return m_rtvDescriptorPool; }
        const auto& GetREFCBVSRVUAVDescriptorPool() const { return m_cbvSRVUAVDescriptorPool; }
        const auto& GetREFDSVDescriptorPool      () const { return m_dsvDescriptorPool; }

        const auto& GetREFGPUMemoryAllocator() const { return m_gpuMemoryAllocator; }

        const auto& GetREFUploadSystem                () const { return m_uploadSystem; }
        const auto& GetREFTextureSystem               () const { return m_textureSystem; }
        const auto& GetREFMaterialSystem              () const { return m_materialSystem; }
        const auto& GetREFStaticModelSystem           () const { return m_staticModelSystem; }
        const auto& GetREFSkeletalAnimationModelSystem() const { return m_skeletalAnimationModelSystem; }

        const auto& GetREFResourceReleaseContext() const { return m_resourceReleaseContext; }

        auto& GetMutableREFRTVDescriptorPool      () { return m_rtvDescriptorPool; }
        auto& GetMutableREFCBVSRVUAVDescriptorPool() { return m_cbvSRVUAVDescriptorPool; }
        auto& GetMutableREFDSVDescriptorPool      () { return m_dsvDescriptorPool; }

        auto& GetMutableREFUploadSystem                () { return m_uploadSystem; }
        auto& GetMutableREFTextureSystem               () { return m_textureSystem; }
        auto& GetMutableREFMaterialSystem              () { return m_materialSystem; }
        auto& GetMutableREFStaticModelSystem           () { return m_staticModelSystem; }
        auto& GetMutableREFSkeletalAnimationModelSystem() { return m_skeletalAnimationModelSystem; }

        auto& GetMutableREFResourceReleaseContext() { return m_resourceReleaseContext; }

    private:

        ShaderCompiler m_shaderCompiler = {};

        TypeAlias::RTVDescriptorPool       m_rtvDescriptorPool       = {};
        TypeAlias::CBVSRVUAVDescriptorPool m_cbvSRVUAVDescriptorPool = {};
        TypeAlias::DSVDescriptorPool       m_dsvDescriptorPool       = {};

        GPUMemoryAllocator m_gpuMemoryAllocator = {};
        UploadSystem       m_uploadSystem       = {};

        // MaterialはTextureを参照し、ModelはMaterialを参照するため、
        // 破棄される順番(宣言と逆順)がModel -> Material -> Textureになるように並べる
        TextureSystem                m_textureSystem                = {};
        MaterialSystem               m_materialSystem               = {};
        StaticModelSystem            m_staticModelSystem            = {};
        SkeletalAnimationModelSystem m_skeletalAnimationModelSystem = {};

        ResourceReleaseContext m_resourceReleaseContext = {};

        Converter::ResourceContextJsonConverter m_jsonConverter = {};
    };
}