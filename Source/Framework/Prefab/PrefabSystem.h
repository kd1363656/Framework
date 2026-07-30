#pragma once

namespace FWK
{
	class PrefabSystem final
	{
	private:

		using PrefabMap = std::unordered_map<std::string, Prefab, Struct::StringHash, std::equal_to<>>;

	public:

		 PrefabSystem() = default;
		~PrefabSystem() = default;

		void INIT       ();
		void Deserialize(const nlohmann::json& a_rootJson);

		void AddPrefabMap(const std::string& a_prefabName, const Prefab& a_prefab);
		void RemovePrefab(const std::string& a_prefabName);

		nlohmann::json Serialize() const;

		const auto& GetREFPrefabMap() const { return m_prefabMap; }

	private:

		PrefabMap m_prefabMap = {};

		Converter::PrefabSystemJsonConverter m_jsonConverter = {};
	};
}