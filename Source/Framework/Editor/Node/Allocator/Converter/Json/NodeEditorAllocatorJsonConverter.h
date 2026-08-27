#pragma once

namespace FWK
{
	class NodeEditorAllocator;
}

namespace FWK::Converter
{
	class NodeEditorAllocatorJsonConverter final
	{
	public:

		 NodeEditorAllocatorJsonConverter() = default;
		~NodeEditorAllocatorJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, NodeEditorAllocator& a_nodeEditorAllocator) const;

		nlohmann::json Serialize(const NodeEditorAllocator& a_nodeEditorAllocator) const;

	private:

		static constexpr std::string_view k_isAllocatedListJsonKey = "IsAllocatedList";
	};
}