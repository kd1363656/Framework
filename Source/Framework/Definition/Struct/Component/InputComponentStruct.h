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
        Enum::EventLaneBitShiftFlag m_checkEventLane = Enum::EventLaneBitShiftFlag::Invalid;

        bool m_expectedObserverResult = false;
    };

    struct InputComponentExecution final
    {
        Enum::ComponentEvent m_notifyComponentEvent = Enum::ComponentEvent::Invalid;

        Enum::EventLaneBitShiftFlag m_notifyEventLaneBitShiftFlag = Enum::EventLaneBitShiftFlag::Invalid;

        bool m_notifyFlag = false;
    };
}