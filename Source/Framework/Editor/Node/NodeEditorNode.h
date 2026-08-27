#pragma once

namespace FWK::Editor
{
	class NodeEditorNode final
	{
	public:

		 NodeEditorNode() = default;
		~NodeEditorNode() = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		bool ApplyNodeID(NodeEditorAllocator& a_nodeEditorAllocator);

		nlohmann::json Serialize() const;

		void Release(NodeEditorAllocator& a_nodeEditorAllocator);

		void ReleaseInputPinIDList (NodeEditorAllocator& a_nodeEditorAllocator);
		void ReleaseOutputPinIDList(NodeEditorAllocator& a_nodeEditorAllocator);

		bool AddInputPinID(      NodeEditorAllocator&    a_nodeEditorAllocator);
		bool AddInputPinID(const TypeAlias::NodeEditorID a_pinID);

		bool AddOutputPinID(      NodeEditorAllocator&    a_nodeEditorAllocator);
		bool AddOutputPinID(const TypeAlias::NodeEditorID a_pinID);

		void SetNodeID(const TypeAlias::NodeEditorID a_set) { m_nodeID = a_set; }

		void SetNodePosition(const ImVec2& a_set) { m_nodePosition = a_set; }

		bool FetchVALIsCreated() const;
		
		auto GetVALNodeID() const { return m_nodeID; }

		const auto& GetREFInputPinIDList () const { return m_inputPInIDList; }
		const auto& GetREFOutputPinIDList() const { return m_outputPInIDList; }
		const auto& GetREFNodePosition   () const { return m_nodePosition; }

	private:

		bool AddPinIDToPinIDList(const TypeAlias::NodeEditorID a_pinID, std::vector<TypeAlias::NodeEditorID>& a_pinIDList) const;

		void Reset();

		std::vector<TypeAlias::NodeEditorID> m_inputPInIDList  = {};
		std::vector<TypeAlias::NodeEditorID> m_outputPInIDList = {};

		Converter::NodeEditorNodeJsonConverter m_jsonConverter = {};

		ImVec2 m_nodePosition = {};

		TypeAlias::NodeEditorID m_nodeID = Constant::k_invalidNodeEditorID;
	};
}