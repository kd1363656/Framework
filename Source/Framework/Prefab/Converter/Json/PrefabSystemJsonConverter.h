#pragma once

namespace FWK
{
	class PrefabSystem;
}

namespace FWK::Converter
{
	class PrefabSystemJsonConverter final
	{
	public:

		 PrefabSystemJsonConverter() = default;
		~PrefabSystemJsonConverter() = default;

		void           Deserialize(const nlohmann::json& a_rootJson, PrefabSystem& a_prefabSystem) const;
		nlohmann::json Serialize  (      PrefabSystem&   a_prefabSystem)                           const;

	private:

		static constexpr std::string_view k_prefabMapJsonKey                  = "PrefabMap";
		static constexpr std::string_view k_prefabFilePathJsonKey             = "PrefabFilePath";
		static constexpr std::string_view k_prefabNameJsonKey                 = "PrefabName";
		static constexpr std::string_view k_prefabInstanceNUMAllocatorJsonKey = "PrefabInstanceNUMAllocator";
		static constexpr std::string_view k_prefabJsonKey                     = "Prefab";
	};
}