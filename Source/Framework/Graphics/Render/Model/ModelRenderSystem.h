#pragma once

namespace FWK::Graphics
{
    class ModelRenderSystem final
    {
    private:

        using ModelRenderTableMap = std::unordered_map<TypeAlias::StaticTypeID, std::shared_ptr<GPUElementTable>>;

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
                          TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool) const;

        void RecordUpload(const DirectCommandList& a_directCommandList, const std::size_t& a_frameIndex) const;

        nlohmann::json Serialize() const;

        void AddTable(const Struct::ModelRenderTableINFO& a_tableINFO, const UINT a_capacity);

        Struct::RCModelTable FetchVALRCModelTable() const;

        template <Concept::IsModelRenderTableElementConcept ElementType>
        TypeAlias::DescriptorIndex FetchVALTableSRVDescriptorIndex() const
        {
            // 要素の型(ModelObjectGPUDataなど)から、マクロが作ったテーブルの情報を取り出し、
            // そのStaticTypeIDでテーブルを探して、SRVの番号を返す
            // 例 : FetchVALTableSRVDescriptorIndex<ModelObjectGPUData>() → オブジェクトのテーブルのSRVの番号
            const auto& l_tableINFO = ElementType::GetREFModelRenderTableINFO();

            return FetchVALTableSRVDescriptorIndex(l_tableINFO.k_typeINFO->k_staticTypeID);
        }

        TypeAlias::DescriptorIndex FetchVALTableSRVDescriptorIndex(const TypeAlias::StaticTypeID a_tableStaticTypeID) const;

        template <Concept::IsModelRenderTableElementConcept ElementType>
        std::weak_ptr<GPUElementTable> FindVALTable() const
        {
            // 要素の型から、その型のテーブルを探す
            // 例 : FindVALTable<ModelMeshGPUData>() → メッシュのテーブル
            // 型で指定するので、別の種類のテーブルを取り違えることがない(取り違えるとコンパイルエラーか、書き込みの大きさのアサートで気づける)
            const auto& l_tableINFO = ElementType::GetREFModelRenderTableINFO();

            return FindVALTable(l_tableINFO.k_typeINFO->k_staticTypeID);
        }

        std::weak_ptr<GPUElementTable> FindVALTable(const TypeAlias::StaticTypeID a_tableStaticTypeID) const;

        const auto& GetREFTableMap() const { return m_tableMap; }

    private:

        ModelRenderTableMap m_tableMap = {};

        Converter::ModelRenderSystemJsonConverter m_jsonConverter = {};
    };
}