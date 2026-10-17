#pragma once

namespace FWK::Utility
{
    // 名前変更を始める
    // アセットブラウザー(ファイル名)とアウトライナー(シーン名・GameObject名)で
    // 同じ「バッファを空にする → 今の名前を入れる → フラグを立てる」を5か所で書いていたため1つにまとめた
    // 名前変更の対象(ファイルパスやGameObject)は種類ごとに違うので、呼び出し側で設定する
    template <typename Type>
        requires Concept::IsEditorRenameStateConcept<Type>
    inline void StartEditorRename(const std::string_view& a_name, Type& a_renameState)
    {
        // 前に入力した文字が後ろに残らないように、先にバッファ全体を終端文字で埋める
        a_renameState.m_inputBuffer.fill(Constant::k_nullCharacter);

        // ImGui::InputTextは終端文字までを文字列として読むため、最後の1文字は必ず終端文字のまま残す
        // 例 : バッファ256文字、名前300文字 → 255文字だけコピーし、256文字目は'\0'
        const auto& l_copySize = std::min(a_name.size(), a_renameState.m_inputBuffer.size() - Constant::k_inputBufferLastSizeOffsetForCopy);

        std::copy_n(a_name.begin(), l_copySize, a_renameState.m_inputBuffer.begin());

        // 名前変更を始めた直後はまだInputTextにフォーカスを当てていないので、次の描画で1回だけ当てる
        a_renameState.m_isActive  = true;
        a_renameState.m_isFocused = false;
    }
}