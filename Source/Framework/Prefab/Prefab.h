#pragma once

namespace FWK
{
	class GameObject;
}

namespace FWK
{
	class Prefab final
	{
	public:

		 Prefab() = default;
		~Prefab() = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		nlohmann::json Serialize();
		
		void SetGameObject(const std::weak_ptr<GameObject>& a_set) { m_gameObject = a_set; }

		void SetJson(const nlohmann::json& a_set) { m_json = a_set; }

		void SetFilePath(const std::filesystem::path& a_set) { m_filePath = a_set; }

		void SetPrefabType(const Enum::PrefabType a_set) { m_prefabType = a_set; }

		void SetPrefabName(const std::string& a_set) { m_prefabName = a_set; }

		const auto& GetREFGameObject() const { return m_gameObject; }

		const auto& GetREFJson() const { return m_json; }

		const auto& GetREFFilePath() const { return m_filePath; }

		const auto& GetREFPrefabName() const { return m_prefabName; }

		auto GetVALPrefabType() const { return m_prefabType; }

	private:

		std::weak_ptr<GameObject> m_gameObject = {};

		Converter::PrefabJsonConverter m_jsonConverter = {};

		nlohmann::json m_json = {};

		std::filesystem::path m_filePath = {};

		std::string m_prefabName = {};

		Enum::PrefabType m_prefabType = Enum::PrefabType::GameObject;
	};
}