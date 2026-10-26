#pragma once

namespace FWK::Graphics
{
    // .materialファイルからMaterialを読み込み、参照数とGPUDataBufferのアップロードを管理するクラス
    //
    // ・.materialファイルに保存された型名からFactoryで派生クラスを作るため、Materialの型を知らなくても読み込める
    // ・Inspectorで値が変わったMaterialは、GPUDataBufferを作り直してアップロードし直す
    class MaterialSystem final
    {
    private:

        using PendingMaterialBatchUploadRecordMap = std::unordered_map<std::wstring, Struct::MaterialBatchUploadRecord, Struct::WStringHash, std::equal_to<>>;

    public:

         MaterialSystem() = default;
        ~MaterialSystem() = default;

        void Deserialize(const nlohmann::json& a_rootJson);
        bool Create     ();

        // 読み込めない場合は、デフォルトマテリアルを返す
        Struct::MaterialLoadResult LoadMaterialForBatchUpload(const Device&                             a_device,
                                                              const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
                                                              const std::filesystem::path&              a_filePath,
                                                                    TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

        // .materialファイルを持たないMesh用のデフォルトマテリアル(StandardLitMaterial)を返す
        Struct::MaterialLoadResult FetchDefaultMaterialForBatchUpload(const Device& a_device, const GPUMemoryAllocator& a_gpuMemoryAllocator, TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

        nlohmann::json Serialize() const;

        // Inspectorで値が変わったMaterialのGPUDataBufferを作り直し、アップロード待ちにする
        void RebuildDirtyMaterialGPUDataBuffers(const Device&                             a_device,
                                                const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
                                                const TypeAlias::DirectCommandQueue&      a_directCommandQueue,
                                                      TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool,
                                                      ResourceReleaseContext&             a_resourceReleaseContext);

        // ※ 注意
        // UploadSystem側でMaterialのバッファーのコピーが終わっていること前提
        void RegisterPendingMaterials();

        bool AddMaterialReferenceCount     (const std::weak_ptr<MaterialBase>& a_material);
        bool SubtractMaterialReferenceCount(const std::weak_ptr<MaterialBase>& a_material, const TypeAlias::DirectCommandQueue& a_directCommandQueue, ResourceReleaseContext& a_resourceReleaseContext);

        const auto& GetREFPendingMaterialBatchUploadRecordMap() const { return m_pendingMaterialBatchUploadRecordMap; }

        const auto& GetREFPendingBufferUploadCommandList() const { return m_pendingBufferUploadCommandList; }

        const auto& GetREFMaterialStorage() const { return m_materialStorage; }

        auto& GetMutableREFMaterialStorage() { return m_materialStorage; }

    private:

        // .materialファイルに保存された型名から、Factoryで派生クラスを作って読み込む
        std::shared_ptr<MaterialBase> CreateMaterialFromBinaryFile(const std::filesystem::path& a_filePath);

        bool TryResolveCachedMaterialResult(const std::filesystem::path& a_filePath, Struct::MaterialLoadResult& a_materialLoadResult);

        PendingMaterialBatchUploadRecordMap m_pendingMaterialBatchUploadRecordMap = {};

        std::vector<StaticStructuredBuffer::BufferUploadCommand> m_pendingBufferUploadCommandList = {};

        std::vector<std::weak_ptr<MaterialBase>> m_materialList = {};

        std::shared_ptr<MaterialBase> m_defaultMaterial = nullptr;

        AssetStorage<MaterialBase> m_materialStorage = {};

        Converter::MaterialBaseBinaryConverter m_materialBaseBinaryConverter = {};
        Converter::MaterialSystemJsonConverter m_jsonConverter               = {};
    };
}