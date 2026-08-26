#pragma once

namespace FWK
{
	class NodeEditorAllocator final
	{
	public:

		 NodeEditorAllocator() = default;
		~NodeEditorAllocator() = default;

		void Deserialize(const nlohmann::json& a_rootJson);
		bool Create     ();

		nlohmann::json Serialize() const;

		TypeAlias::NodeEditorID Allocate();

		void Release(const TypeAlias::NodeEditorID a_nodeEditorID);

		void SetNodeEditorIDCapacity(const TypeAlias::NodeEditorID a_set) { m_nodeEditorIDCapacity = a_set; }
		void SetNextNodeEditorID    (const TypeAlias::NodeEditorID a_set) { m_nextNodeEditorID     = a_set; }

		auto GetVALNodeEditorIDCapacity() const { return m_nodeEditorIDCapacity; }
		auto GetVALNextNodeEditorID    () const { return m_nextNodeEditorID; }

	private:


		bool IsValidNodeEditorID(const TypeAlias::NodeEditorID a_storageID) const;

		static constexpr TypeAlias::NodeEditorID k_initialNextEditorID     = 0;
		static constexpr TypeAlias::NodeEditorID k_invalidEditorIDCapacity = 0;

		std::vector<bool> m_isAllocatedList = {};

		std::queue<TypeAlias::StorageID> m_freeNodeEditorIDQueue = {};

		Converter::NodeEditorAllocatorJsonConverter m_jsonConverter = {};

		TypeAlias::NodeEditorID m_nodeEditorIDCapacity = Constant::k_invalidNodeEditorID;
		TypeAlias::NodeEditorID m_nextNodeEditorID     = k_initialNextEditorID;
	};
}