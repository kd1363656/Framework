#pragma once

namespace FWK::Graphics
{
    class ModelRenderSystem final
    {
    private:

        using ModelMeshTypeDrawItemListMapList = std::array<std::unordered_map<TypeAlias::StaticTypeID, std::shared_ptr<ModelDrawItemList>>, static_cast<std::size_t>(Enum::ModelMeshType::Count)>;

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

        template <Concept::IsModelMaterialRenderTableElementConcept MaterialElementType>
        void RecordDraw(const RootSignature&      a_rootSignature,
                        const DirectCommandList&  a_directCommandList,
                        const std::size_t&        a_frameIndex,
                        const Enum::ModelMeshType a_meshType) const
        {
            // パスが描くマテリアルの種類を、マテリアルのGPUデータの型で指定する
            // 例 : StaticModelStandardLitPassなら RecordDraw<ModelStandardLitMaterialGPUData>(..., Enum::ModelMeshType::Static)
            //      → 「Static × StandardLit」の描画項目の一覧だけを描く
            // Conceptで、MATERIAL版のマクロを書いた型だけを受け付ける(ModelObjectGPUDataなどを渡すとコンパイルエラーになる)
            const auto& l_materialTableINFO = MaterialElementType::GetREFModelRenderTableINFO();
            const auto* l_typeINFO          = l_materialTableINFO.k_typeINFO;

            if (!l_typeINFO) { return; }

            RecordMaterialDraw(a_rootSignature,
                               a_directCommandList,
                               a_frameIndex,
                               a_meshType,
                               l_typeINFO->k_staticTypeID);
        }

        void RecordDrawWithoutMaterial(const RootSignature&      a_rootSignature,
                                       const DirectCommandList&  a_directCommandList,
                                       const std::size_t&        a_frameIndex,
                                       const Enum::ModelMeshType a_meshType) const;

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
            const auto* l_typeINFO = l_tableINFO.k_typeINFO;

            if (!l_typeINFO) { return DescriptorHeap::k_invalidDescriptorIndex; }

            return FetchVALTableSRVDescriptorIndex(l_typeINFO->k_staticTypeID);
        }

        TypeAlias::DescriptorIndex FetchVALTableSRVDescriptorIndex(const TypeAlias::StaticTypeID a_tableStaticTypeID) const;

        template <Concept::IsModelRenderTableElementConcept ElementType>
        std::weak_ptr<GPUElementTable> FindVALTable() const
        {
            // 要素の型から、その型のテーブルを探す
            // 例 : FindVALTable<ModelMeshGPUData>() → メッシュのテーブル
            // 型で指定するので、別の種類のテーブルを取り違えることがない(取り違えるとコンパイルエラーか、書き込みの大きさのアサートで気づける)
            const auto& l_tableINFO = ElementType::GetREFModelRenderTableINFO();
            const auto* l_typeINFO = l_tableINFO.k_typeINFO;

            if (!l_typeINFO) { return {}; }

            return FindVALTable(l_typeINFO->k_staticTypeID);
        }

        std::weak_ptr<GPUElementTable> FindVALTable(const TypeAlias::StaticTypeID a_tableStaticTypeID) const;

        std::weak_ptr<ModelDrawItemList> FindVALDrawItemList(const Enum::ModelMeshType a_meshType, const TypeAlias::StaticTypeID a_materialTableStaticTypeID) const;

        const auto& GetREFTableMap() const { return m_tableMap; }

    private:

        void RecordMaterialDraw(const RootSignature&          a_rootSignature,
                                const DirectCommandList&      a_directCommandList,
                                const std::size_t&            a_frameIndex,
                                const Enum::ModelMeshType     a_meshType,
                                const TypeAlias::StaticTypeID a_materialTableStaticTypeID) const;

        const std::unordered_map<TypeAlias::StaticTypeID, std::shared_ptr<ModelDrawItemList>>* FindPTRDrawItemListMap(const Enum::ModelMeshType a_meshType) const;

        std::unordered_map<TypeAlias::StaticTypeID, std::shared_ptr<GPUElementTable>> m_tableMap = {};

        ModelMeshTypeDrawItemListMapList m_meshTypeDrawItemListMapList = {};

        Converter::ModelRenderSystemJsonConverter m_jsonConverter = {};
    };
}