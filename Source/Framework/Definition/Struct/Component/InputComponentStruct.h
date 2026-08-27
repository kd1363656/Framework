#pragma once

namespace FWK::Struct
{
	template <typename Type>
		requires std::is_enum_v<Type>
	struct ObserverInputExecutionCondition final
	{
		Type m_receiveComponentEvent = Type::Invalid;

		Editor::NodeEditorNode m_editorNodeEditor = {};

		// 入力チェックをするオブザーバーの通知の種類は一つのみ取り扱いたいからEnumで管理
		Enum::EventLane m_checkEventLane = Enum::EventLane::Invalid;

		bool m_expectedObserverResult = false;
	};

	struct InputComponentExecution final
	{
		Editor::NodeEditorNode m_editorNodeEditor = {};

		Enum::ComponentEvent m_notifyComponentEvent = Enum::ComponentEvent::Invalid;

		Enum::EventLane m_notifyEventLane = Enum::EventLane::Invalid;

		bool m_notifyResult = false;
	};
}