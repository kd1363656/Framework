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

		void LoadPrefab();

		void SavePrefab() const;

		void SetGameObject(const std::weak_ptr<GameObject>& a_set) { m_gameObject = a_set; }

		void SetJson(const nlohmann::json& a_set) { m_json = a_set; }

		void SetFilePath(const std::filesystem::path& a_set) { m_filePath = a_set; }

		const auto& GetREFGameObject() const { return m_gameObject; }

		const auto& GetREFJson() const { return m_json; }

		const auto& GetREFFilePath() const { return m_filePath; }

	private:

		std::weak_ptr<GameObject> m_gameObject = {};

		Converter::PrefabJsonConverter m_jsonConverter = {};

		nlohmann::json m_json = {};

		std::filesystem::path m_filePath = {};
	};
}