#include "GPUTimestampQueryRecorder.h"

bool FWK::Graphics::GPUTimestampQueryRecorder::Create(const Device& a_device, ID3D12CommandQueue& a_commandQueue, const std::size_t a_frameRecordCount)
{
    const auto& l_device = a_device.GetREFDevice();

    FWK_ASSERT_RETURN_VALUE_IF(!l_device,                                  "デバイスが作成されておらず、GPUTimestampQueryRecorderの作成に失敗しました。",      false);
    FWK_ASSERT_RETURN_VALUE_IF(a_frameRecordCount == k_initialFrameIndex, "フレームリソースの数が0のため、GPUTimestampQueryRecorderの作成に失敗しました。", false);

    // GPUの時刻は「1秒間に何回カウントが進むか(周波数)」で表される。
    // 時刻の差 ÷ 周波数 = 経過秒数 になるため、最初に周波数を取得しておく
    // GetTimestampFrequency(周波数の書き込み先);
    auto l_hr = a_commandQueue.GetTimestampFrequency(&m_timestampFrequency);

    FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr) || m_timestampFrequency == k_invalidTimestampFrequency, "タイムスタンプ周波数の取得に失敗しました。", false);

    // QueryHeapは「GPUが時刻などの計測結果を書き込む専用の入れ物」
    // D3D12_QUERY_HEAP_DESCについての説明
    // Type     : 何を計測するか(TIMESTAMPなのでGPUの時刻)
    // Count    : 入れ物の数(フレームリソースの数 × 1フレームの最大数)
    // NodeMask : 使用するGPU(GPUが1つなら0)
    D3D12_QUERY_HEAP_DESC l_queryHeapDesc = {};

    l_queryHeapDesc.Type     = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    l_queryHeapDesc.Count    = static_cast<UINT>(a_frameRecordCount) * k_maxQueryCountPerFrame;
    l_queryHeapDesc.NodeMask = k_queryHeapNodeMask;

    // CreateQueryHeap(QueryHeapの設定、
    //                 受け取りたいCOMインターフェース型のID、
    //                 作成結果のポインタを書き込むアドレス);
    l_hr = l_device->CreateQueryHeap(&l_queryHeapDesc, IID_PPV_ARGS(m_queryHeap.ReleaseAndGetAddressOf()));

    FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "タイムスタンプ用QueryHeapの作成に失敗しました。", false);

    m_frameRecordList.clear ();
    m_frameRecordList.resize(a_frameRecordCount);

    for (auto& l_frameRecord : m_frameRecordList)
    {
        FWK_ASSERT_RETURN_VALUE_IF(!CreateReadbackBuffer(a_device, l_frameRecord), "タイムスタンプ読み戻し用バッファの作成に失敗しました。", false);
    }

    return true;
}

void FWK::Graphics::GPUTimestampQueryRecorder::BeginFrame(const std::size_t a_frameIndex, std::vector<Struct::FrameProfileSample>& a_completedSampleList)
{
    FWK_ASSERT_RETURN_IF(a_frameIndex >= m_frameRecordList.size(), "フレームインデックスが範囲外のため、GPUタイムスタンプの計測開始に失敗しました。");

    m_currentFrameIndex = a_frameIndex;

    auto& l_frameRecord = m_frameRecordList[m_currentFrameIndex];

    // このフレームリソースを前回使ったときの計測結果は、
    // CommandAllocatorの再利用待ち(GPU完了待ち)が済んだ今なら安全に読める
    if (l_frameRecord.m_isResolved)
    {
        CollectCompletedSampleList(l_frameRecord, a_completedSampleList);
    }

    // 今回のフレームで使うために記録をリセットする
    l_frameRecord.m_scopeRecordList.clear();

    l_frameRecord.m_usedQueryCount = k_initialUsedQueryCount;
    l_frameRecord.m_isResolved     = false;

    m_openScopeIndexStack.clear();
}

void FWK::Graphics::GPUTimestampQueryRecorder::BeginScope(ID3D12GraphicsCommandList6& a_commandList, const std::string_view a_name)
{
    auto& l_frameRecord = m_frameRecordList[m_currentFrameIndex];

    // Begin、Endの2つ分の空きがなければ記録しない
    if (l_frameRecord.m_usedQueryCount + k_scopeQueryCount > k_maxQueryCountPerFrame) { return; }

    ScopeRecord l_scopeRecord = {};

    l_scopeRecord.m_name            = a_name;
    l_scopeRecord.m_beginQueryIndex = WriteTimestamp(a_commandList);

    // Endが来たときにどの区間を閉じればよいか分かるように、
    // 開いている区間の番号を積んでおく(区間の入れ子に対応するため)
    m_openScopeIndexStack.emplace_back(l_frameRecord.m_scopeRecordList.size());

    l_frameRecord.m_scopeRecordList.emplace_back(l_scopeRecord);
}
void FWK::Graphics::GPUTimestampQueryRecorder::EndScope(ID3D12GraphicsCommandList6& a_commandList)
{
    // BeginScopeで記録できなかった場合は閉じる区間がない
    if (m_openScopeIndexStack.empty()) { return; }

    auto& l_frameRecord = m_frameRecordList[m_currentFrameIndex];

    const auto l_scopeIndex = m_openScopeIndexStack.back();

    m_openScopeIndexStack.pop_back();

    l_frameRecord.m_scopeRecordList[l_scopeIndex].m_endQueryIndex = WriteTimestamp(a_commandList);
}

void FWK::Graphics::GPUTimestampQueryRecorder::Resolve(ID3D12GraphicsCommandList6& a_commandList)
{
    auto& l_frameRecord = m_frameRecordList[m_currentFrameIndex];

    if (l_frameRecord.m_usedQueryCount == k_initialUsedQueryCount) { return; }

    FWK_ASSERT_RETURN_IF(!m_queryHeap,                    "QueryHeapが作成されておらず、タイムスタンプの読み戻し命令に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_frameRecord.m_readbackBuffer, "読み戻し用バッファが作成されておらず、タイムスタンプの読み戻し命令に失敗しました。");

    const auto l_startQueryIndex = static_cast<UINT>(m_currentFrameIndex) * k_maxQueryCountPerFrame;

    // QueryHeapはCPUから直接読めないため、CPUが読めるReadbackBufferへコピーする命令を積む
    // ResolveQueryData(コピー元のQueryHeap、
    //                  Queryの種類、
    //                  コピーを始めるQueryの番号、
    //                  コピーするQueryの数、
    //                  コピー先のバッファ、
    //                  コピー先バッファの書き込み開始位置(バイト));
    a_commandList.ResolveQueryData(m_queryHeap.Get(),
                                   D3D12_QUERY_TYPE_TIMESTAMP,
                                   l_startQueryIndex,
                                   l_frameRecord.m_usedQueryCount,
                                   l_frameRecord.m_readbackBuffer.Get(),
                                   k_readbackBufferStartOffset);

    l_frameRecord.m_isResolved = true;
}

bool FWK::Graphics::GPUTimestampQueryRecorder::CreateReadbackBuffer(const Device& a_device, FrameRecord& a_frameRecord) const
{
    const auto& l_device = a_device.GetREFDevice();

    FWK_ASSERT_RETURN_VALUE_IF(!l_device, "デバイスが作成されておらず、読み戻し用バッファの作成に失敗しました。", false);

    // READBACKヒープは「GPUが書き込んだ結果をCPUが読む」ためのメモリ
    // UPLOADヒープ(CPU→GPU)とは逆向きのデータの流れになる
    const auto l_heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);

    // タイムスタンプ1つはUINT64(8バイト)なので、1フレームの最大数分のサイズを確保する
    const auto l_resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT64) * k_maxQueryCountPerFrame);

    // READBACKヒープのリソースは作成時からCOPY_DEST(コピー先)状態にする決まりがある
    // CreateCommittedResource(ヒープの設定、
    //                         ヒープの追加設定、
    //                         リソースの設定、
    //                         初期状態、
    //                         クリア値(バッファなので不要)、
    //                         受け取りたいCOMインターフェース型のID、
    //                         作成結果のポインタを書き込むアドレス);
    const auto l_hr = l_device->CreateCommittedResource(&l_heapProperties,
                                                        D3D12_HEAP_FLAG_NONE,
                                                        &l_resourceDesc,
                                                        D3D12_RESOURCE_STATE_COPY_DEST,
                                                        nullptr,
                                                        IID_PPV_ARGS(a_frameRecord.m_readbackBuffer.ReleaseAndGetAddressOf()));

    FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "読み戻し用バッファの作成に失敗しました。", false);

    return true;
}

void FWK::Graphics::GPUTimestampQueryRecorder::CollectCompletedSampleList(const FrameRecord& a_frameRecord, std::vector<Struct::FrameProfileSample>& a_completedSampleList) const
{
    FWK_ASSERT_RETURN_IF(!a_frameRecord.m_readbackBuffer, "読み戻し用バッファが作成されておらず、GPU計測結果の取得に失敗しました。");

    // CPUが読む範囲(0バイト目から、使ったタイムスタンプの数分)
    const D3D12_RANGE l_readRange =
    {
        k_readbackBufferStartOffset,
        sizeof(UINT64) * a_frameRecord.m_usedQueryCount
    };

    void* l_mappedData = nullptr;

    // Map(サブリソース番号、CPUが読む範囲、CPUから見たアドレスの受け取り先);
    const auto l_hr = a_frameRecord.m_readbackBuffer->Map(Constant::k_firstSubresourceIndex, &l_readRange, &l_mappedData);

    FWK_ASSERT_RETURN_IF(FAILED(l_hr), "読み戻し用バッファのMapに失敗しました。");

    const auto* const l_timestampList = static_cast<const UINT64*>(l_mappedData);

    for (const auto& l_scopeRecord : a_frameRecord.m_scopeRecordList)
    {
        // Endが記録されていない(閉じ忘れの)区間は計算できないので飛ばす
        if (l_scopeRecord.m_endQueryIndex == k_invalidQueryIndex) { continue; }

        const auto l_beginTimestamp = l_timestampList[l_scopeRecord.m_beginQueryIndex];
        const auto l_endTimestamp   = l_timestampList[l_scopeRecord.m_endQueryIndex];

        // GPUの都合で時刻が逆転して見える場合は不正な値なので飛ばす
        if (l_endTimestamp < l_beginTimestamp) { continue; }

        Struct::FrameProfileSample l_sample = {};

        // (終了時刻 - 開始時刻) ÷ 周波数 = 経過秒数、それを1000倍してミリ秒にする
        l_sample.m_name      = l_scopeRecord.m_name;
        l_sample.m_elapsedMS = static_cast<double>(l_endTimestamp - l_beginTimestamp) * k_millisecondsPerSecond / static_cast<double>(m_timestampFrequency);

        a_completedSampleList.emplace_back(l_sample);
    }

    // CPUは読んだだけで書き込んでいないので、書き込み範囲は空にしてUnmapする
    const D3D12_RANGE l_writtenRange =
    {
        k_readbackBufferStartOffset,
        k_readbackBufferStartOffset
    };

    a_frameRecord.m_readbackBuffer->Unmap(Constant::k_firstSubresourceIndex, &l_writtenRange);
}

UINT FWK::Graphics::GPUTimestampQueryRecorder::WriteTimestamp(ID3D12GraphicsCommandList6& a_commandList)
{
    auto& l_frameRecord = m_frameRecordList[m_currentFrameIndex];

    // QueryHeap全体の中での番号(フレームリソースごとに使う範囲を分けている)
    const auto l_queryIndex = static_cast<UINT>(m_currentFrameIndex) * k_maxQueryCountPerFrame + l_frameRecord.m_usedQueryCount;

    // GPUがこの命令に到達した時刻をQueryHeapへ書き込ませる
    // タイムスタンプはBeginQueryを使わず、EndQueryだけで記録する決まりになっている
    // EndQuery(書き込み先のQueryHeap、
    //          Queryの種類、
    //          書き込み先の番号);
    a_commandList.EndQuery(m_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, l_queryIndex);

    ++l_frameRecord.m_usedQueryCount;

    // ReadbackBufferへは「このフレームで使った範囲の先頭」から詰めてコピーするため、
    // フレーム内での番号を返す
    return l_frameRecord.m_usedQueryCount - k_singleQueryCount;
}