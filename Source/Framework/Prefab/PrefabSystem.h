#pragma once

namespace FWK
{
	class GameObject;
}

namespace FWK
{
	class PrefabSystem final
	{
	private:

		using PrefabMap = std::unordered_map<boost::uuids::uuid, Struct::PrefabData>;

	public:

		 PrefabSystem() = default;
		~PrefabSystem() = default;

		void INIT                         ();
		void Deserialize                  (const nlohmann::json&            a_rootJson);
		void CachePrefabGameObjectIfNeeded(const std::weak_ptr<GameObject>& a_gameObject);

		void AddPrefabMap(const boost::uuids::uuid& a_prefabUUID, const Struct::PrefabData& a_prefabData);
		void RemovePrefab(const boost::uuids::uuid& a_prefabUUID);

		nlohmann::json Serialize();

		TypeAlias::PrefabInstanceNUM AllocatePrefabInstanceNUM(const boost::uuids::uuid& a_prefabUUID);

		void ReleasePrefabInstanceNUM(const boost::uuids::uuid& a_prefabUUID, const TypeAlias::PrefabInstanceNUM a_prefabInstanceNUM);

		const Prefab* FindPTRPrefab(const boost::uuids::uuid& a_prefabUUID) const;

		const auto& GetREFPrefabMap() const { return m_prefabMap; }

		auto& GetMutableREFPrefabMap() { return m_prefabMap; }

	private:

		PrefabMap m_prefabMap = {};

		Converter::PrefabSystemJsonConverter m_jsonConverter = {};
	};
}