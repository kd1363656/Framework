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

		void Deserialize(const nlohmann::json& a_rootJson, Prefab& a_prefab);

		nlohmann::json Serialize(const Prefab& a_prefab) const;

	private:

		static constexpr std::string_view k_gameObjectJsonKey = "GameObject";
	};
}