#pragma once

namespace FWK::Graphics
{
    // D3D12のデバッグレイヤーが出す警告・エラーをファイルへ書き出すクラス(Debugビルド専用)
    //
    // デバッグレイヤーは「リソースの状態遷移が間違っている」「範囲外を読んでいる」などの誤りを
    // 実行中に教えてくれるが、通常はVisual Studioの出力ウィンドウにしか表示されない。
    // 自動テストでも誤りを確認できるように、警告以上のメッセージをファイルへ記録する
    class D3D12DebugMessageLogger final
    {
    public:

         D3D12DebugMessageLogger() = default;
        ~D3D12DebugMessageLogger();

        D3D12DebugMessageLogger(const D3D12DebugMessageLogger&) = delete;
        D3D12DebugMessageLogger(      D3D12DebugMessageLogger&&) = delete;

        D3D12DebugMessageLogger& operator=(const D3D12DebugMessageLogger&) = delete;
        D3D12DebugMessageLogger& operator=(      D3D12DebugMessageLogger&&) = delete;

        bool Create(const Device& a_device);

    private:

        // デバッグレイヤーがメッセージを出すたびに呼ばれる関数
        // D3D12へは関数ポインタで渡すため、メンバー関数ではなくstatic関数にして、
        // a_contextに自分自身(this)を受け取る
        static void CALLBACK OnD3D12Message(D3D12_MESSAGE_CATEGORY a_category,
                                            D3D12_MESSAGE_SEVERITY a_severity,
                                            D3D12_MESSAGE_ID       a_id,
                                            LPCSTR                 a_description,
                                            void*                  a_context);

        void WriteMessage(const D3D12_MESSAGE_SEVERITY a_severity, const D3D12_MESSAGE_ID a_id, const LPCSTR a_description);

        inline static const std::filesystem::path k_logFilePath = "Log/D3D12DebugLayerLog.txt";

        static constexpr UINT64 k_maxWriteMessageCount = 1000ULL;
        static constexpr UINT64 k_initialMessageCount  = 0ULL;

        static constexpr DWORD k_invalidCallbackCookie = 0UL;

        std::mutex m_mutex = {};

        std::ofstream m_ofstream = {};

        TypeAlias::ComPtr<ID3D12InfoQueue1> m_infoQueue = nullptr;

        UINT64 m_messageCount = k_initialMessageCount;

        DWORD m_callbackCookie = k_invalidCallbackCookie;
    };
}