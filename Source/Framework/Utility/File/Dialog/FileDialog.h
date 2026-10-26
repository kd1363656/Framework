#pragma once

namespace FWK::Utility
{
    // Windowsのファイル選択ダイアログ(「開く」と「名前を付けて保存」)を表示するクラス
    //
    // ダイアログは別スレッドで表示する
    // 理由1 : ダイアログを表示している間もメインスレッドはWindowのメッセージ処理と描画を続けられるため
    //         (メインスレッドで表示すると、閉じるまでゲームループが止まってしまう)
    // 理由2 : ファイル選択ダイアログはCOMの「STA(シングルスレッドアパートメント)」で使う決まりがあるが、
    //         このアプリのメインスレッドはMTA(マルチスレッドアパートメント)で初期化しているため
    //         STAで初期化した別スレッドを用意する必要がある
    //
    // 使い方 : RequestOpen / RequestSaveでダイアログを表示し、
    //          毎フレームTryFetchResultFilePathを呼んで、ユーザーがファイルを選んだかを確認する
    class FileDialog final
    {
    public:

         FileDialog() = default;
        ~FileDialog() = default;

        bool RequestOpen(const Struct::FileDialogDesc& a_desc);
        bool RequestSave(const Struct::FileDialogDesc& a_desc);

        bool TryFetchResultFilePath(std::filesystem::path& a_outFilePath);

        bool IsShowing() const { return m_resultFilePathFuture.valid(); }

    private:

        bool Request(const CLSID& a_dialogCLSID, const FILEOPENDIALOGOPTIONS a_addDialogOptions, const Struct::FileDialogDesc& a_desc);

        static std::filesystem::path Show      (const CLSID& a_dialogCLSID, const FILEOPENDIALOGOPTIONS a_addDialogOptions, const Struct::FileDialogDesc& a_desc);
        static std::filesystem::path ShowDialog(const CLSID& a_dialogCLSID, const FILEOPENDIALOGOPTIONS a_addDialogOptions, const Struct::FileDialogDesc& a_desc);

        // ユーザーがキャンセルした場合は空のファイルパスが入る
        std::future<std::filesystem::path> m_resultFilePathFuture = {};
    };
}