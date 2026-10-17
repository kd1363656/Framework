#pragma once

namespace FWK::Concept
{
    // 名前変更の入力状態(アセットブラウザー・アウトライナー)が共通で持つメンバをそろえているか判別するコンセプト
    // 入力バッファ(char配列)と、名前変更中か・初回のフォーカスを当てたかのフラグを持っていればよい
    template <typename Type>
    concept IsEditorRenameStateConcept = requires(Type& a_renameState)
    {
        { a_renameState.m_inputBuffer.data() } -> std::same_as<char*>;
        { a_renameState.m_inputBuffer.size() } -> std::same_as<std::size_t>;
        { a_renameState.m_isActive }           -> std::same_as<bool&>;
        { a_renameState.m_isFocused }          -> std::same_as<bool&>;
    };
}