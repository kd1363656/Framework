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

		void LoadGameObjectPrefab(Prefab& a_prefab) const;

		void Deserialize(const nlohmann::json& a_rootJson, Prefab& a_prefab) const;

		nlohmann::json Serialize(Prefab& a_prefab) const;

	private:

		bool SaveGameObjectPrefab(Prefab& a_prefab) const;

		static constexpr std::string_view k_prefabTypeJsonKey = "PrefabType";
		static constexpr std::string_view k_filePathJsonKey   = "FilePath";
	};
}