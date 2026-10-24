#pragma once

namespace FWK::Utility
{
    // ImGui::InputTextで編集する文字バッファ(固定長のchar配列)へ文字列をコピーする
    // 名前変更を始める時に「今の名前」を入力欄へ入れておくために使う
    // (AssetBrowserのファイル名、WorldOutlinerのGameObject名、Scene名で共通)
    //
    // BufferSize : バッファの長さ(std::arrayの要素数から自動で決まる)
    template <std::size_t BufferSize>
    inline void IMGUICopyStringToInputTextBuffer(const std::string_view& a_string, std::array<char, BufferSize>& a_inputTextBuffer)
    {
        // 前の文字が残らないようにバッファ全体を'\0'で埋める
        a_inputTextBuffer.fill(Constant::k_nullCharacter);

        // 最後の1文字分は終端の'\0'用に空けておき、入りきらない分は切り捨てる
        const auto l_copySize = std::min(a_string.size(), a_inputTextBuffer.size() - Constant::k_inputBufferLastSizeOffsetForCopy);

        std::copy_n(a_string.begin(), l_copySize, a_inputTextBuffer.begin());
    }
}