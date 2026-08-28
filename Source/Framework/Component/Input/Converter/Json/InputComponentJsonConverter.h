#pragma once

namespace FWK
{
	class InputComponent;
}

namespace FWK::Converter
{
	class InputComponentJsonConverter final
	{
	public:

		 InputComponentJsonConverter() = default;
		~InputComponentJsonConverter() = default;

		void DeserializePrefab(const nlohmann::json& a_rootJson, InputComponent& a_inputComponent) const;
		
		nlohmann::json SerializePrefab(const InputComponent& a_inputComponent) const;

	private:

		void DeserializeNotifyComponentEventExecutionCondition(const nlohmann::json& a_rootJson, InputComponent& a_inputComponent) const;
		void DeserializeExecution                             (const nlohmann::json& a_rootJson, InputComponent& a_inputComponent) const;

		nlohmann::json SerializeNotifyComponentEventExecutionCondition(const InputComponent& a_inputComponent) const;
		nlohmann::json SerializeExecution                             (const InputComponent& a_inputComponent) const;

		static constexpr std::string_view k_notifyComponentEventExecutionConditionListJsonKey = "NotifyComponentEventExecutionConditionList";
		static constexpr std::string_view k_receiveComponentEventJsonKey                      = "ReceiveComponentEvent";
		static constexpr std::string_view k_nodeEditorNodeEventJsonKey                        = "NodeEditorNode";
		static constexpr std::string_view k_checkEventLaneJsonKey                             = "CheckEventLane";
		static constexpr std::string_view k_expectedObserverResultJsonKey                     = "ExpectedObserverResult";
		static constexpr std::string_view k_executionJsonKey                                  = "Execution";
		static constexpr std::string_view k_notifyComponentEventJsonKey                       = "NotifyComponentEvent";
		static constexpr std::string_view k_notifyEventLaneJsonKey                            = "NotifyEventLane";
		static constexpr std::string_view k_notifyResultJsonKey                               = "NotifyResult";
		static constexpr std::string_view k_inspectorJsonKey                                  = "Inspector";
	};
}