#pragma once

namespace FWK::Graphics
{
    class ModelDrawItemList final
    {
    public:

         ModelDrawItemList();
        ~ModelDrawItemList();

        ModelDrawItemList(const ModelDrawItemList&)  = delete;
        ModelDrawItemList(      ModelDrawItemList&&) = delete;

        ModelDrawItemList& operator=(const ModelDrawItemList&)  = delete;
        ModelDrawItemList& operator=(      ModelDrawItemList&&) = delete;

        bool Create(const std::size_t& a_frameCount);

        void RecordDraw(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const std::size_t& a_frameIndex);

        std::uint64_t Register(std::vector<std::vector<Struct::ModelDrawItem>>&& a_frameDrawItemList);

        void Unregister(const std::uint64_t& a_registrationID);

        static constexpr std::uint64_t k_invalidRegistrationID = std::numeric_limits<std::uint64_t>::max();

    private:

        void RebuildFrameDrawItemListIfNeeded();

        static constexpr std::size_t k_emptyFrameCount = 0ULL;
        static constexpr std::size_t k_noErasedCount   = 0ULL;

        static constexpr std::uint64_t k_firstRegistrationID = 0ULL;

        std::vector<std::vector<Struct::ModelDrawItem>> m_frameDrawItemList;

        std::unordered_map<std::uint64_t, std::vector<std::vector<Struct::ModelDrawItem>>> m_registrationMap;

        std::size_t m_frameCount;

        std::uint64_t m_nextRegistrationID;

        bool m_isDirty;
    };
}