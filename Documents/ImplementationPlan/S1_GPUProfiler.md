# S1 GPU の計測(タイムスタンプクエリ)と CPU の計測

## 目的

「重い」を感覚ではなく数字で確かめるための道具を作る。

- **GPU**: パス(影・Lit・UnLit・スキニングなど)ごとに、GPU が何ミリ秒かけたか。
- **CPU**: シーンの更新 / 描画の開始(GPU 待ちを含む)/ 描画命令の記録 / エディター / 描画の終了(画面の切り替え待ちを含む)の、それぞれの時間。
- エディターに「プロファイラー」ウィンドウを足して表示する。

S0 で置いたモデルを 1 体 → 100 体と増やしたとき、どこが伸びるかを見ると、S2 以降の改善の効果も数字で確かめられる。

- 例 : 60FPS なら 1 フレームの持ち時間は 16.6ms。
  - CPU の「描画命令の記録」が 8ms なら、ExecuteIndirect(S6)が効く。
  - GPU の「SkeletalAnimationComputePass」が 5ms なら、スキニングの間引き(フェーズ2 P1)が効く。

## DirectX12 の解説

### GPU の時間は CPU の時計では測れない

- CPU が `DispatchMesh` を呼んだ瞬間には、GPU はまだ何もしていない。コマンドリストに「命令を書いた」だけ。
- GPU が実際に描くのは、`ExecuteCommandLists` で渡した後、数フレーム遅れて。
- そのため、CPU の `std::chrono` で `Execute` の前後を測っても、**「命令を書くのにかかった時間(CPU)」しか分からない**。

### タイムスタンプクエリ

GPU に「この命令まで来たときの GPU の時計の値を書いて」と頼む仕組み。

| 用語 | 意味 |
|---|---|
| クエリヒープ(`ID3D12QueryHeap`) | GPU の時計の値を書き込む、GPU 側の小さな配列。種類 `D3D12_QUERY_HEAP_TYPE_TIMESTAMP` で作る |
| `EndQuery` | コマンドリストに「ここまで来たら、ヒープの○番に時計の値を書け」という命令を積む |
| `ResolveQueryData` | ヒープの値を、CPU が読めるバッファへコピーする命令を積む(ヒープは CPU から直接読めない) |
| READBACK ヒープ | GPU が書き、CPU が読むためのメモリ。UPLOAD(CPU が書き GPU が読む)の逆向き |
| `GetTimestampFrequency` | GPU の時計が 1 秒に何回進むか。例 : 10,000,000(1 秒に 1000 万回 = 0.1 マイクロ秒単位) |

時間の計算の例:

- パスの前のタイムスタンプ = 1,000,000、後 = 1,050,000、周波数 = 10,000,000
- (1,050,000 − 1,000,000) ÷ 10,000,000 = 0.005 秒 = **5ms**

### 結果は数フレーム後に読む

- GPU は数フレーム遅れて動くので、今フレームに積んだクエリの結果は、まだ書かれていない。
- このエンジンはフレームリソースが 3 つ(GraphicsCONFIG.json の `FrameResourceList.Count`)。
  フレームリソース 0 を次に使うとき(3 フレーム後)は、`Renderer::BeginFrame` の `EnsureAllocatorAvailable` で
  「前回フレームリソース 0 で積んだ命令が、GPU で全部終わった」ことをフェンスで待っている。
- そこで、**フレームリソースごとに READBACK バッファの区画を分け**、次にそのフレームリソースを使うときに前回の結果を読む。

```
クエリヒープ(と READBACK バッファ)の並び : 1 区画 = 64 スコープ × 2(前・後)= 128 個
[ フレーム0 の 128 個 ][ フレーム1 の 128 個 ][ フレーム2 の 128 個 ]
```

### キューごとに時計がある

- スキニングはコンピュートキュー、描画はダイレクトキューで動く。
- 時計(周波数)はキューごとに取得するので、**キューごとに別のプロファイラー**を持つ(Renderer に2つ)。

## ファイル一覧

### 新規

| ファイル | 内容 |
|---|---|
| `Source/Framework/Definition/Constant/Profiler/ProfilerConstant.h` | 平均の重みなど |
| `Source/Framework/Definition/Struct/Profiler/ProfilerStruct.h` | `Struct::ProfilerResult`(名前・今回・平均) |
| `Source/Framework/Definition/Struct/Profiler/CPUTimeProfilerStruct.h` | `Struct::CPUTimeProfilerScope` |
| `Source/Framework/Definition/Struct/Graphics/GPUTimestampProfilerStruct.h` | `Struct::GPUTimestampProfilerScope` / `FrameData` |
| `Source/Framework/Utility/Profiler/ProfilerUtility.h` | `Utility::ApplyProfilerResult`(CPU と GPU の両方から使う) |
| `Source/Framework/Profiler/CPU/CPUTimeProfiler.h/.cpp` | CPU の計測 |
| `Source/Framework/Graphics/Render/Profiler/GPUTimestampProfiler.h/.cpp` | GPU の計測 |
| `Source/Framework/Editor/Window/Profiler/ProfilerEditorWindow.h/.cpp` | 表示用のウィンドウ |

### 変更

| ファイル | 変更 |
|---|---|
| `Graphics/Command/List/CommandListBase.h` | `EndTimestampQuery` / `ResolveTimestampQueryData` |
| `Graphics/Render/Renderer.h/.cpp` | プロファイラーを2つ持つ。BeginFrame / EndFrame で計測 |
| `Graphics/Render/Graph/RenderGraph.h/.cpp` | `ExecutePass` でパスの前後を計測 |
| `Source/Application/Application.h/.cpp` | `FWK::CPUTimeProfiler` を持ち、ループの各段を計測 |
| `CONFIG/Editor/EditorCONFIG.json` | `WindowList` に `ProfilerEditorWindow` |

### 登録

- フィルター: `Source\Framework\Definition\Constant\Profiler` / `Source\Framework\Definition\Struct\Profiler` / `Source\Framework\Utility\Profiler` / `Source\Framework\Profiler` / `Source\Framework\Profiler\CPU` / `Source\Framework\Graphics\Render\Profiler` / `Source\Framework\Editor\Window\Profiler`
- Framework.h:
  - Constant の区画に `ProfilerConstant.h`
  - Struct の区画に `ProfilerStruct.h` → `CPUTimeProfilerStruct.h` → `GPUTimestampProfilerStruct.h`
  - Utility の区画に `ProfilerUtility.h`
  - `Profiler/CPU/CPUTimeProfiler.h` は Utility の後(Graphics より前でよい)
  - `Graphics/Render/Profiler/GPUTimestampProfiler.h` は CommandListBase より後、Renderer より前
  - `Editor/Window/Profiler/ProfilerEditorWindow.h` は他のエディターウィンドウと同じ区画

---

## コード

### Definition/Constant/Profiler/ProfilerConstant.h(新規)

```cpp
#pragma once

namespace FWK::Constant
{
    inline constexpr float k_profilerAverageWeight = 0.1F;
    inline constexpr float k_millisecondsPerSecond = 1000.0F;
}
```

### Definition/Struct/Profiler/ProfilerStruct.h(新規)

```cpp
#pragma once

namespace FWK::Struct
{
    struct ProfilerResult final
    {
        static constexpr float k_initialMilliseconds = 0.0F;

        std::string m_name = {};

        float m_milliseconds        = k_initialMilliseconds;
        float m_averageMilliseconds = k_initialMilliseconds;
    };
}
```

### Definition/Struct/Profiler/CPUTimeProfilerStruct.h(新規)

```cpp
#pragma once

namespace FWK::Struct
{
    struct CPUTimeProfilerScope final
    {
        static constexpr float k_initialMilliseconds = 0.0F;

        std::string m_name = {};

        std::chrono::steady_clock::time_point m_beginTimePoint = {};

        float m_milliseconds = k_initialMilliseconds;
    };
}
```

### Definition/Struct/Graphics/GPUTimestampProfilerStruct.h(新規)

```cpp
#pragma once

namespace FWK::Struct
{
    struct GPUTimestampProfilerScope final
    {
        static constexpr std::uint32_t k_initialBeginQueryIndex = 0U;

        std::string m_name = {};

        std::uint32_t m_beginQueryIndex = k_initialBeginQueryIndex;
    };

    struct GPUTimestampProfilerFrameData final
    {
        std::vector<GPUTimestampProfilerScope> m_scopeList = {};
    };
}
```

### Utility/Profiler/ProfilerUtility.h(新規・写経)

```cpp
#pragma once

namespace FWK::Utility
{
    inline void ApplyProfilerResult(const std::string_view& a_name, const float a_milliseconds, std::vector<Struct::ProfilerResult>& a_resultList)
    {
        // 計測した時間を、名前ごとの結果の一覧へ反映する関数
        // CPUの計測(CPUTimeProfiler)とGPUの計測(GPUTimestampProfiler)の両方から使う
        // 1フレームだけの値は、ほかの処理の割り込みなどで大きく揺れるため、
        // 「平均」も一緒に持ち、表示ではそちらを見られるようにする
        const auto& l_resultITR = std::ranges::find_if(a_resultList,
                                                       [&a_name](const auto& a_result)
                                                       {
                                                           return a_result.m_name == a_name;
                                                       });

        // 初めて出てきた名前なら、平均も今回の値から始める
        if (l_resultITR == a_resultList.end())
        {
            Struct::ProfilerResult l_result = {};

            l_result.m_name                = std::string{ a_name };
            l_result.m_milliseconds        = a_milliseconds;
            l_result.m_averageMilliseconds = a_milliseconds;

            a_resultList.emplace_back(std::move(l_result));

            return;
        }

        // 指数移動平均 : 平均 = 平均 + (今回 - 平均) × 重み
        // 重みが0.1なら、今回の値を1割だけ取り込む(直近の数十フレームをならした値になる)
        // 例 : 平均5.0ms、今回6.0msなら、5.0 + (6.0 - 5.0) × 0.1 = 5.1ms
        l_resultITR->m_milliseconds         = a_milliseconds;
        l_resultITR->m_averageMilliseconds += (a_milliseconds - l_resultITR->m_averageMilliseconds) * Constant::k_profilerAverageWeight;
    }
}
```

### Profiler/CPU/CPUTimeProfiler.h(新規)

```cpp
#pragma once

namespace FWK
{
    class CPUTimeProfiler final
    {
    public:

         CPUTimeProfiler() = default;
        ~CPUTimeProfiler() = default;

        void BeginFrame();
        void EndFrame  ();

        std::uint32_t BeginScope(const std::string_view& a_name);
        void          EndScope  (const std::uint32_t     a_scopeIndex);

        const auto& GetREFResultList() const { return m_resultList; }

        static constexpr std::uint32_t k_invalidScopeIndex = std::numeric_limits<std::uint32_t>::max();

    private:

        std::vector<Struct::CPUTimeProfilerScope> m_scopeList  = {};
        std::vector<Struct::ProfilerResult>       m_resultList = {};
    };
}
```

### Profiler/CPU/CPUTimeProfiler.cpp(新規・写経)

```cpp
#include "CPUTimeProfiler.h"

// CPUの処理時間を、名前を付けた区間(スコープ)ごとに測るクラス
// 使い方 : BeginFrame → BeginScope / EndScope を何回か → EndFrame
// EndFrameで、このフレームに測った区間を結果の一覧(m_resultList)へ反映する
void FWK::CPUTimeProfiler::BeginFrame()
{
    // 前のフレームの区間を消す
    // clearは要素を消すだけで確保済みのメモリは残すため、毎フレームの確保は起きない
    m_scopeList.clear();
}
void FWK::CPUTimeProfiler::EndFrame()
{
    for (const auto& l_scope : m_scopeList)
    {
        Utility::ApplyProfilerResult(l_scope.m_name, l_scope.m_milliseconds, m_resultList);
    }
}

std::uint32_t FWK::CPUTimeProfiler::BeginScope(const std::string_view& a_name)
{
    Struct::CPUTimeProfilerScope l_scope = {};

    l_scope.m_name = std::string{ a_name };

    // steady_clockは、PCの時刻合わせで巻き戻ったりしない時計
    // 経過時間を測るときは、system_clockではなくこちらを使う
    l_scope.m_beginTimePoint = std::chrono::steady_clock::now();

    m_scopeList.emplace_back(std::move(l_scope));

    // 今追加した区間の番号を返す
    // EndScopeにこの番号を渡して、区間の終わりを記録する
    return static_cast<std::uint32_t>(m_scopeList.size() - k_lastScopeOffset);
}
void FWK::CPUTimeProfiler::EndScope(const std::uint32_t a_scopeIndex)
{
    FWK_ASSERT_RETURN_IF(a_scopeIndex >= m_scopeList.size(), "区間の番号が範囲外のため、CPUの計測に失敗しました。");

    auto& l_scope = m_scopeList[a_scopeIndex];

    // 開始からの経過時間を、ミリ秒の小数で求める
    // duration<float, std::milli>は「ミリ秒単位のfloat」で時間の長さを表す型
    // 例 : 開始から0.0042秒たっていれば、4.2Fになる
    const std::chrono::duration<float, std::milli> l_elapsedTime = std::chrono::steady_clock::now() - l_scope.m_beginTimePoint;

    l_scope.m_milliseconds = l_elapsedTime.count();
}
```

> `k_lastScopeOffset` はクラスの private に `static constexpr std::size_t k_lastScopeOffset = 1ULL;` として足す(ヘッダーの private の先頭)。

### Graphics/Command/List/CommandListBase.h(変更)

`CopyBufferRegion` の後ろ(public)に追加する。

```cpp
        void EndTimestampQuery(const TypeAlias::ComPtr<ID3D12QueryHeap>& a_queryHeap, const UINT a_queryIndex) const
        {
            const auto& l_commandList = GetREFCommandList();

            FWK_ASSERT_RETURN_IF(!l_commandList, "コマンドリストが作成されておらず、タイムスタンプの記録に失敗しました。");
            FWK_ASSERT_RETURN_IF(!a_queryHeap,   "クエリヒープが無効のため、タイムスタンプの記録に失敗しました。");

            // GPUがこの命令まで処理を進めた時点の、GPUの時計の値をクエリヒープへ書かせる命令を積む
            // CPUがこの関数を呼んだ時点では書かれない(GPUが実際にここまで来たときに書かれる)
            // EndQuery(書き込み先のクエリヒープ、
            //          クエリの種類(タイムスタンプ)、
            //          ヒープの何番目に書くか);
            l_commandList->EndQuery(a_queryHeap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, a_queryIndex);
        }

        void ResolveTimestampQueryData(const TypeAlias::ComPtr<ID3D12QueryHeap>& a_queryHeap,
                                       const TypeAlias::ComPtr<ID3D12Resource2>& a_destinationBuffer,
                                       const UINT64&                             a_destinationOffset,
                                       const UINT                                a_startQueryIndex,
                                       const UINT                                a_queryCount) const
        {
            const auto& l_commandList = GetREFCommandList();

            FWK_ASSERT_RETURN_IF(!l_commandList,       "コマンドリストが作成されておらず、タイムスタンプの読み出しに失敗しました。");
            FWK_ASSERT_RETURN_IF(!a_queryHeap,         "クエリヒープが無効のため、タイムスタンプの読み出しに失敗しました。");
            FWK_ASSERT_RETURN_IF(!a_destinationBuffer, "コピー先のバッファが無効のため、タイムスタンプの読み出しに失敗しました。");

            // クエリヒープはCPUから直接読めないため、CPUが読めるバッファ(READBACKヒープ)へコピーさせる命令を積む
            // 1つの値はUINT64(8バイト)なので、コピー先には「個数 × 8バイト」の空きが必要
            // ResolveQueryData(コピー元のクエリヒープ、
            //                  クエリの種類、
            //                  コピーを始めるヒープの番号、
            //                  コピーする個数、
            //                  コピー先のバッファ、
            //                  コピー先のバッファの何バイト目から書くか);
            l_commandList->ResolveQueryData(a_queryHeap.Get(),
                                            D3D12_QUERY_TYPE_TIMESTAMP,
                                            a_startQueryIndex,
                                            a_queryCount,
                                            a_destinationBuffer.Get(),
                                            a_destinationOffset);
        }
```

### Graphics/Render/Profiler/GPUTimestampProfiler.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class GPUTimestampProfiler final
    {
    public:

         GPUTimestampProfiler() = default;
        ~GPUTimestampProfiler() = default;

        GPUTimestampProfiler(const GPUTimestampProfiler&)  = delete;
        GPUTimestampProfiler(      GPUTimestampProfiler&&) = delete;

        GPUTimestampProfiler& operator=(const GPUTimestampProfiler&)  = delete;
        GPUTimestampProfiler& operator=(      GPUTimestampProfiler&&) = delete;

        bool Create(const TypeAlias::ComPtr<ID3D12CommandQueue>& a_commandQueue, const Device& a_device, const std::size_t& a_frameCount);

        void BeginFrame(const std::size_t& a_frameIndex);

        template <D3D12_COMMAND_LIST_TYPE CommandType>
        std::uint32_t BeginScope(const std::string_view& a_name, const CommandListBase<CommandType>& a_commandList)
        {
            // 計測する区間(スコープ)の始まりに、GPUの時計を記録する命令を積む
            // 戻り値は区間の番号で、EndScopeに渡して区間の終わりを記録する
            // 例 : 3番目の区間なら、このフレームの区画の 3 × 2 = 6番に「前」、7番に「後」の値が書かれる
            FWK_ASSERT_RETURN_VALUE_IF(m_currentFrameIndex >= m_frameDataList.size(), "フレームの番号が範囲外のため、GPUの計測の開始に失敗しました。", k_invalidScopeIndex);

            auto& l_scopeList = m_frameDataList[m_currentFrameIndex].m_scopeList;

            // 1フレームで測れる区間の数には上限がある(クエリヒープの大きさで決まる)
            // 上限を超えた区間は測らずに無効な番号を返す(EndScopeも何もしない)
            if (l_scopeList.size() >= k_maxScopeCount) { return k_invalidScopeIndex; }

            const auto& l_scopeIndex     = static_cast<std::uint32_t>(l_scopeList.size());
            const auto& l_frameBaseIndex = FetchVALFrameBaseQueryIndex(m_currentFrameIndex);

            Struct::GPUTimestampProfilerScope l_scope = {};

            l_scope.m_name            = std::string{ a_name };
            l_scope.m_beginQueryIndex = l_frameBaseIndex + l_scopeIndex * k_queryCountPerScope;

            a_commandList.EndTimestampQuery(m_queryHeap, l_scope.m_beginQueryIndex);

            l_scopeList.emplace_back(std::move(l_scope));

            return l_scopeIndex;
        }

        template <D3D12_COMMAND_LIST_TYPE CommandType>
        void EndScope(const CommandListBase<CommandType>& a_commandList, const std::uint32_t a_scopeIndex)
        {
            // BeginScopeが上限で測らなかった区間は、終わりも記録しない
            if (a_scopeIndex == k_invalidScopeIndex) { return; }

            FWK_ASSERT_RETURN_IF(m_currentFrameIndex >= m_frameDataList.size(), "フレームの番号が範囲外のため、GPUの計測の終了に失敗しました。");

            const auto& l_scopeList = m_frameDataList[m_currentFrameIndex].m_scopeList;

            FWK_ASSERT_RETURN_IF(a_scopeIndex >= l_scopeList.size(), "区間の番号が範囲外のため、GPUの計測の終了に失敗しました。");

            // 区間の「後」の値は、「前」の値の次の番号へ書く
            const auto& l_endQueryIndex = l_scopeList[a_scopeIndex].m_beginQueryIndex + k_endQueryOffset;

            a_commandList.EndTimestampQuery(m_queryHeap, l_endQueryIndex);
        }

        template <D3D12_COMMAND_LIST_TYPE CommandType>
        void EndFrame(const CommandListBase<CommandType>& a_commandList) const
        {
            // このフレームに積んだタイムスタンプを、CPUが読めるREADBACKバッファへコピーする命令を積む
            // コピーされた値を読むのは、このフレームリソースを次に使うとき(BeginFrame)
            // それまでにGPUがこのフレームの命令をすべて終えていることは、フェンスで保証されている
            FWK_ASSERT_RETURN_IF(m_currentFrameIndex >= m_frameDataList.size(), "フレームの番号が範囲外のため、GPUの計測の読み出しに失敗しました。");

            const auto& l_scopeList = m_frameDataList[m_currentFrameIndex].m_scopeList;

            // 区間を1つも測っていなければ、コピーするものがない
            if (l_scopeList.empty()) { return; }

            const auto& l_frameBaseIndex = FetchVALFrameBaseQueryIndex(m_currentFrameIndex);
            const auto& l_queryCount     = static_cast<UINT>(l_scopeList.size()) * k_queryCountPerScope;

            // READBACKバッファの並びは、クエリヒープの並びと同じにしてある
            // そのため、コピー先の位置は「ヒープの番号 × 8バイト」になる
            // 例 : フレーム1の区画は128番から始まるので、128 × 8 = 1024バイト目から書く
            const auto& l_destinationOffset = static_cast<UINT64>(l_frameBaseIndex) * k_timestampByteSize;

            a_commandList.ResolveTimestampQueryData(m_queryHeap,
                                                    m_readbackBuffer,
                                                    l_destinationOffset,
                                                    l_frameBaseIndex,
                                                    l_queryCount);
        }

        const auto& GetREFResultList() const { return m_resultList; }

        static constexpr std::uint32_t k_invalidScopeIndex = std::numeric_limits<std::uint32_t>::max();

    private:

        bool CreateQueryHeap     (const Device& a_device, const std::size_t& a_frameCount);
        bool CreateReadbackBuffer(const Device& a_device, const std::size_t& a_frameCount);

        void ReadFrameResult(const std::size_t& a_frameIndex);

        std::uint32_t FetchVALFrameBaseQueryIndex(const std::size_t& a_frameIndex) const;

        static constexpr UINT64 k_timestampByteSize          = sizeof(UINT64);
        static constexpr UINT64 k_initialTimestampFrequency  = 0ULL;

        static constexpr std::size_t k_initialFrameIndex = 0ULL;
        static constexpr std::size_t k_emptyFrameCount   = 0ULL;

        static constexpr SIZE_T k_emptyRangeBegin = 0ULL;
        static constexpr SIZE_T k_emptyRangeEnd   = 0ULL;

        static constexpr std::uint32_t k_maxScopeCount       = 64U;
        static constexpr std::uint32_t k_queryCountPerScope  = 2U;
        static constexpr std::uint32_t k_endQueryOffset      = 1U;

        static constexpr UINT k_defaultNodeMask = 0U;

        std::vector<Struct::GPUTimestampProfilerFrameData> m_frameDataList = {};
        std::vector<Struct::ProfilerResult>               m_resultList    = {};

        TypeAlias::ComPtr<ID3D12QueryHeap> m_queryHeap      = nullptr;
        TypeAlias::ComPtr<ID3D12Resource2> m_readbackBuffer = nullptr;

        std::size_t m_currentFrameIndex = k_initialFrameIndex;

        UINT64 m_timestampFrequency = k_initialTimestampFrequency;
    };
}
```

### Graphics/Render/Profiler/GPUTimestampProfiler.cpp(新規・写経)

```cpp
#include "GPUTimestampProfiler.h"

// GPUがパスごとに何ミリ秒かけたかを、タイムスタンプクエリで測るクラス
// コマンドキュー(ダイレクト / コンピュート)ごとにGPUの時計が違うため、キューごとに1つ作る
// 結果は数フレーム遅れて読める(GPUが数フレーム遅れて動くため)
bool FWK::Graphics::GPUTimestampProfiler::Create(const TypeAlias::ComPtr<ID3D12CommandQueue>& a_commandQueue, const Device& a_device, const std::size_t& a_frameCount)
{
    FWK_ASSERT_RETURN_VALUE_IF(!a_commandQueue,                   "コマンドキューが無効のため、GPUTimestampProfilerの作成に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF(a_frameCount == k_emptyFrameCount, "フレーム数が0のため、GPUTimestampProfilerの作成に失敗しました。",       false);

    // GPUの時計が1秒に何回進むかを取得する
    // タイムスタンプの差をこの値で割ると、秒になる
    // GetTimestampFrequency(1秒あたりの時計の進み数を書き込むアドレス);
    const auto& l_hr = a_commandQueue->GetTimestampFrequency(&m_timestampFrequency);

    FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "タイムスタンプの周波数の取得に失敗しました。", false);

    FWK_ASSERT_RETURN_VALUE_IF(!CreateQueryHeap     (a_device, a_frameCount), "タイムスタンプ用クエリヒープの作成に失敗しました。",   false);
    FWK_ASSERT_RETURN_VALUE_IF(!CreateReadbackBuffer(a_device, a_frameCount), "タイムスタンプ用READBACKバッファの作成に失敗しました。", false);

    // フレームリソースの数だけ、そのフレームで測った区間の一覧を持つ
    // 結果を読むとき(数フレーム後)に、どの番号がどの区間かを知るため
    m_frameDataList.resize(a_frameCount);

    return true;
}

void FWK::Graphics::GPUTimestampProfiler::BeginFrame(const std::size_t& a_frameIndex)
{
    FWK_ASSERT_RETURN_IF(a_frameIndex >= m_frameDataList.size(), "フレームの番号が範囲外のため、GPUの計測の開始に失敗しました。");

    m_currentFrameIndex = a_frameIndex;

    // このフレームリソースを前回使ったときに測った結果を読む
    // Renderer::BeginFrameのEnsureAllocatorAvailableで、前回の命令がGPUで終わるのを待った後に呼ばれるため、
    // READBACKバッファには、もう正しい値が書かれている
    ReadFrameResult(a_frameIndex);

    // 読み終わったので、今回の区間の一覧を空にする
    m_frameDataList[a_frameIndex].m_scopeList.clear();
}

bool FWK::Graphics::GPUTimestampProfiler::CreateQueryHeap(const Device& a_device, const std::size_t& a_frameCount)
{
    const auto& l_device = a_device.GetREFDevice();

    FWK_ASSERT_RETURN_VALUE_IF(!l_device, "デバイスが作成されておらず、クエリヒープの作成に失敗しました。", false);

    D3D12_QUERY_HEAP_DESC l_queryHeapDesc = {};

    // クエリヒープの種類
    // TIMESTAMPは、GPUの時計の値を書き込むためのヒープ
    l_queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;

    // ヒープに入るクエリの数
    // 1区間で「前」と「後」の2つ、1フレームで最大64区間、それをフレームリソースの数だけ用意する
    // 例 : フレームリソースが3つなら、64 × 2 × 3 = 384個
    l_queryHeapDesc.Count = static_cast<UINT>(a_frameCount) * k_maxScopeCount * k_queryCountPerScope;

    // どのGPUで使うか(GPUが1つなら0)
    l_queryHeapDesc.NodeMask = k_defaultNodeMask;

    // CreateQueryHeap(クエリヒープの設定、
    //                 受け取りたいCOMインターフェース型のID、
    //                 作成結果のポインタを書き込むアドレス);
    const auto& l_hr = l_device->CreateQueryHeap(&l_queryHeapDesc, IID_PPV_ARGS(m_queryHeap.ReleaseAndGetAddressOf()));

    FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "クエリヒープの作成に失敗しました。", false);

    return true;
}
bool FWK::Graphics::GPUTimestampProfiler::CreateReadbackBuffer(const Device& a_device, const std::size_t& a_frameCount)
{
    const auto& l_device = a_device.GetREFDevice();

    FWK_ASSERT_RETURN_VALUE_IF(!l_device, "デバイスが作成されておらず、READBACKバッファの作成に失敗しました。", false);

    // READBACKヒープ : GPUが書き込み、CPUが読むためのメモリ
    // UPLOADヒープ(CPUが書きGPUが読む)の逆向きで、GPUの計算結果をCPUへ持ち帰るときに使う
    const auto& l_heapProperties = CD3DX12_HEAP_PROPERTIES{ D3D12_HEAP_TYPE_READBACK };

    // バッファの大きさは、クエリヒープのクエリの数 × 8バイト
    // 例 : 384個なら 384 × 8 = 3072バイト
    const auto& l_bufferSize   = static_cast<UINT64>(a_frameCount) * k_maxScopeCount * k_queryCountPerScope * k_timestampByteSize;
    const auto& l_resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(l_bufferSize);

    // CreateCommittedResource(ヒープ設定、
    //                         ヒープ追加フラグ、
    //                         リソース設定、
    //                         初期リソース状態、
    //                         ClearValue(バッファなので未使用)、
    //                         受け取りたいインターフェース型、
    //                         作成結果のポインタ書き込み先);
    // READBACKヒープのリソースは、GPUからのコピー先にしか使えないため、COPY_DESTで作る(ほかの状態へは遷移できない)
    const auto& l_hr = l_device->CreateCommittedResource(&l_heapProperties,
                                                         D3D12_HEAP_FLAG_NONE,
                                                         &l_resourceDesc,
                                                         D3D12_RESOURCE_STATE_COPY_DEST,
                                                         nullptr,
                                                         IID_PPV_ARGS(m_readbackBuffer.ReleaseAndGetAddressOf()));

    FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "READBACKバッファの作成に失敗しました。", false);

    return true;
}

void FWK::Graphics::GPUTimestampProfiler::ReadFrameResult(const std::size_t& a_frameIndex)
{
    const auto& l_scopeList = m_frameDataList[a_frameIndex].m_scopeList;

    // 前回このフレームリソースで何も測っていなければ、読むものがない
    // (起動直後の数フレームはここで終わる)
    if (l_scopeList.empty()) { return; }

    FWK_ASSERT_RETURN_IF(m_timestampFrequency == k_initialTimestampFrequency, "タイムスタンプの周波数が0のため、GPUの計測結果の読み込みに失敗しました。");

    const auto& l_frameBaseIndex = FetchVALFrameBaseQueryIndex(a_frameIndex);
    const auto& l_queryCount     = l_scopeList.size() * k_queryCountPerScope;

    // CPUが読む範囲(バイト)を伝えてMapする
    // 範囲を伝えると、ドライバーがその部分だけをCPUから見えるように準備できる
    // D3D12_RANGE{ 読み始めのバイト, 読み終わりのバイト(含まない) }
    const D3D12_RANGE l_readRange = { static_cast<SIZE_T>(l_frameBaseIndex * k_timestampByteSize), static_cast<SIZE_T>((l_frameBaseIndex + l_queryCount) * k_timestampByteSize) };

    void* l_mappedData = nullptr;

    // Map(サブリソースの番号(バッファは0)、
    //     CPUが読む範囲、
    //     CPUから読めるアドレスの受け取り先);
    const auto& l_hr = m_readbackBuffer->Map(UploadBuffer::k_firstSubresourceIndex, &l_readRange, &l_mappedData);

    FWK_ASSERT_RETURN_IF(FAILED(l_hr), "READBACKバッファのMapに失敗しました。");

    // バッファの先頭からUINT64が並んでいるので、UINT64の配列として読む
    const auto* l_timestampList = static_cast<const UINT64*>(l_mappedData);

    for (const auto& l_scope : l_scopeList)
    {
        const auto& l_beginTimestamp = l_timestampList[l_scope.m_beginQueryIndex];
        const auto& l_endTimestamp   = l_timestampList[l_scope.m_beginQueryIndex + k_endQueryOffset];

        // 何らかの理由で前後が逆になっていたら(GPUがリセットされたなど)、その区間は使わない
        if (l_endTimestamp < l_beginTimestamp) { continue; }

        // (後 - 前) ÷ 周波数 = 秒、それを1000倍してミリ秒にする
        // 例 : 差が50,000、周波数が10,000,000なら、0.005秒 = 5ms
        const auto& l_elapsedTick  = l_endTimestamp - l_beginTimestamp;
        const float l_milliseconds = static_cast<float>(static_cast<double>(l_elapsedTick) / static_cast<double>(m_timestampFrequency)) * Constant::k_millisecondsPerSecond;

        Utility::ApplyProfilerResult(l_scope.m_name, l_milliseconds, m_resultList);
    }

    // CPUは書き込んでいないので、書き込んだ範囲は空(0 ~ 0)で伝える
    const D3D12_RANGE l_writtenRange = { k_emptyRangeBegin, k_emptyRangeEnd };

    m_readbackBuffer->Unmap(UploadBuffer::k_firstSubresourceIndex, &l_writtenRange);
}

std::uint32_t FWK::Graphics::GPUTimestampProfiler::FetchVALFrameBaseQueryIndex(const std::size_t& a_frameIndex) const
{
    // フレームリソースごとの区画が、クエリヒープの何番から始まるかを返す
    // 例 : 1区画が128個(64区間 × 2)なら、フレーム0は0番、フレーム1は128番、フレーム2は256番
    return static_cast<std::uint32_t>(a_frameIndex) * k_maxScopeCount * k_queryCountPerScope;
}
```

> `SIZE_T` は 64 ビット環境では `unsigned long long` なので、定数の接尾辞は `ULL` にしている。

### Graphics/Render/Renderer.h(変更)

```cpp
        auto& GetMutableREFDirectCommandList () { return m_directCommandList; }
        auto& GetMutableREFComputeCommandList() { return m_computeCommandList; }

        auto& GetMutableREFDirectGPUTimestampProfiler () { return m_directGPUTimestampProfiler; }
        auto& GetMutableREFComputeGPUTimestampProfiler() { return m_computeGPUTimestampProfiler; }
```

(const の取得関数も、ウィンドウが読むために足す)

```cpp
        const auto& GetREFDirectGPUTimestampProfiler () const { return m_directGPUTimestampProfiler; }
        const auto& GetREFComputeGPUTimestampProfiler() const { return m_computeGPUTimestampProfiler; }
```

private の定数とメンバ:

```cpp
        static constexpr std::string_view k_frameGPUTimestampScopeName = "フレーム全体";
```

```cpp
        SwapChain     m_swapChain         = {};
        ShadowContext m_shadowContext     = {};
        RenderView    m_mainRenderView    = {};
        RenderView    m_previewRenderView = {};

        GPUTimestampProfiler m_directGPUTimestampProfiler  = {};
        GPUTimestampProfiler m_computeGPUTimestampProfiler = {};

        TypeAlias::DirectCommandQueue  m_directCommandQueue  = {};
```

```cpp
        std::size_t m_currentFrameResourceIndex = k_initialFrameResourceIndex;

        std::uint32_t m_frameGPUTimestampScopeIndex = GPUTimestampProfiler::k_invalidScopeIndex;
```

> **プロファイラーはコマンドキューより前に宣言する(規約 20-6 の例外)。**
> メンバは宣言の逆の順に破棄される。キューのデストラクタは GPU の処理が終わるのを待つので、
> プロファイラーをキューより後に宣言すると、GPU がまだタイムスタンプを書いている READBACK バッファを先に解放してしまう。

### Graphics/Render/Renderer.cpp(変更・写経)

**PostDeserialize(キューを作った直後):**

```cpp
    // GPUの計測の準備
    // キューごとにGPUの時計(周波数)を取得するため、キューを作った後に作る
    FWK_ASSERT_RETURN_VALUE_IF(!m_directGPUTimestampProfiler.Create (m_directCommandQueue.GetREFCommandQueue (), a_device, m_frameResourceList.size()), "ダイレクトキュー用GPUTimestampProfilerの作成に失敗しました。",   false);
    FWK_ASSERT_RETURN_VALUE_IF(!m_computeGPUTimestampProfiler.Create(m_computeCommandQueue.GetREFCommandQueue(), a_device, m_frameResourceList.size()), "コンピュートキュー用GPUTimestampProfilerの作成に失敗しました。", false);
```

**BeginFrame(`ResetCommandObjects` の直後):**

```cpp
    // このフレームリソースを前回使ったときのGPUの計測結果を読み、今回の計測を始める
    // ResetCommandObjectsの中で、前回の命令がGPUで終わるのを待っているため、ここで読める
    m_directGPUTimestampProfiler.BeginFrame (m_currentFrameResourceIndex);
    m_computeGPUTimestampProfiler.BeginFrame(m_currentFrameResourceIndex);

    // ダイレクトキューの「フレーム全体」の計測を始める(終わりはEndFrame)
    m_frameGPUTimestampScopeIndex = m_directGPUTimestampProfiler.BeginScope(k_frameGPUTimestampScopeName, m_directCommandList);
```

**EndFrame(`m_renderGraph.EndFrame(*this);` の直後、Close の前):**

```cpp
    // 「フレーム全体」の計測を終え、このフレームのタイムスタンプをREADBACKバッファへコピーする命令を積む
    // Closeした後のコマンドリストには命令を積めないため、Closeより前に行う
    m_directGPUTimestampProfiler.EndScope(m_directCommandList, m_frameGPUTimestampScopeIndex);

    m_directGPUTimestampProfiler.EndFrame (m_directCommandList);
    m_computeGPUTimestampProfiler.EndFrame(m_computeCommandList);
```

### Graphics/Render/Graph/RenderGraph.h(変更)

private に追加:

```cpp
        std::string FetchVALGPUTimestampScopeName(const RenderGraphPassBase& a_pass, const Renderer& a_renderer) const;

        static constexpr std::string_view k_previewGPUTimestampScopeNameFormat = "{} [プレビュー]";
```

### Graphics/Render/Graph/RenderGraph.cpp(変更・写経)

**ExecutePass を次のようにする:**

```cpp
void FWK::Graphics::RenderGraph::ExecutePass(const ResourceContext& a_resourceContext, RenderGraphPassBase& a_pass, Renderer& a_renderer)
{
    // テーブルのコピー(Uploadレイヤー)はコピーキューで動く
    // コピーキューは、タイムスタンプを使えるかがGPUによって違うため測らず、パスだけを実行する
    if (a_pass.GetVALExecutionLayer() == Enum::RenderGraphPassExecutionLayer::Upload)
    {
        a_pass.Execute(a_resourceContext, a_renderer, *this);

        return;
    }

    // スキニングなどのアニメーションのパスはコンピュートキュー、それ以外はダイレクトキューで動く
    // GPUの時計はキューごとに違うため、パスが動くキューのプロファイラーで測る
    const bool  l_isComputePass         = a_pass.GetVALExecutionLayer() == Enum::RenderGraphPassExecutionLayer::Animation;
    const auto& l_gpuTimestampScopeName = FetchVALGPUTimestampScopeName  (a_pass, a_renderer);
    const auto& l_directCommandList     = a_renderer.GetREFDirectCommandList              ();
    const auto& l_computeCommandList    = a_renderer.GetREFComputeCommandList             ();
          auto& l_directProfiler        = a_renderer.GetMutableREFDirectGPUTimestampProfiler ();
          auto& l_computeProfiler       = a_renderer.GetMutableREFComputeGPUTimestampProfiler();
          auto  l_gpuTimestampScopeIndex = GPUTimestampProfiler::k_invalidScopeIndex;

    // パスの命令より前に「前」のタイムスタンプを積む
    if (l_isComputePass)
    {
        l_gpuTimestampScopeIndex = l_computeProfiler.BeginScope(l_gpuTimestampScopeName, l_computeCommandList);
    }
    else
    {
        l_gpuTimestampScopeIndex = l_directProfiler.BeginScope(l_gpuTimestampScopeName, l_directCommandList);
    }

    // Pass実行前に、ResourceAccessのbeforeUsageへ遷移する
    m_resourceTransitioner.TransitionPassResourceBefore(a_pass, a_renderer);

    // PassのWriteResourceを見て、RenderGraph側でRTVを自動セットする
    m_resourceBinder.SetupPassRenderTarget(a_resourceContext, a_pass, a_renderer);

    a_pass.Execute(a_resourceContext, a_renderer, *this);

    // Pass実行後に、ResourceAccessのafterUsageへ遷移する
    m_resourceTransitioner.TransitionPassResourceAfter(a_pass, a_renderer);

    // パスの命令(遷移を含む)の後に「後」のタイムスタンプを積む
    if (l_isComputePass)
    {
        l_computeProfiler.EndScope(l_computeCommandList, l_gpuTimestampScopeIndex);
    }
    else
    {
        l_directProfiler.EndScope(l_directCommandList, l_gpuTimestampScopeIndex);
    }
}
```

**RemoveExpiredPassList の前に追加:**

```cpp
std::string FWK::Graphics::RenderGraph::FetchVALGPUTimestampScopeName(const RenderGraphPassBase& a_pass, const Renderer& a_renderer) const
{
    // 計測結果に表示する名前は、パスのクラス名(TypeINFOのk_name)にする
    // 例 : "StaticModelStandardLitPass"
    const auto& l_passTypeINFO = a_pass.GetREFRuntimeTypeINFO();

    // プレビューのビューを描いているときは、同じパスがもう一度動くため、名前で区別する
    // 例 : "StaticModelStandardLitPass [プレビュー]"
    if (const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();
        l_currentFrameResource &&
        l_currentFrameResource->GetVALActiveViewType() == Enum::RenderViewType::Preview)
    {
        return std::format(k_previewGPUTimestampScopeNameFormat, l_passTypeINFO.k_name);
    }

    return std::string{ l_passTypeINFO.k_name };
}
```

### Source/Application/Application.h(変更)

```cpp
    const auto& GetREFWindow         () const { return m_window; }
    const auto& GetREFFPSController  () const { return m_fpsController; }
    const auto& GetREFCPUTimeProfiler() const { return m_cpuTimeProfiler; }
```

```cpp
    static constexpr std::string_view k_sceneUpdateCPUScopeName     = "シーンの更新";
    static constexpr std::string_view k_graphicsBeginCPUScopeName   = "描画の開始(GPU待ちを含む)";
    static constexpr std::string_view k_graphicsExecuteCPUScopeName = "描画命令の記録";
    static constexpr std::string_view k_editorDrawCPUScopeName      = "エディターの描画";
    static constexpr std::string_view k_graphicsEndCPUScopeName     = "描画の終了(画面の切り替え待ちを含む)";
```

> 文字列の定数は規約 10-7 のとおり `static constexpr std::string_view`。既存の `k_titleName` などは `std::string` / `std::filesystem::path`(constexpr にできない型)なので `inline static const` のままでよい。
> 規約 20-3 の並び(文字列は配列・ポインタの次)に合わせ、既存の `k_titleName` の前に置く。

```cpp
    FWK::Window          m_window;
    FWK::FPSController   m_fpsController;
    FWK::CPUTimeProfiler m_cpuTimeProfiler;
```

### Source/Application/Application.cpp(変更・写経)

- コンストラクタの初期化子リストの最後に `m_cpuTimeProfiler()` を足す。
- ループの中を次のように区切る(既存の処理はそのまま、前後に BeginScope / EndScope を足す)。

```cpp
        l_inputManager.Update();

        // このフレームのCPUの計測を始める
        m_cpuTimeProfiler.BeginFrame();

        // CharacterVirtualのDraw内容も受け取るため、ここでクリア
        l_physicsManager.ClearFrame();

        // 更新
        const auto l_sceneUpdateScopeIndex = m_cpuTimeProfiler.BeginScope(k_sceneUpdateCPUScopeName);

        l_sceneManager.EarlyUpdate   ();
        l_sceneManager.Update        ();
        l_sceneManager.LateUpdate    ();
        l_sceneManager.PostLateUpdate();

        m_cpuTimeProfiler.EndScope(l_sceneUpdateScopeIndex);

        ...(既存の処理)...

        // 描画処理
        // BeginFrameは、GPUが3フレーム前の命令を終えるまで待つことがあるため、その待ち時間も含まれる
        const auto l_graphicsBeginScopeIndex = m_cpuTimeProfiler.BeginScope(k_graphicsBeginCPUScopeName);

        l_graphicsManager.BeginFrame();

        m_cpuTimeProfiler.EndScope(l_graphicsBeginScopeIndex);

        // Executeは、パスごとの描画命令をコマンドリストへ書くだけの時間(GPUが描く時間ではない)
        const auto l_graphicsExecuteScopeIndex = m_cpuTimeProfiler.BeginScope(k_graphicsExecuteCPUScopeName);

        l_graphicsManager.Execute();

        m_cpuTimeProfiler.EndScope(l_graphicsExecuteScopeIndex);

        // エディターの描画
        const auto l_editorDrawScopeIndex = m_cpuTimeProfiler.BeginScope(k_editorDrawCPUScopeName);

        l_editorManager.DrawEditor();

        m_cpuTimeProfiler.EndScope(l_editorDrawScopeIndex);

        // EndFrameのPresentは、垂直同期(SyncInterval = 1)のとき、画面の切り替えのタイミングまで待つ
        // そのため、ここが長くても「重い」とは限らない(余った時間を待っているだけのことが多い)
        const auto l_graphicsEndScopeIndex = m_cpuTimeProfiler.BeginScope(k_graphicsEndCPUScopeName);

        l_graphicsManager.EndFrame();

        m_cpuTimeProfiler.EndScope(l_graphicsEndScopeIndex);

        // このフレームに測った区間を、結果の一覧へ反映する
        m_cpuTimeProfiler.EndFrame();

        EndFrame();
```

> `BeginScope` の戻り値は `std::uint32_t`(4バイト)なので、規約 10-1 のとおり `const auto` の値で受ける。

### Editor/Window/Profiler/ProfilerEditorWindow.h(新規)

```cpp
#pragma once

namespace FWK::Editor
{
    class ProfilerEditorWindow final : public EditorWindowBase
    {
    public:

         ProfilerEditorWindow()          = default;
        ~ProfilerEditorWindow() override = default;

        void Draw(EditorManager& a_editorManager) override;

    private:

        void DrawProfilerResultTable(const std::vector<Struct::ProfilerResult>& a_resultList, const std::string_view& a_tableID) const;

        static constexpr std::string_view k_editorName                 = "プロファイラー";
        static constexpr std::string_view k_thisWindowExplanationLabel = "CPUとGPUが、1フレームの中の各処理に何ミリ秒かけたかを確認するウィンドウ。";
        static constexpr std::string_view k_cpuHeaderLabel             = "CPU";
        static constexpr std::string_view k_directGPUHeaderLabel       = "GPU(描画 : ダイレクトキュー)";
        static constexpr std::string_view k_computeGPUHeaderLabel      = "GPU(計算 : コンピュートキュー)";
        static constexpr std::string_view k_cpuTableID                 = "##ProfilerEditorWindowCPUTable";
        static constexpr std::string_view k_directGPUTableID           = "##ProfilerEditorWindowDirectGPUTable";
        static constexpr std::string_view k_computeGPUTableID          = "##ProfilerEditorWindowComputeGPUTable";
        static constexpr std::string_view k_nameColumnLabel            = "処理";
        static constexpr std::string_view k_currentColumnLabel         = "今回(ms)";
        static constexpr std::string_view k_averageColumnLabel         = "平均(ms)";
        static constexpr std::string_view k_millisecondsFormat         = "%.3f";

        static constexpr int k_tableColumnCount   = 3;
        static constexpr int k_nameColumnIndex    = 0;
        static constexpr int k_currentColumnIndex = 1;
        static constexpr int k_averageColumnIndex = 2;

        FWK_DEFINE_TYPE_INFO(ProfilerEditorWindow, EditorWindowBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::ProfilerEditorWindow)
```

### Editor/Window/Profiler/ProfilerEditorWindow.cpp(新規・写経)

```cpp
#include "ProfilerEditorWindow.h"
#include "../../../../Application/Application.h"

void FWK::Editor::ProfilerEditorWindow::Draw(EditorManager& a_editorManager)
{
    if (!ImGui::Begin(k_editorName.data()))
    {
        ImGui::End();

        return;
    }

    // このウィンドウがクリックされているかどうかを判定
    ReportActiveWindowIfMouseClicked(a_editorManager);

    Utility::IMGUIDelayedTooltip(k_thisWindowExplanationLabel);

    const auto& l_application       = Application::GetInstance                      ();
    const auto& l_cpuTimeProfiler   = l_application.GetREFCPUTimeProfiler           ();
    const auto& l_graphicsManager   = Graphics::GraphicsManager::GetInstance        ();
    const auto& l_renderer          = l_graphicsManager.GetREFRenderer              ();
    const auto& l_directProfiler    = l_renderer.GetREFDirectGPUTimestampProfiler   ();
    const auto& l_computeProfiler   = l_renderer.GetREFComputeGPUTimestampProfiler  ();

    // CollapsingHeaderは、クリックで開閉できる見出し
    // ImGuiTreeNodeFlags_DefaultOpenを付けると、最初から開いた状態で表示される
    if (ImGui::CollapsingHeader(k_cpuHeaderLabel.data(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        DrawProfilerResultTable(l_cpuTimeProfiler.GetREFResultList(), k_cpuTableID);
    }

    if (ImGui::CollapsingHeader(k_directGPUHeaderLabel.data(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        DrawProfilerResultTable(l_directProfiler.GetREFResultList(), k_directGPUTableID);
    }

    if (ImGui::CollapsingHeader(k_computeGPUHeaderLabel.data(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        DrawProfilerResultTable(l_computeProfiler.GetREFResultList(), k_computeGPUTableID);
    }

    ImGui::End();
}

void FWK::Editor::ProfilerEditorWindow::DrawProfilerResultTable(const std::vector<Struct::ProfilerResult>& a_resultList, const std::string_view& a_tableID) const
{
    // BeginTableがfalseを返したとき(表が画面外で描く必要がないなど)は、EndTableを呼んではいけない
    // ImGui::BeginTable(表のID、
    //                   列の数、
    //                   表の見た目の設定(枠線を引く / 1行おきに背景色を変える));
    if (!ImGui::BeginTable(a_tableID.data(), k_tableColumnCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) { return; }

    // 1行目の見出し
    ImGui::TableSetupColumn(k_nameColumnLabel.data());
    ImGui::TableSetupColumn(k_currentColumnLabel.data());
    ImGui::TableSetupColumn(k_averageColumnLabel.data());
    ImGui::TableHeadersRow ();

    for (const auto& l_result : a_resultList)
    {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(k_nameColumnIndex);
        ImGui::TextUnformatted    (l_result.m_name.c_str());

        ImGui::TableSetColumnIndex(k_currentColumnIndex);
        ImGui::Text               (k_millisecondsFormat.data(), l_result.m_milliseconds);

        ImGui::TableSetColumnIndex(k_averageColumnIndex);
        ImGui::Text               (k_millisecondsFormat.data(), l_result.m_averageMilliseconds);
    }

    ImGui::EndTable();
}
```

### CONFIG/Editor/EditorCONFIG.json(変更)

`WindowList` に追加する。

```json
        {
            "Window": null,
            "WindowTypeName": "ProfilerEditorWindow"
        }
```

---

## 動作の確認

1. 起動して「プロファイラー」ウィンドウを開く。CPU と GPU の表が出る。
2. GPU(描画)に `フレーム全体` と各パスの名前が並ぶ。
3. S0 のモデルを 1 体置いたときと、複製して 100 体にしたときの数字を控えておく(S3・S5・S6 で比べる)。
   - 特に見る所: CPU の「描画命令の記録」、GPU の `ModelCascadeShadowPass`・`StaticModelStandardLitPass`・`SkeletalAnimationComputePass`。

## 次のステップへのつながり

- S2 で作るテーブルの「書き換えたものだけ送る」は、CPU の「描画命令の記録」と、GPU の UPLOAD からのコピー量を減らす。
- S6 の ExecuteIndirect は、CPU の「描画命令の記録」をほぼ一定(パスの数 × 数命令)にする。
