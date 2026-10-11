#pragma once

namespace FWK::Graphics
{
    class ModelDrawRegistration final
    {
    private:

        struct DrawItemListRegistration final
        {
            std::weak_ptr<ModelDrawItemList> m_drawItemList = {};

            std::uint64_t m_registrationID = ModelDrawItemList::k_invalidRegistrationID;
        };

    public:

         ModelDrawRegistration();
        ~ModelDrawRegistration();

        ModelDrawRegistration(const ModelDrawRegistration&)  = delete;
        ModelDrawRegistration(      ModelDrawRegistration&&) = delete;

        ModelDrawRegistration& operator=(const ModelDrawRegistration&)  = delete;
        ModelDrawRegistration& operator=(      ModelDrawRegistration&&) = delete;

        void Register(const std::vector<std::vector<std::uint32_t>>& a_frameMeshIndexList,
                      const std::vector<std::uint32_t>&              a_meshletCountList,
                      const std::vector<ModelMaterial>&              a_materialList,
                      const Enum::ModelMeshType                      a_meshType,
                      const std::uint32_t                            a_objectIndex);

        void Unregister();

    private:

        std::unordered_map<TypeAlias::StaticTypeID, std::vector<std::vector<Struct::ModelDrawItem>>> BuildMaterialFrameDrawItemListMap(const std::vector<std::vector<std::uint32_t>>& a_frameMeshIndexList,
                                                                                                                                       const std::vector<std::uint32_t>&              a_meshletCountList,
                                                                                                                                       const std::vector<ModelMaterial>&              a_materialList,
                                                                                                                                       const std::uint32_t                            a_objectIndex) const;

        void RegisterDrawItemList(const Enum::ModelMeshType a_meshType, const TypeAlias::StaticTypeID a_materialTableStaticTypeID, std::vector<std::vector<Struct::ModelDrawItem>>&& a_frameDrawItemList);

        static UINT CalculateAmplificationShaderGroupCount(const std::uint32_t a_meshletCount);

        static constexpr UINT k_dispatchMeshThreadGroupCountY = 1U;
        static constexpr UINT k_dispatchMeshThreadGroupCountZ = 1U;

        std::vector<DrawItemListRegistration> m_drawItemListRegistrationList;
    };
}