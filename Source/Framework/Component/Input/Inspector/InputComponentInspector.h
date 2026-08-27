#pragma once

namespace FWK
{
	class InputComponent;
}

namespace FWK
{
	class InputComponentInspector final
	{
	public:

		 InputComponentInspector() = default;
		~InputComponentInspector() = default;

		void INIT();

		void Deserialize(const nlohmann::json& a_rootJson);

		void EditInspector(InputComponent& a_inputComponent);

		nlohmann::json Serialize() const;

		const auto& GetREFNodeEditor() const { return m_nodeEditor; }

		auto& GetMutableREFNodeEditor() { return m_nodeEditor; }

	private:

		void DrawExecutionCheckBoxList     (InputComponent& a_inputComponent);
		void DrawStartNode                 ();
		void DrawExecutionConditionNodeList(InputComponent& a_inputComponent);

		bool PrepareExecutionConditionNode(Struct::ObserverInputExecutionCondition<Enum::ComponentEvent>& a_executionCondition);

		static constexpr std::string_view k_nodeEditorLabel                 = "##InputComponentNodeEditor";
		static constexpr std::string_view k_executionConditionCheckBoxLabel = "条件ノード使用是非チェックボックス";
		
		static constexpr std::string_view k_startNodeLabel         = "Start";
		static constexpr std::string_view k_inputLabel             = "In";
		static constexpr std::string_view k_outputPinLabel         = "Next";
		static constexpr std::string_view k_eventLaneLabel         = "EventLane";
		static constexpr std::string_view k_eventLaneComboBoxLabel = "##EventLane";
		static constexpr std::string_view k_expectedResultLabel    = "ExpectedObserverResult";

		static constexpr float k_executionConditionNodeSpacingX = 220.0F;

		Editor::NodeEditor     m_nodeEditor = {};
		Editor::NodeEditorNode m_startNode = {};

		Converter::InputComponentInspectorJsonConverter m_jsonConverter = {};
	};
}