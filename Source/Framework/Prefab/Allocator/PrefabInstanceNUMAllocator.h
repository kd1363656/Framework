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

		TypeAlias::PrefabInstanceNUM Allocate();

		void Release(const TypeAlias::PrefabInstanceNUM a_prefabInstanceNUM);

		auto& GetMutableREFIsAllocatedList() { return m_isAllocatedList; }

		const auto& GetREFIsAllocatedList() const { return m_isAllocatedList; }

	private:

		void RebuildFreePrefabIDQueue();

		std::vector<bool> m_isAllocatedList = {};

		std::queue<TypeAlias::PrefabInstanceNUM> m_freePrefabInstanceNUMQueue = {};

		Converter::PrefabInstanceNUMAllocatorJsonConverter m_jsonConverter = {};
	};
}