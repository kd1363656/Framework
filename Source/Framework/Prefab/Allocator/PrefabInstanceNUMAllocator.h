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

		TypeAlias::PrefabSceneInstanceNUM Allocate();

		void Release(const TypeAlias::PrefabSceneInstanceNUM a_prefabInstanceNUM);

		auto& GetMutableREFIsAllocatedList() { return m_isAllocatedList; }

		const auto& GetREFIsAllocatedList() const { return m_isAllocatedList; }

	private:

		void RebuildFreePrefabIDQueue();

		std::vector<bool> m_isAllocatedList = {};

		std::queue<TypeAlias::PrefabSceneInstanceNUM> m_freePrefabInstanceNUMQueue = {};

		Converter::PrefabInstanceNUMAllocatorJsonConverter m_jsonConverter = {};
	};
}