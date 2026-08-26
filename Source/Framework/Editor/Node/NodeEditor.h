#pragma once

namespace FWK::Editor
{
	class NodeEditor final
	{
	public:

		 NodeEditor();
		~NodeEditor();

		void INIT();

		void Release();

		auto* GetMutablePTREditorContext() { return m_editorContext; }

	private:

		ax::NodeEditor::EditorContext* m_editorContext;
	};
}