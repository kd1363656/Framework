#pragma once

namespace FWK::Graphics
{
    // 1フレームの中で「どの処理に何ミリ秒かかったか」をCPUとGPUの両方で計測し、
    // 平均・最小・最大をレポートファイルへ書き出すクラス
    //
    // 60FPSを保つには1フレームを16.67ミリ秒以内に収める必要がある。
    // CPUとGPUは並行して動くため、どちらか遅い方がフレームの速さを決める。
    // そのため両方を計測して、どちらがボトルネックかを判断できるようにする
    class FrameProfiler final
    {
    private:

        // 同じ名前の区間の計測結果をまとめた統計
        struct ProfileStatistics final
        {
            static constexpr double k_initialTotalMS = 0.0;

            static constexpr UINT64 k_initialSampleCount = 0ULL;

            std::string_view m_name = {};

            double m_totalMS = k_initialTotalMS;
            double m_minMS   = std::numeric_limits<double>::max   ();
            double m_maxMS   = std::numeric_limits<double>::lowest();

            UINT64 m_sampleCount = k_initialSampleCount;
        };

        // 計測中のCPU区間
        struct OpenCPUScope final
        {
            std::string_view m_name = {};

            std::chrono::steady_clock::time_point m_beginTime = {};
        };

    public:

         FrameProfiler() = default;
        ~FrameProfiler() = default;

        bool Create(const Device&                         a_device,
                    const TypeAlias::DirectCommandQueue&  a_directCommandQueue,
                    const TypeAlias::ComputeCommandQueue& a_computeCommandQueue,
                    const std::size_t                     a_frameResourceCount);

        void BeginFrame(const std::size_t a_frameIndex, const DirectCommandList& a_directCommandList, const ComputeCommandList& a_computeCommandList);

        void BeginGPUScope(const DirectCommandList& a_directCommandList, const std::string_view a_name);
        void EndGPUScope  (const DirectCommandList& a_directCommandList);

        void BeginCPUScope(const std::string_view a_name);
        void EndCPUScope  ();

        void EndFrame(const DirectCommandList& a_directCommandList, const ComputeCommandList& a_computeCommandList);

        // 処理時間ではない数値(描画した三角形数など)を1フレーム分記録する
        void AddCounterSample(const std::string_view a_name, const double a_value);

        void AddReportNote(const std::string& a_note);

        void WriteReport() const;

        // 計測区間の名前
        static constexpr std::string_view k_directQueueFrameScopeName  = "GPU DirectQueue : Frame Total";
        static constexpr std::string_view k_computeQueueFrameScopeName = "GPU ComputeQueue : Frame Total";

    private:

        void AddSample(const Struct::FrameProfileSample& a_sample, std::vector<ProfileStatistics>& a_statisticsList) const;

        void WriteStatisticsList(const std::vector<ProfileStatistics>& a_statisticsList, std::ofstream& a_ofstream) const;

        bool IsMeasuring() const;

        inline static const std::filesystem::path k_reportFilePath = "Log/FrameProfileReport.txt";

        static constexpr double k_targetFrameMS = 1000.0 / 60.0;

        // 起動直後はモデルの読み込みなどで処理時間が大きくぶれるため、
        // このフレーム数を過ぎてから集計を始める
        // (負荷が高く数FPSしか出ない状態でも計測できるように、少なめにしている)
        static constexpr UINT64 k_warmupFrameCount = 30ULL;

        static constexpr UINT64 k_initialFrameCount = 0ULL;

        std::vector<ProfileStatistics> m_gpuStatisticsList     = {};
        std::vector<ProfileStatistics> m_cpuStatisticsList     = {};
        std::vector<ProfileStatistics> m_counterStatisticsList = {};

        std::vector<Struct::FrameProfileSample> m_completedSampleList = {};

        std::vector<OpenCPUScope> m_openCPUScopeStack = {};

        std::vector<std::string> m_reportNoteList = {};

        GPUTimestampQueryRecorder m_directQueryRecorder  = {};
        GPUTimestampQueryRecorder m_computeQueryRecorder = {};

        UINT64 m_frameCount = k_initialFrameCount;

        bool m_isCreated = false;
    };
}