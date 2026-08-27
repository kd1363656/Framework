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

		const auto& GetREFNodeEditor       () const { return m_nodeEditor; }
		const auto& GetREFStartNodeEditor  () const { return m_startNodeEditor; }
		const auto& GetREFExecuteNodeEditor() const { return m_executeNodeEditor; }

		auto& GetMutableREFNodeEditor       () { return m_nodeEditor; }
		auto& GetMutableREFStartNodeEditor  () { return m_startNodeEditor; }
		auto& GetMutableREFExecuteNodeEditor() { return m_executeNodeEditor; }

	private:

		bool SynchronizeNodeGraph        (InputComponent& a_inputComponent);
		bool SynchronizeStartNode        ();
		bool SynchronizeExecuteNode      ();
		bool SynchronizeConditionNodeList(      InputComponent& a_inputComponent);
		void SynchronizeLinkList         (const InputComponent& a_inputComponent);

		void ApplyDefaultNodePositions(const InputComponent& a_inputComponent);

		void DrawStartNode        ()                                 const;
		void DrawConditionNodeList(InputComponent& a_inputComponent) const;
		void DrawExecuteNode      ()                                 const;

		void ProcessLinkCreation   (const InputComponent& a_inputComponent);
		void ProcessLinkDestruction();

		bool FetchVALIsAllowedInputPin(const InputComponent& a_inputComponent, const TypeAlias::NodeEditorID a_inputPinID) const;

		static constexpr std::string_view k_addConditionButtonLabel = "Condition追加";
		static constexpr std::string_view k_startNodeLabel          = "Start";
		static constexpr std::string_view k_conditionNodeLabel      = "Condition";
		static constexpr std::string_view k_executeNodeLabel        = "Execute";
		static constexpr std::string_view k_outputPinLabel          = "Output";
		static constexpr std::string_view k_inputPinLabel           = "Input";

		static constexpr float k_startNodePositionX            = 0.0F;
		static constexpr float k_startNodePositionY            = 160.0F;
		static constexpr float k_conditionNodePositionX        = 320.0F;
		static constexpr float k_conditionNodeFirstPositionY   = 20.0F;
		static constexpr float k_conditionNodeVerticalInterval = 150.0F;
		static constexpr float k_executeNodePositionX          = 640.0F;
		static constexpr float k_executeNodePositionY          = 160.0F;

		static constexpr std::size_t k_primaryPinIndex = 0ULL;
		static constexpr std::size_t k_primaryPinIndex = 1ULL;

		Editor::NodeEditor m_nodeEditor = {};

		Editor::NodeEditorNode m_startNodeEditor   = {};
		Editor::NodeEditorNode m_executeNodeEditor = {};

		Converter::InputComponentInspectorJsonConverter m_jsonConverter = {};

		// Nodeを毎フレーム初期位置へ戻してしまうと
		// ドラッグできなくなるため、一度だけ配置する。
		bool m_isDefaultNodePositionApplied = false;
	};
}