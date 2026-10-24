#include "FrameProfiler.h"

bool FWK::Graphics::FrameProfiler::Create(const Device&                         a_device,
                                          const TypeAlias::DirectCommandQueue&  a_directCommandQueue,
                                          const TypeAlias::ComputeCommandQueue& a_computeCommandQueue,
                                          const std::size_t                     a_frameResourceCount)
{
    const auto& l_directCommandQueue  = a_directCommandQueue.GetREFCommandQueue ();
    const auto& l_computeCommandQueue = a_computeCommandQueue.GetREFCommandQueue();

    FWK_ASSERT_RETURN_VALUE_IF(!l_directCommandQueue,  "DirectCommandQueueが作成されておらず、FrameProfilerの作成に失敗しました。",  false);
    FWK_ASSERT_RETURN_VALUE_IF(!l_computeCommandQueue, "ComputeCommandQueueが作成されておらず、FrameProfilerの作成に失敗しました。", false);

    // DirectQueueとComputeQueueは時刻の進み方が異なる場合があるため、別々に記録する
    FWK_ASSERT_RETURN_VALUE_IF(!m_directQueryRecorder.Create(a_device, *l_directCommandQueue.Get(), a_frameResourceCount),   "DirectQueue用タイムスタンプ記録クラスの作成に失敗しました。",  false);
    FWK_ASSERT_RETURN_VALUE_IF(!m_computeQueryRecorder.Create(a_device, *l_computeCommandQueue.Get(), a_frameResourceCount), "ComputeQueue用タイムスタンプ記録クラスの作成に失敗しました。", false);

    m_isCreated = true;

    return true;
}

void FWK::Graphics::FrameProfiler::BeginFrame(const std::size_t a_frameIndex, const DirectCommandList& a_directCommandList, const ComputeCommandList& a_computeCommandList)
{
    if (!m_isCreated) { return; }

    const auto& l_directCommandList  = a_directCommandList.GetREFCommandList ();
    const auto& l_computeCommandList = a_computeCommandList.GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList,  "DirectCommandListが作成されておらず、計測の開始に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_computeCommandList, "ComputeCommandListが作成されておらず、計測の開始に失敗しました。");

    ++m_frameCount;

    // 前回このフレームリソースで計測したGPUの結果を受け取る
    m_completedSampleList.clear();

    m_directQueryRecorder.BeginFrame (a_frameIndex, m_completedSampleList);
    m_computeQueryRecorder.BeginFrame(a_frameIndex, m_completedSampleList);

    if (IsMeasuring())
    {
        for (const auto& l_sample : m_completedSampleList)
        {
            AddSample(l_sample, m_gpuStatisticsList);
        }
    }

    // フレーム全体のGPU処理時間を計測するため、記録の先頭で区間を開く
    m_directQueryRecorder.BeginScope (*l_directCommandList.Get(),  k_directQueueFrameScopeName);
    m_computeQueryRecorder.BeginScope(*l_computeCommandList.Get(), k_computeQueueFrameScopeName);
}

void FWK::Graphics::FrameProfiler::BeginGPUScope(const DirectCommandList& a_directCommandList, const std::string_view a_name)
{
    if (!m_isCreated) { return; }

    const auto& l_directCommandList = a_directCommandList.GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList, "DirectCommandListが作成されておらず、GPU区間の計測開始に失敗しました。");

    m_directQueryRecorder.BeginScope(*l_directCommandList.Get(), a_name);
}
void FWK::Graphics::FrameProfiler::EndGPUScope(const DirectCommandList& a_directCommandList)
{
    if (!m_isCreated) { return; }

    const auto& l_directCommandList = a_directCommandList.GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList, "DirectCommandListが作成されておらず、GPU区間の計測終了に失敗しました。");

    m_directQueryRecorder.EndScope(*l_directCommandList.Get());
}

void FWK::Graphics::FrameProfiler::BeginCPUScope(const std::string_view a_name)
{
    OpenCPUScope l_openCPUScope = {};

    // steady_clockは時刻合わせなどで巻き戻らない時計のため、処理時間の計測に向いている
    l_openCPUScope.m_name      = a_name;
    l_openCPUScope.m_beginTime = std::chrono::steady_clock::now();

    m_openCPUScopeStack.emplace_back(l_openCPUScope);
}
void FWK::Graphics::FrameProfiler::EndCPUScope()
{
    FWK_ASSERT_RETURN_IF(m_openCPUScopeStack.empty(), "BeginCPUScopeが呼ばれていないため、CPU区間の計測終了に失敗しました。");

    const auto l_openCPUScope = m_openCPUScopeStack.back();

    m_openCPUScopeStack.pop_back();

    if (!IsMeasuring()) { return; }

    const auto l_elapsedTime = std::chrono::steady_clock::now() - l_openCPUScope.m_beginTime;

    Struct::FrameProfileSample l_sample = {};

    l_sample.m_name      = l_openCPUScope.m_name;
    l_sample.m_elapsedMS = std::chrono::duration<double, std::milli>(l_elapsedTime).count();

    AddSample(l_sample, m_cpuStatisticsList);
}

void FWK::Graphics::FrameProfiler::EndFrame(const DirectCommandList& a_directCommandList, const ComputeCommandList& a_computeCommandList)
{
    if (!m_isCreated) { return; }

    const auto& l_directCommandList  = a_directCommandList.GetREFCommandList ();
    const auto& l_computeCommandList = a_computeCommandList.GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList,  "DirectCommandListが作成されておらず、計測の終了に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_computeCommandList, "ComputeCommandListが作成されておらず、計測の終了に失敗しました。");

    // BeginFrameで開いたフレーム全体の区間を閉じる
    m_directQueryRecorder.EndScope (*l_directCommandList.Get());
    m_computeQueryRecorder.EndScope(*l_computeCommandList.Get());

    // 記録したタイムスタンプをCPUが読めるバッファへコピーする命令を積む
    // (CommandListをCloseする前に呼ぶ必要がある)
    m_directQueryRecorder.Resolve (*l_directCommandList.Get());
    m_computeQueryRecorder.Resolve(*l_computeCommandList.Get());
}

void FWK::Graphics::FrameProfiler::AddCounterSample(const std::string_view a_name, const double a_value)
{
    if (!IsMeasuring()) { return; }

    // 処理時間と同じ集計(平均・最小・最大)を使うため、FrameProfileSampleとして記録する
    Struct::FrameProfileSample l_sample = {};

    l_sample.m_name      = a_name;
    l_sample.m_elapsedMS = a_value;

    AddSample(l_sample, m_counterStatisticsList);
}

void FWK::Graphics::FrameProfiler::AddReportNote(const std::string& a_note)
{
    m_reportNoteList.emplace_back(a_note);
}

void FWK::Graphics::FrameProfiler::WriteReport() const
{
    std::error_code l_errorCode = {};

    // 出力先フォルダがなければ作成する
    std::filesystem::create_directories(k_reportFilePath.parent_path(), l_errorCode);

    std::ofstream l_ofstream(k_reportFilePath);

    FWK_ASSERT_RETURN_IF(!l_ofstream, "FrameProfilerのレポートファイルを開けませんでした。");

    const auto l_measuredFrameCount = m_frameCount > k_warmupFrameCount ? m_frameCount - k_warmupFrameCount : k_initialFrameCount;

    l_ofstream << std::format("[FrameProfileReport]\n");
    l_ofstream << std::format("MeasuredFrameCount : {} (最初の{}フレームは集計対象外)\n", l_measuredFrameCount, k_warmupFrameCount);
    l_ofstream << std::format("TargetFrameMS      : {:.3f} (60FPS)\n\n", k_targetFrameMS);

    l_ofstream << "[Note]\n";

    for (const auto& l_note : m_reportNoteList)
    {
        l_ofstream << l_note << "\n";
    }

    l_ofstream << "\n[CPU] Name, AverageMS, MINMS, MAXMS, SampleCount\n";

    WriteStatisticsList(m_cpuStatisticsList, l_ofstream);

    l_ofstream << "\n[GPU] Name, AverageMS, MINMS, MAXMS, SampleCount\n";

    WriteStatisticsList(m_gpuStatisticsList, l_ofstream);

    l_ofstream << "\n[Counter] Name, Average, MIN, MAX, SampleCount\n";

    WriteStatisticsList(m_counterStatisticsList, l_ofstream);
}

void FWK::Graphics::FrameProfiler::AddSample(const Struct::FrameProfileSample& a_sample, std::vector<ProfileStatistics>& a_statisticsList) const
{
    // 区間の種類は数十個程度なので、先頭から順番に同じ名前を探す
    auto l_itr = std::ranges::find(a_statisticsList, a_sample.m_name, &ProfileStatistics::m_name);

    if (l_itr == a_statisticsList.end())
    {
        ProfileStatistics l_statistics = {};

        l_statistics.m_name = a_sample.m_name;

        a_statisticsList.emplace_back(l_statistics);

        l_itr = std::prev(a_statisticsList.end());
    }

    l_itr->m_totalMS += a_sample.m_elapsedMS;
    l_itr->m_minMS    = std::min(l_itr->m_minMS, a_sample.m_elapsedMS);
    l_itr->m_maxMS    = std::max(l_itr->m_maxMS, a_sample.m_elapsedMS);

    ++l_itr->m_sampleCount;
}

void FWK::Graphics::FrameProfiler::WriteStatisticsList(const std::vector<ProfileStatistics>& a_statisticsList, std::ofstream& a_ofstream) const
{
    for (const auto& l_statistics : a_statisticsList)
    {
        if (l_statistics.m_sampleCount == ProfileStatistics::k_initialSampleCount) { continue; }

        const double l_averageMS = l_statistics.m_totalMS / static_cast<double>(l_statistics.m_sampleCount);

        a_ofstream << std::format("{}, {:.3f}, {:.3f}, {:.3f}, {}\n",
                                  l_statistics.m_name,
                                  l_averageMS,
                                  l_statistics.m_minMS,
                                  l_statistics.m_maxMS,
                                  l_statistics.m_sampleCount);
    }
}

bool FWK::Graphics::FrameProfiler::IsMeasuring() const
{
    return m_frameCount > k_warmupFrameCount;
}