#pragma once

namespace FWK
{
	class NodeEditorAllocator final
	{
	public:

		 NodeEditorAllocator() = default;
		~NodeEditorAllocator() = default;

		void Deserialize(const nlohmann::json& a_rootJson);
		
		nlohmann::json Serialize() const;

		TypeAlias::NodeEditorID Allocate();

		void Release(const TypeAlias::NodeEditorID a_nodeEditorID);

		auto& GetMutableREFIsAllocatedList() { return m_isAllocatedList; }

		const auto& GetREFIsAllocatedList() const { return m_isAllocatedList; }
		
	private:

		std::vector<bool> m_isAllocatedList = {};

		std::queue<TypeAlias::NodeEditorID> m_freeNodeEditorIDQueue = {};

		Converter::NodeEditorAllocatorJsonConverter m_jsonConverter = {};
	};
}