#pragma once

namespace FWK::Editor
{
	class NodeEditor final
	{
	public:

		 NodeEditor();
		~NodeEditor();

		void INIT();

		void Deserialize(const nlohmann::json& a_rootJson);

		bool BeginDraw();

		// 現在登録されているLinkを全てImNodesへ描画する
		void DrawLinkList() const;

		void EndDraw();

		nlohmann::json Serialize() const;

		bool ApplyNodePosition(const NodeEditorNode& a_nodeEditorNode);

		bool AddLink(const TypeAlias::NodeEditorID     a_inputPinID, TypeAlias::NodeEditorID a_outputPinID);
		bool AddLink(const Struct::NodeEditorLinkData& a_linkData);

		void RemoveLink(const TypeAlias::NodeEditorID a_linkID);

		void ClearLinkData();

		bool FetchVALIsInitialized() const;

		bool FetchVALIsInputPinLinked(const TypeAlias::NodeEditorID a_inputPinID)                                              const;
		bool FetchVALHasLink         (const TypeAlias::NodeEditorID a_inputPinID, const TypeAlias::NodeEditorID a_outputPinID) const;

		const auto& GetREFLinkDataList() const { return m_linkDataList; }

		const auto& GetREFNodeEditorAllocator() const { return m_nodeEditorAllocator; }

		auto& GetMutableREFNodeEditorAllocator() { return m_nodeEditorAllocator; }

	private:

		void Release();

		ImNodesEditorContext* m_editorContext;

		NodeEditorAllocator m_nodeEditorAllocator = {};

		std::vector<Struct::NodeEditorLinkData> m_linkDataList = {};

		Converter::NodeEditorJsonConverter m_jsonConverter = {};

		bool m_isDrawing;
	};
}