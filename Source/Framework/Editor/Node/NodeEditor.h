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

		void EndDraw();

		nlohmann::json Serialize() const;

		bool FetchVALIsInitialized() const;

		const auto& GetREFNodeEditorAllocator() const { return m_nodeEditorAllocator; }

		auto& GetMutableREFNodeEditorAllocator() { return m_nodeEditorAllocator; }

	private:

		void Release();

		ImNodesEditorContext* m_editorContext;

		NodeEditorAllocator m_nodeEditorAllocator = {};

		Converter::NodeEditorJsonConverter m_jsonConverter = {};

		bool m_isDrawing;
	};
}