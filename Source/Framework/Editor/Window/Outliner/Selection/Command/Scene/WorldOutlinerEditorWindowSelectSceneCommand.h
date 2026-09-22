#pragma once

namespace FWK::Editor
{
	class WorldOutlinerEditorWindowSelectionState;
}

namespace FWK::Editor
{
	class WorldOutlinerEditorWindowSelectSceneCommand final : public ICommand
	{
	public:

		 WorldOutlinerEditorWindowSelectSceneCommand(const std::weak_ptr<WorldOutlinerEditorWindowSelectionState>& a_selectionState);
		~WorldOutlinerEditorWindowSelectSceneCommand() override;

		void Undo() override;
		void Redo() override;

	private:

		std::weak_ptr<WorldOutlinerEditorWindowSelectionState> m_selectionState;
	};
}