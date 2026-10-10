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
                                            const UINT                                a_capacity,
                                            const UINT                                a_elementByteStride,
                                                  TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
    const auto& l_currentBufferGPUResource = GetREFBufferGPUResource();

    FWK_ASSERT_RETURN_VALUE_IF(a_frameCount == k_emptyFrameCount,                                      "フレーム数が0のため、GPUElementTableの作成に失敗しました。",                      false);
    FWK_ASSERT_RETURN_VALUE_IF(a_capacity == k_initialCapacity,                                        "容量が0のため、GPUElementTableの作成に失敗しました。",                            false);
    FWK_ASSERT_RETURN_VALUE_IF(a_elementByteStride == k_initialElementByteStride,                      "1要素の大きさが0のため、GPUElementTableの作成に失敗しました。",                   false);
    FWK_ASSERT_RETURN_VALUE_IF(l_currentBufferGPUResource.m_resource,                                  "既にGPUResourceを保持しているため、GPUElementTableの作成に失敗しました。",        false);
    FWK_ASSERT_RETURN_VALUE_IF(GetVALSRVDescriptorIndex() != DescriptorHeap::k_invalidDescriptorIndex, "既にSRVDescriptorIndexを保持しているため、GPUElementTableの作成に失敗しました。", false);

    // テーブル全体の大きさ = 容量 × 1要素の大きさ
    // 例 : 4096 × 136 = 557,056バイト
    const auto& l_bufferSize = static_cast<UINT64>(a_capacity) * static_cast<UINT64>(a_elementByteStride);

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
                                                 a_capacity,
                                                 a_elementByteStride,
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

    m_capacity          = a_capacity;
    m_elementByteStride = a_elementByteStride;

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

    const auto& l_bufferGPUResource = GetREFBufferGPUResource();
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
    // 既に解放するものがなければreturn
    if (const auto& l_bufferGPUResource = GetREFBufferGPUResource();
        !l_bufferGPUResource.m_resource &&
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