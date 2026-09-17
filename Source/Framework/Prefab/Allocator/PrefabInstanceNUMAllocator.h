#pragma once

namespace FWK
{
    class PrefabInstanceNUMAllocator final
    {
    public:

         PrefabInstanceNUMAllocator() = default;
        ~PrefabInstanceNUMAllocator() = default;

        void Deserialize(const nlohmann::json& a_rootJson);

        nlohmann::json Serialize() const;

        void Resize(const std::size_t& a_size, const bool a_setValue);

        TypeAlias::PrefabSceneInstanceNUM Allocate();

        void Release(const TypeAlias::PrefabSceneInstanceNUM a_prefabInstanceNUM);

        void SetIsAllocatedList(std::vector<bool>&& a_set) { m_isAllocatedList = std::move(a_set); }

        const auto& GetREFIsAllocatedList() const { return m_isAllocatedList; }

    private:

        void RebuildFreePrefabIDQueue();

        std::vector<bool> m_isAllocatedList = {};

        std::queue<TypeAlias::PrefabSceneInstanceNUM> m_freePrefabInstanceNUMQueue = {};

        Converter::PrefabInstanceNUMAllocatorJsonConverter m_jsonConverter = {};
    };
}