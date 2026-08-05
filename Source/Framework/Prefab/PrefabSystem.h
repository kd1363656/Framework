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

		using PrefabMap = std::unordered_map<std::string, Struct::PrefabData, Struct::StringHash, std::equal_to<>>;

	public:

		 PrefabSystem() = default;
		~PrefabSystem() = default;

		void INIT        ();
		void Deserialize (const nlohmann::json& a_rootJson);
		
		void AddPrefabMap(const std::string& a_prefabName, const Struct::PrefabData& a_prefabData);
		void RemovePrefab(const std::string& a_prefabName);

		nlohmann::json Serialize() const;

		TypeAlias::PrefabInstanceNUM AllocateVALPrefabInstanceNUM(const std::string& a_prefabName);

		void ReleasePrefabInstanceNUM(const std::string& a_prefabName, const TypeAlias::PrefabInstanceNUM a_prefabInstanceNUM);

		const Prefab* FindPTRPrefab(const std::string& a_prefabName) const;

		const auto& GetREFPrefabMap() const { return m_prefabMap; }

	private:

		PrefabMap m_prefabMap = {};

		Converter::PrefabSystemJsonConverter m_jsonConverter = {};
	};
}