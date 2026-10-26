#include "D3D12DebugMessageLogger.h"

FWK::Graphics::D3D12DebugMessageLogger::~D3D12DebugMessageLogger()
{
    // 登録した関数を解除してから、ファイルを閉じる
    if (m_infoQueue &&
        m_callbackCookie != k_invalidCallbackCookie)
    {
        m_infoQueue->UnregisterMessageCallback(m_callbackCookie);
    }

    const std::scoped_lock l_lock(m_mutex);

    if (!m_ofstream.is_open()) { return; }

    m_ofstream << std::format("\n[Summary] WarningOrErrorMessageCount : {}\n", m_messageCount);
}

bool FWK::Graphics::D3D12DebugMessageLogger::Create(const Device& a_device)
{
    const auto& l_device = a_device.GetREFDevice();

    FWK_ASSERT_RETURN_VALUE_IF(!l_device, "デバイスが作成されておらず、D3D12DebugMessageLoggerの作成に失敗しました。", false);

    // InfoQueue1はデバッグレイヤーのメッセージを受け取るためのインターフェース
    // デバッグレイヤーが無効な場合や古いOSでは取得できないため、その場合は何もしない
    if (FAILED(l_device.As(&m_infoQueue))) { return false; }

    std::error_code l_errorCode = {};

    std::filesystem::create_directories(k_logFilePath.parent_path(), l_errorCode);

    m_ofstream.open(k_logFilePath);

    FWK_ASSERT_RETURN_VALUE_IF(!m_ofstream, "D3D12デバッグレイヤーのログファイルを開けませんでした。", false);

    m_ofstream << "[D3D12DebugLayerLog] Severity, ID, Description\n";

    // RegisterMessageCallback(メッセージが出たときに呼ぶ関数、
    //                         オプション(今回は特になし)、
    //                         関数へ渡す値(自分自身)、
    //                         登録解除に使う番号の受け取り先);
    const auto l_hr = m_infoQueue->RegisterMessageCallback(&OnD3D12Message,
                                                           D3D12_MESSAGE_CALLBACK_FLAG_NONE,
                                                           this,
                                                           &m_callbackCookie);

    FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "D3D12デバッグレイヤーのメッセージ受け取り関数の登録に失敗しました。", false);

    return true;
}

void CALLBACK FWK::Graphics::D3D12DebugMessageLogger::OnD3D12Message(D3D12_MESSAGE_CATEGORY,
                                                                      D3D12_MESSAGE_SEVERITY a_severity,
                                                                      D3D12_MESSAGE_ID       a_id,
                                                                      LPCSTR                 a_description,
                                                                      void*                  a_context)
{
    // 情報(INFO)やメッセージ(MESSAGE)は量が多いため、警告(WARNING)以上だけを記録する
    // 重要度の値は CORRUPTION(0) < ERROR(1) < WARNING(2) < INFO(3) < MESSAGE(4) の順に小さいほど深刻
    if (a_severity > D3D12_MESSAGE_SEVERITY_WARNING) { return; }

    auto* const l_logger = static_cast<D3D12DebugMessageLogger*>(a_context);

    if (!l_logger) { return; }

    l_logger->WriteMessage(a_severity, a_id, a_description);
}

void FWK::Graphics::D3D12DebugMessageLogger::WriteMessage(const D3D12_MESSAGE_SEVERITY a_severity, const D3D12_MESSAGE_ID a_id, const LPCSTR a_description)
{
    // デバッグレイヤーは複数のスレッドから呼ぶ可能性があるため、書き込みを1つずつに制限する
    const std::scoped_lock l_lock(m_mutex);

    ++m_messageCount;

    if (m_messageCount > k_maxWriteMessageCount) { return; }

    m_ofstream << std::format("{}, {}, {}\n",
                              static_cast<int>(a_severity),
                              static_cast<int>(a_id),
                              a_description ? a_description : "");

    m_ofstream.flush();
}