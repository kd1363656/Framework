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

		void PostDeserialize(InputComponent& a_inputComponent);

		void EditInspector(InputComponent& a_inputComponent);

		nlohmann::json Serialize() const;

		const auto& GetREFNodeEditor                 () const { return m_nodeEditor; }
		const auto& GetREFStartNodeEditorNode        () const { return m_startNodeEditorNode; }
		const auto& GetREFRootConditionNodeEditorNode() const { return m_rootConditionNodeEditorNode; }
		const auto& GetREFExecuteNodeEditorNode      () const { return m_executeNodeEditorNode; }

		auto& GetMutableREFNodeEditor                 () { return m_nodeEditor; }
		auto& GetMutableREFStartNodeEditorNode        () { return m_startNodeEditorNode; }
		auto& GetMutableREFRootConditionNodeEditorNode() { return m_rootConditionNodeEditorNode; }
		auto& GetMutableREFExecuteNodeEditorNode      () { return m_executeNodeEditorNode; }

	private:

		bool SynchronizeNodeGraph        (InputComponent& a_inputComponent);
		bool SynchronizeStartNode        ();
		bool SynchronizeConditionNode    ();
		bool SynchronizeExecuteNode      ();
		bool SynchronizeConditionNodeList(InputComponent& a_inputComponent);

		void SynchronizeLinkList        (const InputComponent& a_inputComponent);
		void SynchronizeNodePositionList(      InputComponent& a_inputComponent);

		void ApplyNodePositions               (const InputComponent& a_inputComponent);
		void ApplyPendingConditionNodePosition(      InputComponent& a_inputComponent);

		void DrawStartNode               ()                                 const;
		void DrawConditionNode           ()                                 const;
		void DrawConditionNodeList       (InputComponent& a_inputComponent) const;
		void DrawConditionNodeCreatePopup(InputComponent& a_inputComponent);
		void DrawExecuteNode             (InputComponent& a_inputComponent) const;

		void RequestConditionNodeCreatePopup();

		bool AddConditionNode(const Enum::ComponentEvent a_componentEvent, const ImVec2& a_nodeScreenPosition, InputComponent& a_inputComponent);

		bool FetchVALIsAllowedLink       (const InputComponent& a_inputComponent, const TypeAlias::NodeEditorID a_inputPinID, const TypeAlias::NodeEditorID a_outputPinID) const;
		bool FetchVALIsComponentEventUsed(const InputComponent& a_inputComponent, const Enum::ComponentEvent    a_componentEvent)                                          const;

		static constexpr std::string_view k_conditionNodeCreatePopupLabel                        = "Condition作成##ConditionNodeCreatePopup";
		static constexpr std::string_view k_conditionNodeCreateListBoxLabel                      = "##ConditionNodeCreateListBox";
		static constexpr std::string_view k_noUnusedComponentEventLabel                          = "追加できるComponentEventがありません。";
		static constexpr std::string_view k_addConditionButtonLabel                              = "Condition追加";
		static constexpr std::string_view k_startNodeLabel                                       = "Start";
		static constexpr std::string_view k_conditionNodeLabel                                   = "Condition";
		static constexpr std::string_view k_executeNodeLabel                                     = "Execute";
		static constexpr std::string_view k_outputPinLabel                                       = "Output";
		static constexpr std::string_view k_inputPinLabel                                        = "Input";
		static constexpr std::string_view k_notifyComponentEventLabel                            = "ComponentEvent";
		static constexpr std::string_view k_notifyEventLaneLabel                                 = "EventLane";
		static constexpr std::string_view k_notifyFlagLabel                                      = "NotifyFlag";
		static constexpr std::string_view k_componentEventNotifyStrategyRadioButtonSelectorLabel = "ComponentEventNotifyStrategyRadioButtonSelector";
		static constexpr std::string_view k_expectedObserverResultLabel                          = "ExpectedResult";

		static constexpr float k_initialRootConditionNodePositionX = 96.0F;
		static constexpr float k_initialRootConditionNodePositionY = -71.0F;

		static constexpr float k_initialChildConditionNodePositionX = 340.0F;
		static constexpr float k_initialChildConditionNodePositionY = 20.0F;
		static constexpr float k_childConditionNodeVerticalInterval = 50.0F;
		
		static constexpr float k_initialExecuteNodePositionX = 97.0F;
		static constexpr float k_initialExecuteNodePositionY = 47.0F;

		static constexpr std::size_t k_initialConditionIndex = 0ULL;
		static constexpr std::size_t k_initialLinkIndex      = 0ULL;
		static constexpr std::size_t k_primaryPinIndex       = 0ULL;
		static constexpr std::size_t k_primaryPinCount       = 1ULL;

		Editor::NodeEditor m_nodeEditor = {};

		Editor::NodeEditorNode m_startNodeEditorNode         = {};
		Editor::NodeEditorNode m_rootConditionNodeEditorNode = {};
		Editor::NodeEditorNode m_executeNodeEditorNode       = {};
		
		ImVec2 m_conditionNodeDropScreenPosition          = {};
		ImVec2 m_pendingConditionNodeCreateScreenPosition = {};

		TypeAlias::NodeEditorID m_pendingConditionNodePositionNodeID = Constant::k_invalidNodeEditorID;

		Converter::InputComponentInspectorJsonConverter m_jsonConverter = {};
	};
}