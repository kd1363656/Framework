#pragma once

namespace FWK
{
	class InputComponent;
}

namespace FWK
{
	class InputComponentInspector final
	{
	private:

		enum class ContextMenuRequest
		{
			None,

			CreateNode,
			Node
		};

	public:

		 InputComponentInspector() = default;
		~InputComponentInspector() = default;

		void INIT();

		void Deserialize(const nlohmann::json& a_rootJson);

		void EditInspector(InputComponent& a_inputComponent);

		nlohmann::json Serialize() const;

		const auto& GetREFStartNode () const { return m_startNode; }
		const auto& GetREFNodeEditor() const { return m_nodeEditor; }

		auto& GetMutableREFStartNode () { return m_startNode; }
		auto& GetMutableREFNodeEditor() { return m_nodeEditor; }

	private:

		void DrawStartNode                 ();
		void DrawExecutionConditionNodeList(InputComponent& a_inputComponent);
		void DrawNodeEditorContextMenu     (InputComponent& a_inputComponent);
		void PrepareNodeEditorContextMenu  (const bool      a_isEditorHovered, InputComponent& a_inputComponent);
		void DrawCreateNodeContextMenu     (InputComponent& a_inputComponent);
		void DrawNodeContextMenu           (InputComponent& a_inputComponent);

		bool PrepareExecutionConditionNode(Struct::ObserverInputExecutionCondition<Enum::ComponentEvent>& a_executionCondition);

		bool AddComponentEventNode(const ImVec2& a_screeSpacePosition, const Enum::ComponentEvent a_componentEvent, InputComponent& a_inputComponent);

		void RemoveExecutionConditionNode(const TypeAlias::NodeEditorID a_nodeID, InputComponent& a_inputComponent);

		static constexpr std::string_view k_nodeEditorLabel = "##InputComponentNodeEditor";
		
		static constexpr std::string_view k_startNodeLabel               = "Start";
		static constexpr std::string_view k_inputLabel                   = "In";
		static constexpr std::string_view k_outputPinLabel               = "Next";
		static constexpr std::string_view k_eventLaneLabel               = "EventLane";
		static constexpr std::string_view k_eventLaneComboBoxLabel       = "##EventLane";
		static constexpr std::string_view k_expectedResultLabel          = "ExpectedObserverResult";
		static constexpr std::string_view k_addNodeContextMenuLabel      = "##InputComponentAddNodeContextMenu";
		static constexpr std::string_view k_nodeContextMenuLabel         = "##InputComponentNodeContextMenu";
		static constexpr std::string_view k_addNodeMenuLabel             = "追加";
		static constexpr std::string_view k_componentEventMenuLabel      = "ComponentEvent";
		static constexpr std::string_view k_deleteNodeMenuLabel          = "削除";
		static constexpr std::string_view k_noAddableComponentEventLabel = "追加可能なComponentEventはありません";
		static constexpr std::string_view k_startNodeDeleteDisabledLabel = "Startノードは削除できません";

		static constexpr float  k_minMapSizeFraction       = 0.20F;
		static constexpr ImVec2 k_startNodeInitialPosition = { 40.0F, 80.0F };
		
		Editor::NodeEditor     m_nodeEditor = {};
		Editor::NodeEditorNode m_startNode  = {};

		ContextMenuRequest m_contextMenuRequest = ContextMenuRequest::None;

		TypeAlias::NodeEditorID m_contextMenuTargetNodeID = Constant::k_invalidNodeEditorID;

		ImVec2 m_createNodeScreenSpacePosition = {};

		Converter::InputComponentInspectorJsonConverter m_jsonConverter = {};
	};
}