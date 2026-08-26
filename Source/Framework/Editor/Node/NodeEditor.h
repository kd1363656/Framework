#pragma once

namespace FWK::Editor
{
	class NodeEditor final
	{
	public:

		 NodeEditor();
		~NodeEditor();

		void INIT();

		bool PrepareDraw();

		void FinalizeDraw();

		bool FetchVALIsInitialized() const;

	private:

		void Release();

		ImNodesEditorContext* m_editorContext;

		bool m_isDrawing;
	};
}