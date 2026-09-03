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

		void Load(const nlohmann::json& a_rootJson, Prefab& a_prefab) const;

		bool Save(const std::filesystem::path& a_filePath, Prefab& a_prefab) const;

	private:

		static constexpr std::string_view k_prefabJsonKey     = "Prefab";
	    static constexpr std::string_view k_prefabNameJsonKey = "PrefabName";
	};
}