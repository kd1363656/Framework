#pragma once

namespace FWK::Editor
{
	class WorldOutlinerEditorWindowSelectionState;
}

namespace FWK::Editor
{
	class WorldOutlinerEditorWindowSelectGameObjectCommand final : public ICommand
	{
	public:

		 WorldOutlinerEditorWindowSelectGameObjectCommand(const std::vector<boost::uuids::uuid>& a_prevSelectedGameObjectUUIDList, const std::vector<boost::uuids::uuid> a_nextSelectedGameObjectUUIDList, const std::weak_ptr<WorldOutlinerEditorWindowSelectionState>& a_selectionState);
		~WorldOutlinerEditorWindowSelectGameObjectCommand() override;

		void Undo() override;
		void Redo() override;

	private:

		void ApplySelectedGameObjectUUIDListHistory(const std::vector<boost::uuids::uuid>& a_selectedGameObjectUUIDList);

		std::vector<boost::uuids::uuid> m_prevSelectedGameObjectUUIDList;
		std::vector<boost::uuids::uuid> m_nextSelectedGameObjectUUIDList;

		std::weak_ptr<WorldOutlinerEditorWindowSelectionState> m_selectionState;
	};
}