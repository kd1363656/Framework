#pragma once

namespace FWK::Graphics
{
    // GPUがコマンドを実行した瞬間の「時刻(タイムスタンプ)」を記録し、
    // 「Begin～Endの区間」でGPUが何ミリ秒かかったかを求めるクラス
    //
    // 仕組み :
    // 1. CommandListへ「この位置に来たら時刻を書き込んで」という命令(EndQuery)を積む
    // 2. GPUが実行した時刻はQueryHeap(時刻を書き込む専用の入れ物)へ書き込まれる
    // 3. ResolveQueryDataでQueryHeapの中身をCPUが読めるReadbackBufferへコピーする
    // 4. 数フレーム後、GPUがそのフレームを実行し終えてからCPUがReadbackBufferを読む
    //
    // CommandQueue(Direct又はCompute)ごとに時刻の進み方(周波数)が違うため、
    // CommandQueue1つにつき1つ作成する
    class GPUTimestampQueryRecorder final
    {
    private:

        // 「Begin～End」の区間1つ分の記録
        struct ScopeRecord final
        {
            static constexpr UINT k_invalidQueryIndex = UINT_MAX;

            std::string_view m_name = {};

            UINT m_beginQueryIndex = k_invalidQueryIndex;
            UINT m_endQueryIndex   = k_invalidQueryIndex;
        };

        // フレームリソース1つ分の記録
        // GPUは数フレーム遅れて実行するため、フレームリソースの数だけ記録を分けて持つ
        struct FrameRecord final
        {
            static constexpr UINT k_initialUsedQueryCount = 0U;

            std::vector<ScopeRecord> m_scopeRecordList = {};

            TypeAlias::ComPtr<ID3D12Resource2> m_readbackBuffer = nullptr;

            UINT m_usedQueryCount = k_initialUsedQueryCount;

            bool m_isResolved = false;
        };

    public:

         GPUTimestampQueryRecorder() = default;
        ~GPUTimestampQueryRecorder() = default;

        bool Create(const Device& a_device, ID3D12CommandQueue& a_commandQueue, const std::size_t a_frameRecordCount);

        void BeginFrame(const std::size_t a_frameIndex, std::vector<Struct::FrameProfileSample>& a_completedSampleList);

        void BeginScope(ID3D12GraphicsCommandList6& a_commandList, const std::string_view a_name);
        void EndScope  (ID3D12GraphicsCommandList6& a_commandList);

        void Resolve(ID3D12GraphicsCommandList6& a_commandList);

    private:

        bool CreateReadbackBuffer(const Device& a_device, FrameRecord& a_frameRecord) const;

        void CollectCompletedSampleList(const FrameRecord& a_frameRecord, std::vector<Struct::FrameProfileSample>& a_completedSampleList) const;

        UINT WriteTimestamp(ID3D12GraphicsCommandList6& a_commandList);

        static constexpr double k_millisecondsPerSecond = 1000.0;

        static constexpr UINT64 k_invalidTimestampFrequency = 0ULL;

        // 1フレームで記録できるタイムスタンプの最大数(区間1つにつきBegin、Endの2つ使う)
        static constexpr UINT k_maxQueryCountPerFrame = 512U;

        // 区間1つで使うタイムスタンプの数(Begin、End)
        static constexpr UINT k_scopeQueryCount   = 2U;
        static constexpr UINT k_singleQueryCount  = 1U;
        static constexpr UINT k_queryHeapNodeMask = 0U;

        static constexpr std::size_t k_initialFrameIndex         = 0ULL;
        static constexpr std::size_t k_readbackBufferStartOffset = 0ULL;

        std::vector<FrameRecord> m_frameRecordList     = {};
        std::vector<std::size_t> m_openScopeIndexStack = {};

        TypeAlias::ComPtr<ID3D12QueryHeap> m_queryHeap = nullptr;

        std::size_t m_currentFrameIndex = k_initialFrameIndex;

        UINT64 m_timestampFrequency = k_invalidTimestampFrequency;
    };
}