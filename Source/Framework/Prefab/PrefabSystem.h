#pragma once

namespace FWK
{
	class PrefabSystem final
	{
	private:

		using PrefabMap = std::unordered_map<std::filesystem::path, Prefab>;

	public:

		 PrefabSystem() = default;
		~PrefabSystem() = default;

		void INIT       ();
		void Deserialize(const nlohmann::json& a_rootJson);


		void AddPrefabMap(const std::filesystem::path& a_filePath, const Prefab& a_prefab);
		void RemovePrefab(const std::filesystem::path& a_filePath);

		nlohmann::json Serialize() const;

	private:

		PrefabMap m_prefabMap = {};
	};
}