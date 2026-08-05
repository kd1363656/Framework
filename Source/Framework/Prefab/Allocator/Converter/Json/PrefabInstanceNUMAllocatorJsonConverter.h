#pragma once

namespace FWK
{
	class PrefabInstanceNUMAllocator;
}

namespace FWK::Converter
{
	class PrefabInstanceNUMAllocatorJsonConverter final
	{
	public:

		 PrefabInstanceNUMAllocatorJsonConverter() = default;
		~PrefabInstanceNUMAllocatorJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, PrefabInstanceNUMAllocator& a_prefabInstanceNUMAllocator) const;

		nlohmann::json Serialize(const PrefabInstanceNUMAllocator& a_prefabInstanceNUMAllocator) const;

	private:

		static constexpr std::string_view k_isAllocatedListJsonKey = "IsAllocatedList";
	};
}