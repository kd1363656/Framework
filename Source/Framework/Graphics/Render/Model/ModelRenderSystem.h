#pragma once

namespace FWK::Graphics
{
    class ModelRenderSystem final
    {
    private:

        using ModelRenderTableMap = std::unordered_map<Enum::ModelRenderTableType, std::shared_ptr<GPUElementTable>>;

    public:

         ModelRenderSystem() = default;
        ~ModelRenderSystem() = default;

        ModelRenderSystem(const ModelRenderSystem&)  = delete;
        ModelRenderSystem(      ModelRenderSystem&&) = delete;

        ModelRenderSystem& operator=(const ModelRenderSystem&)  = delete;
        ModelRenderSystem& operator=(      ModelRenderSystem&&) = delete;

        void Deserialize(const nlohmann::json& a_rootJson);

        bool Create(const Device&                             a_device,
                    const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
                    const std::size_t&                        a_frameCount,
                          TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

        void RecordUpload(const DirectCommandList& a_directCommandList, const std::size_t& a_frameIndex) const;

        nlohmann::json Serialize() const;

        void SetTableSettingList(std::vector<Struct::ModelRenderTableSetting>&& a_set) { m_tableSettingList = std::move(a_set); }

        Struct::RCModelTable FetchVALRCModelTable() const;

        TypeAlias::DescriptorIndex FetchVALTableSRVDescriptorIndex(const Enum::ModelRenderTableType a_type) const;

        std::weak_ptr<GPUElementTable> FindVALTable(const Enum::ModelRenderTableType a_type) const;

        const auto& GetREFTableSettingList() const { return m_tableSettingList; }

    private:

        UINT FetchVALElementByteStride(const Enum::ModelRenderTableType a_type) const;

        UINT FetchVALCapacity(const Enum::ModelRenderTableType a_type) const;

        static constexpr UINT k_invalidElementByteStride = 0U;

        std::vector<Struct::ModelRenderTableSetting> m_tableSettingList = {};

        ModelRenderTableMap m_tableMap = {};

        Converter::ModelRenderSystemJsonConverter m_jsonConverter = {};
    };
}