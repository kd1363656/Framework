#pragma once

namespace FWK
{
	class Prefab;
}

namespace FWK::Converter
{
	class PrefabJsonConverter final
	{
	public:

		 PrefabJsonConverter() = default;
		~PrefabJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Prefab& a_prefab) const;

		nlohmann::json Serialize(const Prefab& a_prefab) const;

	private:

		void LoadGameObjectPrefab(Prefab& a_prefab) const;
		void SaveGameObjectPrefab(Prefab& a_prefab) const;

		static constexpr std::string_view k_gameObjectJsonKey = "GameObject";
		static constexpr std::string_view k_filePathJsonKey   = "FilePath";
	};
}