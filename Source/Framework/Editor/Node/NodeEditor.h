#pragma once

namespace FWK::Editor
{
	class NodeEditor final
	{
	public:

		 NodeEditor();
		~NodeEditor();

		void INIT();

		bool Begin(const std::string_view& a_editorLabel, const ImVec2 a_canvasSize = Constant::k_defaultNodeEditorCanvasSize);

		void End();

		void Release();

		auto* GetMutablePTREditorContext() { return m_editorContext; }

	private:

		static constexpr std::string_view k_missingNodeEditorContextLabel = "ノードエディターコンテキストが作成されていません。";

		ax::NodeEditor::EditorContext* m_editorContext;
	};
}