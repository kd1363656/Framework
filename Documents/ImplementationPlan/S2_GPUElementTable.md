# S2 GPUElementTable(変わった要素だけ GPU へ送るテーブル)

> **2026-10-10 改訂(容量と1要素の大きさは Create の前に Set する)** : ModelRenderSystem が `m_tableSettingList` を持たず、
> Deserialize の時点でテーブルを作って map へ入れ、Create では map を回すだけにした(Renderer の RootSignatureMap と同じ形、S3 参照)。
> そのため、容量と1要素の大きさは `SetCapacity` / `SetElementByteStride` で先に持たせ、`Create` の引数から外した。
> Serialize で容量を書き出すために `GetVALCapacity` を足した。
> **写経し直すところ** : `Create` の中で `a_capacity` / `a_elementByteStride` を使っていた所を `m_capacity` / `m_elementByteStride` にし(4か所)、
> 最後の `m_capacity = a_capacity;` / `m_elementByteStride = a_elementByteStride;` の2行を消す。引数の宣言はこちらで消した。

## 目的

今のモデル描画は、**毎フレーム・メッシュごと・パスごとに** `CBModelPerObject`(行列・マテリアルの値・SRV の番号)を作り直して書いている。
値が変わっていなくても書くので、同じ値を何度も書いている。

そこで、次の形の「テーブル」を作る。

- GPU 専用のメモリ(DEFAULT ヒープ)に、同じ構造体を並べた配列(StructuredBuffer)を1つ置く。
- 使う側は「何番目か(要素番号)」を1回だけもらい、値が変わったときだけ書く。
- 書いた要素だけを、次のフレームの最初に GPU へコピーする。
- シェーダーは「テーブルの SRV の番号」と「要素番号」で値を読む。

このクラスは、S3(オブジェクト・メッシュ)と S4(マテリアル)で使い回す。

### 数値の例

モデル 100 体(各 5 メッシュ)、パスが 4 つ(影 3 枚 + Lit)、動いているのは 10 体だけのとき。

| | 今 | テーブル化した後 |
|---|---|---|
| 毎フレーム書くデータ | 500 メッシュ × 4 パス × 256 バイト = **約 500KB** | 動いた 10 体 × 136 バイト = **約 1.4KB** |
| GPU へのコピー命令 | (CB は UPLOAD を直接読むのでコピーは無い) | 10 回の CopyBufferRegion(小さい) |

## DirectX12 の解説

### ヒープの種類(メモリの置き場所)

| 種類 | CPU から | GPU から | 使い道 |
|---|---|---|---|
| DEFAULT | 書けない・読めない | **一番速く読み書きできる** | テクスチャ・頂点・このテーブルの本体 |
| UPLOAD | 書ける(Map して memcpy) | 読める(遅め) | CPU から GPU へ渡す途中の置き場・毎フレーム変わる定数バッファ |
| READBACK | 読める | 書ける | GPU の結果を CPU へ戻す(S1 のタイムスタンプ) |

- テーブルは何度も読まれるので DEFAULT に置く。
- CPU は DEFAULT に直接書けないので、**UPLOAD に書いてから `CopyBufferRegion` で DEFAULT へコピー**する。

### StructuredBuffer と SRV

- **StructuredBuffer** : 「1 要素 ○ バイトの構造体が、○ 個並んだ配列」として GPU から読めるバッファ。HLSL では `StructuredBuffer<ModelObjectData>`。
- **SRV(シェーダーリソースビュー)** : 「このメモリを、1 要素何バイトの配列として、何要素ぶん読む」という読み方の説明書。ディスクリプタヒープの何番目かに置かれる。
- このエンジンは **Bindless**(ルートシグネチャの `CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED`)なので、シェーダーは SRV の番号だけで読める。

```hlsl
StructuredBuffer<ModelObjectData> l_objectTable = ResourceDescriptorHeap[g_objectTableSRVDescriptorIndex];
const ModelObjectData l_object = l_objectTable[g_objectIndex];
```

### UPLOAD をフレームの数だけ持つ理由

- GPU は数フレーム遅れて動く。フレーム N のコピー命令を GPU がまだ実行していないうちに、
  CPU がフレーム N+1 の値で同じ UPLOAD を上書きすると、フレーム N のコピーで N+1 の値が送られてしまう。
- フレームリソースと同じ数(3 つ)の UPLOAD を持ち、フレームごとに使い分ける。
  フレーム 0 の UPLOAD を次に使うのは 3 フレーム後で、そのときには `EnsureAllocatorAvailable` で GPU の完了を待っている。

### リソースバリア(状態の遷移)

D3D12 では、リソースを「今何に使っているか」を GPU に伝える必要がある。

| 状態 | 意味 |
|---|---|
| `COMMON` | 何にも使っていない(バッファは、コマンドリストの実行が終わるたびに自動でここへ戻る = **状態の減衰**) |
| `COPY_DEST` | コピー先として書かれる |
| `NON_PIXEL_SHADER_RESOURCE` / `PIXEL_SHADER_RESOURCE` | シェーダーから読まれる(ピクセルシェーダー以外 / ピクセルシェーダー)。2つを合わせたのが `ALL_SHADER_RESOURCE` |

コピーの前後に、次の2つのバリアを積む。

1. `COMMON → COPY_DEST` : これより前の「このテーブルを読む処理」(前のフレームの描画)が終わってからコピーする、と GPU に伝える。
2. `COPY_DEST → ALL_SHADER_RESOURCE` : コピーが終わってから、後の描画がテーブルを読む、と GPU に伝える。

- 書き換えが無いフレームは、バリアもコピーも積まない。テーブルは `COMMON` のままだが、
  **バッファは `COMMON` から読む状態へ自動で切り替わる(暗黙の昇格)** ため、シェーダーはそのまま読める。

### 要素番号の使い回し

- 削除された番号は「空き番号の一覧」に戻し、次の割り当てで使い回す。
- 前のフレームの描画が、その番号の古い値をまだ読んでいても問題ない。
  新しい値のコピーは、次のフレームの `COMMON → COPY_DEST` のバリアの後に行われ、
  同じダイレクトキューの上では「前のフレームの読み取りが終わってからコピーする」順番になる。

## ファイル一覧

### 新規

| ファイル | 内容 |
|---|---|
| `Source/Framework/Graphics/Resource/Buffer/Table/GPUElementTable.h/.cpp` | テーブル本体 |

### 登録

- フィルター: `Source\Framework\Graphics\Resource\Buffer\Table`
- Framework.h: `StaticStructuredBuffer.h` の後(StructuredBufferBase / UploadBuffer を使うため)

---

## コード

### Graphics/Resource/Buffer/Table/GPUElementTable.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class GPUElementTable final : public StructuredBufferBase
    {
    public:

         GPUElementTable();
        ~GPUElementTable() override;

        GPUElementTable(const GPUElementTable&)           = delete;
        GPUElementTable(      GPUElementTable&&) noexcept = delete;

        GPUElementTable& operator=(const GPUElementTable&)           = delete;
        GPUElementTable& operator=(      GPUElementTable&&) noexcept = delete;

        bool Create(const Device&                             a_device,
                    const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
                    const std::size_t&                        a_frameCount,
                          TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

        std::uint32_t AllocateElementIndex();

        void ReleaseElementIndex(const std::uint32_t a_elementIndex);

        template <typename ElementType>
        void WriteElement(const ElementType& a_element, const std::uint32_t a_elementIndex)
        {
            // GPUへはバイト列としてそのままコピーするため、memcpyできる単純なデータ型だけを許可する
            // (std::stringやstd::vectorのように、中身が別の場所にある型はコピーしても意味がない)
            static_assert(std::is_trivially_copyable_v<ElementType>, "GPUElementTableへ書き込む型は、triviallyCopyableである必要があります。");

            FWK_ASSERT_RETURN_IF(a_elementIndex >= m_capacity,               "書き込む番号がテーブルの容量を超えているため、テーブルへの書き込みに失敗しました。");
            FWK_ASSERT_RETURN_IF(sizeof(ElementType) != m_elementByteStride, "書き込む型の大きさがテーブルの1要素の大きさと違うため、テーブルへの書き込みに失敗しました。");

            const auto& l_byteOffset = static_cast<std::size_t>(a_elementIndex) * m_elementByteStride;

            // ここではCPU側の写しに書くだけで、GPUへはまだ送らない
            // 番号を書き換え待ちに入れておき、次のフレームのRecordUploadでまとめてGPUへコピーする
            // 同じフレームで同じ番号を何度書き換えても、コピーは1回で済む(setなので重複しない)
            // 例 : 1要素136バイトで5番を書くなら、写しの680バイト目から136バイトを書き換える
            std::memcpy(m_cpuElementDataList.data() + l_byteOffset, &a_element, sizeof(ElementType));

            m_dirtyElementIndexSet.emplace(a_elementIndex);
        }

        void RecordUpload(const DirectCommandList& a_directCommandList, const std::size_t& a_frameIndex);

        bool ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext) override;
        void Release       ()                                                                                    override;

        void SetCapacity         (const UINT a_set) { m_capacity          = a_set; }
        void SetElementByteStride(const UINT a_set) { m_elementByteStride = a_set; }

        UINT GetVALCapacity() const { return m_capacity; }

        static constexpr std::uint32_t k_invalidElementIndex = std::numeric_limits<std::uint32_t>::max();

    private:

        static constexpr std::size_t k_emptyFrameCount = 0ULL;

        static constexpr UINT k_initialCapacity          = 0U;
        static constexpr UINT k_initialElementByteStride = 0U;

        static constexpr std::uint32_t k_initialNextElementIndex = 0U;

        std::vector<UploadBuffer> m_uploadBufferList;

        std::vector<std::uint8_t> m_cpuElementDataList;

        std::vector<std::uint32_t> m_freeElementIndexList;

        std::unordered_set<std::uint32_t> m_dirtyElementIndexSet;

        UINT m_capacity;
        UINT m_elementByteStride;

        std::uint32_t m_nextElementIndex;
    };
}
```

### Graphics/Resource/Buffer/Table/GPUElementTable.cpp(新規・写経)

```cpp
#include "GPUElementTable.h"

// 同じ構造体をたくさん並べた配列(StructuredBuffer)を、GPU専用のメモリ(DEFAULTヒープ)に1つ持つクラス
// 使う側は「何番目か(要素番号)」を1回だけもらい、値が変わったときだけWriteElementで書く
// 書いた要素だけを、次のフレームの最初にRecordUploadでGPUへコピーする(毎フレーム全部は送らない)
// シェーダーは、テーブルのSRVの番号と要素番号を受け取り、ResourceDescriptorHeap[番号][要素番号]で値を読む
// 例 : 1要素136バイト、容量4096なら、テーブル本体は約557KB、フレームごとのUPLOADも同じ大きさを持つ
// ※ 注意 : 解放(Release)はGPUがこのテーブルを使い終わっている前提で、その場で行う
//           ModelRenderSystem(Rendererのメンバ)が持ち、アプリの終了時にだけ破棄される
//           Rendererはコマンドキューより前にModelRenderSystemを宣言しているため、
//           キューのデストラクタがGPUの完了を待った後に、このデストラクタが呼ばれる
FWK::Graphics::GPUElementTable::GPUElementTable() :
    m_uploadBufferList(),

    m_cpuElementDataList(),

    m_freeElementIndexList(),

    m_dirtyElementIndexSet(),

    m_capacity         (k_initialCapacity),
    m_elementByteStride(k_initialElementByteStride),

    m_nextElementIndex(k_initialNextElementIndex)
{}
FWK::Graphics::GPUElementTable::~GPUElementTable()
{
    Release();
}

bool FWK::Graphics::GPUElementTable::Create(const Device&                             a_device,
                                            const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
                                            const std::size_t&                        a_frameCount,
                                                  TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
    const auto& l_currentBufferGPUResource = GetREFBufferGPUResource();

    FWK_ASSERT_RETURN_VALUE_IF(a_frameCount == k_emptyFrameCount,                                      "フレーム数が0のため、GPUElementTableの作成に失敗しました。",                         false);
    FWK_ASSERT_RETURN_VALUE_IF(m_capacity == k_initialCapacity,                                        "容量が0のため、GPUElementTableの作成に失敗しました。",                               false);
    FWK_ASSERT_RETURN_VALUE_IF(m_elementByteStride == k_initialElementByteStride,                      "1要素の大きさが0のため、GPUElementTableの作成に失敗しました。",                      false);
    FWK_ASSERT_RETURN_VALUE_IF(l_currentBufferGPUResource.m_resource,                                  "既にGPUResourceを保持しているため、GPUElementTableの作成に失敗しました。",           false);
    FWK_ASSERT_RETURN_VALUE_IF(GetVALSRVDescriptorIndex() != DescriptorHeap::k_invalidDescriptorIndex, "既にSRVDescriptorIndexを保持しているため、GPUElementTableの作成に失敗しました。", false);

    // テーブル全体の大きさ = 容量 × 1要素の大きさ
    // 例 : 4096 × 136 = 557,056バイト
    const auto& l_bufferSize = static_cast<UINT64>(m_capacity) * static_cast<UINT64>(m_elementByteStride);

    // 失敗してもこのテーブルが中途半端な状態にならないように、まずはローカル変数で作る
    Struct::GPUResource l_bufferGPUResource = {};

    // シェーダーが読むテーブル本体を、DEFAULTヒープ(GPU専用のメモリ)に作る
    // DEFAULTヒープはGPUから一番速く読めるが、CPUからは直接書き込めない
    // そのため値は、CPUから書けるUPLOADヒープのバッファを経由して、CopyBufferRegionでコピーして入れる
    // 作成直後の状態はCOMMONにしておく
    // (バッファはコマンドリストの実行が終わるたびにCOMMONへ戻る(状態の減衰)ため、毎フレームCOMMONから始まる前提で扱える)
    FWK_ASSERT_RETURN_VALUE_IF(!a_gpuMemoryAllocator.CreateBufferResource(l_bufferSize,
                                                                          D3D12_RESOURCE_FLAG_NONE,
                                                                          D3D12_RESOURCE_STATE_COMMON,
                                                                          l_bufferGPUResource),
                                                                          "GPUElementTable用GPUResourceの作成に失敗しました。",
                                                                          false);

    // シェーダーからStructuredBuffer<構造体>として読めるように、SRV(シェーダーリソースビュー)を作る
    // SRVは「このメモリを、1要素何バイトの配列として、何要素ぶん読む」という読み方の説明書
    // 作ったSRVはディスクリプタヒープの何番目かに置かれ、シェーダーはその番号でResourceDescriptorHeapから取り出す
    const auto& l_srvDescriptorIndex = CreateSRV(a_device,
                                                 l_bufferGPUResource,
                                                 m_capacity,
                                                 m_elementByteStride,
                                                 a_cbvSRVUAVDescriptorPool);

    FWK_ASSERT_RETURN_VALUE_IF(l_srvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "GPUElementTable用SRVの作成に失敗しました。", false);

    // コピー元になるUPLOADヒープのバッファを、フレームの数だけ作る
    // GPUは数フレーム遅れて命令を実行するため、1つのバッファを使い回すと、
    // 前のフレームのコピーをGPUがまだ実行していないうちに、次のフレームの値で上書きしてしまう
    // フレームごとに分けておけば、そのフレームの命令が終わったこと(フェンス)を待ってから再利用されるので安全
    std::vector<UploadBuffer> l_uploadBufferList = {};

    l_uploadBufferList.resize(a_frameCount);

    for (auto& l_uploadBuffer : l_uploadBufferList)
    {
        if (!l_uploadBuffer.Create(a_device, l_bufferSize))
        {
            // 作ったSRVの番号は、ここで返さないと使用中のまま残る
            a_cbvSRVUAVDescriptorPool.Release(l_srvDescriptorIndex);

            FWK_ASSERT_RETURN_VALUE("GPUElementTable用UploadBufferの作成に失敗しました。", false);
        }
    }

    // 作成内容をメンバに反映
    SetBufferGPUResource (std::move(l_bufferGPUResource));
    SetSRVDescriptorIndex(l_srvDescriptorIndex);

    m_uploadBufferList = std::move(l_uploadBufferList);

    // CPU側の写しも、テーブルと同じ大きさで持つ
    // WriteElementはまずここへ書き、RecordUploadでここからUPLOADへ写す
    m_cpuElementDataList.resize(static_cast<std::size_t>(l_bufferSize));

    return true;
}

std::uint32_t FWK::Graphics::GPUElementTable::AllocateElementIndex()
{
    // 削除された要素が返した番号があれば、先にそれを使い回す
    // 例 : 0~9番を使っていて3番が返されたら、次は10番ではなく3番を渡す
    if (!m_freeElementIndexList.empty())
    {
        const auto l_elementIndex = m_freeElementIndexList.back();

        m_freeElementIndexList.pop_back();

        return l_elementIndex;
    }

    FWK_ASSERT_RETURN_VALUE_IF(m_nextElementIndex >= m_capacity, "テーブルの容量を超えたため、要素の番号の割り当てに失敗しました。", k_invalidElementIndex);

    const auto l_elementIndex = m_nextElementIndex;

    ++m_nextElementIndex;

    return l_elementIndex;
}

void FWK::Graphics::GPUElementTable::ReleaseElementIndex(const std::uint32_t a_elementIndex)
{
    if (a_elementIndex == k_invalidElementIndex) { return; }

    FWK_ASSERT_RETURN_IF(a_elementIndex >= m_capacity, "返す番号がテーブルの容量を超えているため、要素の番号の返却に失敗しました。");

    // まだGPUへ送っていない書き換えがあっても、もう使わない番号なので送らない
    m_dirtyElementIndexSet.erase(a_elementIndex);

    // 返した番号は、すぐに別の要素へ割り当ててよい
    // 前のフレームの描画がまだこの番号を読んでいても、新しい値のコピーはRecordUploadのバリアの後に行われるため、
    // GPUは「前のフレームの読み取りが終わってからコピーする」順番で実行する(同じダイレクトキューに積むため)
    m_freeElementIndexList.emplace_back(a_elementIndex);
}

void FWK::Graphics::GPUElementTable::RecordUpload(const DirectCommandList& a_directCommandList, const std::size_t& a_frameIndex)
{
    // 書き換えた要素が無いフレームは、何もしない(ほとんどのフレームはここで終わる)
    if (m_dirtyElementIndexSet.empty()) { return; }

    FWK_ASSERT_RETURN_IF(a_frameIndex >= m_uploadBufferList.size(), "フレームの番号がUploadBufferの数を超えているため、テーブルのGPUへのコピーに失敗しました。");

    const auto& l_bufferGPUResource = GetREFBufferGPUResource         ();
    const auto& l_uploadBuffer      = m_uploadBufferList[a_frameIndex];
    const auto& l_uploadResource    = l_uploadBuffer.GetREFUploadBuffer();
          auto* l_mappedData        = l_uploadBuffer.FetchPTRMappedData();

    FWK_ASSERT_RETURN_IF(!l_bufferGPUResource.m_resource, "テーブルのGPUResourceが無効なため、テーブルのGPUへのコピーに失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_uploadResource,               "UploadBufferが無効なため、テーブルのGPUへのコピーに失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_mappedData,                   "UploadBufferのMap済みデータが無効なため、テーブルのGPUへのコピーに失敗しました。");

    // リソースバリア(状態の遷移) : COMMON → COPY_DEST
    // D3D12では、リソースを「今何に使っているか」をGPUへ伝える必要がある(シェーダーが読む / コピー先 など)
    // このバリアを入れると、GPUは「これより前に積まれた、このテーブルを読む処理(前のフレームの描画)」が終わるまで待ってからコピーする
    // そのため、描画の途中でテーブルの値が書き換わることはない
    a_directCommandList.TransitionResourceBarrier(l_bufferGPUResource.m_resource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

    for (const auto& l_elementIndex : m_dirtyElementIndexSet)
    {
        const auto& l_byteOffset = static_cast<UINT64>(l_elementIndex) * static_cast<UINT64>(m_elementByteStride);

        // CPU側の写しから、このフレーム用のUPLOADバッファの同じ位置へ、書き換えた要素だけを写す
        // UPLOADバッファはMapしたままなので、memcpyするだけでGPUから見えるメモリに書ける
        std::memcpy(l_mappedData + l_byteOffset, m_cpuElementDataList.data() + l_byteOffset, m_elementByteStride);

        // UPLOADバッファ → テーブル本体へ、書き換えた要素の範囲だけをコピーする命令を積む
        // 例 : 1要素136バイトで5番を書き換えたなら、680バイト目から136バイトだけをコピーする
        a_directCommandList.CopyBufferRegion(l_byteOffset,
                                             l_byteOffset,
                                             m_elementByteStride,
                                             *l_bufferGPUResource.m_resource.Get(),
                                             *l_uploadResource.Get());
    }

    // リソースバリア(状態の遷移) : COPY_DEST → シェーダーが読む状態
    // NON_PIXEL_SHADER_RESOURCE(ピクセルシェーダー以外)とPIXEL_SHADER_RESOURCE(ピクセルシェーダー)のどちらからも読めるようにする
    // このバリアより後の描画は、コピーが終わってから実行される
    a_directCommandList.TransitionResourceBarrier(l_bufferGPUResource.m_resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

    m_dirtyElementIndexSet.clear();
}

bool FWK::Graphics::GPUElementTable::ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
    // 描画中に作り直すなど、GPUがまだ使っている可能性があるときは、こちらで遅延解放を予約する
    return ReserveReleaseCurrentResource(a_retiredFenceValue, a_resourceReleaseContext);
}
void FWK::Graphics::GPUElementTable::Release()
{
    const auto& l_bufferGPUResource = GetREFBufferGPUResource();

    // 既に解放するものがなければreturn
    if (!l_bufferGPUResource.m_resource &&
        GetVALSRVDescriptorIndex() == DescriptorHeap::k_invalidDescriptorIndex)
    {
        return;
    }

    // ※ 注意 : ここではGPUがこのテーブルを使い終わっている前提で、その場で解放する(クラスの説明を参照)
    //           ResourceContext(ディスクリプタプールとGPUMemoryAllocatorを持つ)はRendererより前に宣言されているため、
    //           Rendererのメンバが破棄されるこの時点では、まだ生きている
    auto& l_graphicsManager         = GraphicsManager::GetInstance                          ();
    auto& l_resourceContext         = l_graphicsManager.GetMutableREFResourceContext        ();
    auto& l_cbvSRVUAVDescriptorPool = l_resourceContext.GetMutableREFCBVSRVUAVDescriptorPool();

    // SRVの番号をディスクリプタプールへ返す
    ReleaseImmediatelySRVDescriptorIndex(l_cbvSRVUAVDescriptorPool);

    // 空のGPUResourceで上書きすると、ComPtrが手放され、D3D12MAのメモリが解放される
    SetBufferGPUResource(Struct::GPUResource{});

    m_uploadBufferList.clear    ();
    m_cpuElementDataList.clear  ();
    m_freeElementIndexList.clear();
    m_dirtyElementIndexSet.clear();
}
```

---

## 動作の確認

このステップ単体では何も描画は変わらない(S3 で使い始める)。
S3 のオブジェクトのテーブルで、値を書いたフレームだけ GPU のコピーが走ることを、PIX や S1 のプロファイラーで確かめる。

## 次のステップへのつながり

- S3 : `ModelRenderSystem` がこのクラスを2つ持つ(オブジェクトのテーブル / メッシュのテーブル)。
- S4 : マテリアルの種類ごとに1つ持つ(StandardLit / StandardUnLit、後にトゥーン)。
