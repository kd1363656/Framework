#pragma once

namespace FWK
{
    // FWK_ASSERT_RETURN系マクロの条件が成立した(=何かに失敗した)ことを報告するクラス
    // assert()はReleaseビルド(NDEBUG定義時)では消えてしまうため、
    // DebugでもReleaseでも必ず失敗内容を残すためにこのクラスを経由する
    // 報告先は以下の2つ
    // ・OutputDebugStringW : VSの出力ウィンドウやDebugViewで見られる
    // ・EditorのLogウィンドウ : エディター上で赤字で見られる
    class AssertReporter final : public Utility::SingletonBase<AssertReporter>
    {
    private:

        friend class SingletonBase<AssertReporter>;

         AssertReporter()          = default;
        ~AssertReporter() override = default;

    public:

        // 失敗を報告する
        // 同じ場所(ファイル名 + 行番号)の失敗は、アプリ起動中に1回だけ報告する
        // (毎フレーム呼ばれる処理で失敗しても、ログが毎フレーム増え続けないようにするため)
        void Report(const std::string_view& a_message, const std::source_location& a_location);

        // EditorのLogウィンドウへの報告を有効にするかどうか
        // EditorManagerの破棄が始まった後にLogウィンドウへ書き込むと
        // 破棄途中のオブジェクトを触ってしまうため、破棄前にfalseにする
        void SetIsEditorLogEnabled(const bool a_set) { m_isEditorLogEnabled = a_set; }

    private:

        void OutputDebugReport(const std::string& a_report) const;

        static constexpr TypeAlias::Math::Color k_editorLogColor = { 1.00F,
                                                                     0.35F,
                                                                     0.35F,
                                                                     1.00F };

        static constexpr UINT k_reportCodePage = CP_UTF8;

        static constexpr DWORD k_reportMultiByteFlags = 0UL;

        static constexpr int k_reportQueryBufferSize      = 0;
        static constexpr int k_reportInvalidConvertedSize = 0;

        std::unordered_set<std::string> m_reportedSiteSet = {};

        bool m_isEditorLogEnabled = true;

        // 報告処理の中でさらに失敗して、Report()同士が無限に呼び合うのを防ぐためのフラグ
        bool m_isReporting = false;
    };
}