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

		void Load(const std::filesystem::path& a_filePath);

		bool Save(const std::filesystem::path& a_filePath);
		
		void SetGameObject(const std::weak_ptr<GameObject>& a_set) { m_gameObject = a_set; }

		void SetJson(const nlohmann::json& a_set) { m_json = a_set; }

		void SetPrefabName(const std::string& a_set) { m_prefabName = a_set; }

		const auto& GetREFGameObject() const { return m_gameObject; }

		const auto& GetREFJson() const { return m_json; }

		const auto& GetREFPrefabName() const { return m_prefabName; }

	private:

		std::weak_ptr<GameObject> m_gameObject = {};

		Converter::PrefabJsonConverter m_jsonConverter = {};

		nlohmann::json m_json = {};

		std::string m_prefabName = {};
	};
}