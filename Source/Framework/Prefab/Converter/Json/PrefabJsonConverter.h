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

		nlohmann::json Serialize(const Prefab& a_prefab) const;
	};
}