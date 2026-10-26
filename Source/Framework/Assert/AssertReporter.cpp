#include "AssertReporter.h"

void FWK::AssertReporter::Report(const std::string_view& a_message, const std::source_location& a_location)
{
    // std::source_location::file_name()とline()で
    // 「どのファイルの何行目か」を1つの文字列にして、報告済みかどうかの目印にする
    // std::unordered_set::emplace()は、既に同じ値があると.secondがfalseになる
    // ※報告処理の中(Logウィンドウへの追加など)で同じ場所の失敗がもう一度起きても、
    //   ここで登録済みになっているため2回目は必ずここで終わる(無限に呼び合わない)
    if (const auto l_insertResult = m_reportedSiteSet.emplace(std::format("{}:{}", a_location.file_name(), a_location.line()));
        !l_insertResult.second)
    {
        return;
    }

    // 例 : [ASSERT][C:\...\UploadSystem.cpp : 45][関数名]
    //      TextureRecordが無効になっているため、このテクスチャのコピーをスキップしました。
    const auto& l_report = std::format("[ASSERT][{} : {}][{}]\n{}\n",
                                       a_location.file_name(),
                                       a_location.line(),
                                       a_location.function_name(),
                                       a_message);

    OutputDebugReport(l_report);

    // FWK_ADD_LOGマクロを使うと、ログの発生場所が「この関数」になってしまうため
    // AddLog()へ失敗した場所(a_location)をそのまま渡す
    if (m_isEditorLogEnabled)
    {
        auto& l_editorManager = Editor::EditorManager::GetInstance();
        
        l_editorManager.AddLog(a_location,
                               k_editorLogColor,
                               "[ASSERT]\n{}",
                               a_message);
    }
}

void FWK::AssertReporter::OutputDebugReport(const std::string& a_report) const
{
    if (a_report.empty()) { return; }

    // MultiByteToWideChar
    // (
    //      変換元文字列の文字コード(今回はUTF-8)、
    //      変換フラグ、
    //      変換元文字列の先頭ポインタ、
    //      変換元文字列のバイト数、
    //      変換先の先頭アドレス(nullptrなら必要サイズの問い合わせだけを行う)、
    //      変換先のサイズ(0なら必要サイズの問い合わせだけを行う)
    // )
    // OutputDebugStringA()はShift-JISとして解釈するため、
    // UTF-8の日本語を渡すと文字化けする。UTF-16へ変換してからW版で出力する
    // ※ここでFWK_ASSERT_RETURN系マクロや、それを使うUtility::StringToWString()を使うと
    //   失敗したときに報告の中から報告を呼ぶことになるため、直接書いている
    const int l_size = MultiByteToWideChar(k_reportCodePage,
                                           k_reportMultiByteFlags,
                                           a_report.data(),
                                           static_cast<int>(a_report.size()),
                                           nullptr,
                                           k_reportQueryBufferSize);

    if (l_size == k_reportInvalidConvertedSize) { return; }

    std::wstring l_wideReport(static_cast<std::size_t>(l_size), L'\0');

    MultiByteToWideChar(k_reportCodePage,
                        k_reportMultiByteFlags,
                        a_report.data(),
                        static_cast<int>(a_report.size()),
                        l_wideReport.data(),
                        l_size);

    // デバッガー(VSの出力ウィンドウ)やDebugViewへ文字列を送るWindowsの関数
    // デバッガーが繋がっていなければ何も起きないので、Releaseで呼んでも安全
    OutputDebugStringW(l_wideReport.c_str());
}