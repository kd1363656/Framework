# S6 ExecuteIndirect(描画を GPU のバッファからまとめて投げる)

> **2026-10-10 改訂** : コマンドシグネチャを一覧ごとではなく、ModelRenderSystem が「RCModelDrawItem を持つルートシグネチャ」ごとに1つ持つ形にした。
> 改訂前は「影のパスか」で ModelStandard / ModelCascadeShadow のルートシグネチャを選ぶ分岐があり、ルートシグネチャ(トゥーンなど)を足すたびに書き足す必要があった。
> 今は Renderer のルートシグネチャを全部見て、RCModelDrawItem を持つものに自動で作る。パスは自分のルートシグネチャを渡すだけで、合うコマンドシグネチャが選ばれる。
>
> **2026-10-10 確認** : S3 でテーブルのコピーを RenderGraph のパス(`ModelRenderTableUploadPass`、実行レイヤー `Upload`)にしたが、S6 は変更なし。
> 引数のバッファは UPLOAD ヒープ(バリア不要)で `RecordDraw` の中で写すため、RenderGraph が扱うものは増えない(S3 の「S5 / S6 / P1 への影響」)。

## 目的

S5 では、パスが描画項目を CPU で1つずつ回し、`SetGraphicsRoot32BitConstants` と `DispatchMesh` をコマンドリストへ積んでいる。
項目が 1 万あれば、毎フレーム 2 万回の API 呼び出しになる。

ExecuteIndirect は、**「描画の引数を並べたバッファ」を GPU に渡し、GPU がそれを読みながら描画を繰り返す**命令。

- パスの中の命令は「ExecuteIndirect を 1 回」だけになる(項目の数に関係なく、CPU の仕事は一定)。
- 引数のバッファは、描画項目の一覧が変わったフレームだけ書き直す(S5 の一覧をそのまま memcpy)。

### 数値の例(1 万項目、Lit + 影 3 枚 = 4 回)

| | S5 | S6 |
|---|---|---|
| 毎フレームの API 呼び出し | 4 × 1 万 × 2 = **8 万回** | 4 × 1 = **4 回**(+ ルート定数の設定数回) |
| 一覧が変わらないフレームの CPU のコピー | 0 | 0 |
| 一覧が変わったフレーム | (CPU の一覧を詰め直すだけ) | + 24 バイト × 1 万 = 240KB を UPLOAD へ memcpy(変わったフレームだけ) |

> **この後のフェーズ2 P1(GPU でのカリング)の土台**でもある。GPU のコンピュートシェーダーが「見える項目だけ」を引数のバッファへ詰め、
> その数をカウントバッファに書けば、CPU は一切触らずに、見えるものだけが描かれる。

## DirectX12 の解説

### ExecuteIndirect

```cpp
// ExecuteIndirect(コマンドシグネチャ(1件の中身の説明書)、
//                 最大で何件実行するか、
//                 引数のバッファ、
//                 引数のバッファの何バイト目から読むか、
//                 件数のバッファ(使わないならnullptr)、
//                 件数のバッファの何バイト目か);
commandList->ExecuteIndirect(commandSignature, maxCommandCount, argumentBuffer, argumentBufferOffset, countBuffer, countBufferOffset);
```

- GPU は引数のバッファから「1件ぶんのバイト列」を読み、そのとおりにルート定数を設定して `DispatchMesh` する、を件数ぶん繰り返す。
- 件数のバッファ(カウントバッファ)を渡すと、実際の件数を GPU のバッファから読む(フェーズ2 P1 の GPU カリングで使う)。今回は nullptr で、最大件数ぶん全部実行する。

### コマンドシグネチャ(ID3D12CommandSignature)

「引数のバッファの1件が、どんな中身か」の説明書。このエンジンでは次の2つを並べる。

| 順番 | 種類(`D3D12_INDIRECT_ARGUMENT_TYPE_...`) | 中身 | 大きさ |
|---|---|---|---|
| 1 | `CONSTANT` | ルート定数 `RCModelDrawItem`(オブジェクト・メッシュ・マテリアルの番号) | 4 × 3 = 12 バイト |
| 2 | `DISPATCH_MESH` | `D3D12_DISPATCH_MESH_ARGUMENTS`(X / Y / Z) | 4 × 3 = 12 バイト |

- 1件の大きさ(`ByteStride`) = 24 バイト。S3 の `Struct::ModelDrawItem` と同じ並び・同じ大きさにしてある。
- ルート定数(ルートシグネチャの中身)を書き換える種類を含むときは、**作るときにルートシグネチャを渡す必要がある**(どのルートパラメーターの番号に書くかを決めるため)。
  そのため、ルートシグネチャごとに別々のコマンドシグネチャを作る。ModelRenderSystem が、RCModelDrawItem を持つルートシグネチャ(今は ModelStandard と ModelCascadeShadow)を探して、自動で1つずつ作る。
- 引数のバッファ(1件の並び)は、どのコマンドシグネチャでも同じ。そのため、Lit のパスと影のパスが同じ描画項目の一覧(引数のバッファ)を共有できる。

```
引数のバッファ(1件 24 バイト)
[ obj 0 | mesh 3 | mat 5 | X 4 | Y 1 | Z 1 ][ obj 0 | mesh 4 | mat 5 | X 2 | Y 1 | Z 1 ][ obj 1 | ... ]
   └ CONSTANT(12バイト)─┘└ DISPATCH_MESH(12バイト)┘
```

### 引数のバッファを UPLOAD ヒープに置く理由

- GPU は、引数のバッファを `D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT` の状態で読む。
- UPLOAD ヒープのリソースは、ずっと `GENERIC_READ` のまま(状態を変えられない)。`GENERIC_READ` には `INDIRECT_ARGUMENT` が含まれるので、**バリア無しでそのまま読める**。
- 一覧が変わるのはまれで、1 件 24 バイトと小さいので、DEFAULT ヒープへコピーし直す手間をかけるより、UPLOAD から直接読む方が簡単で十分に速い。
- フレームごとに別の UPLOAD を持つ(S2 と同じ理由。GPU がまだ読んでいる引数を、次のフレームの値で上書きしないため)。

### ExecuteIndirect の後のルート定数

- ExecuteIndirect が書き換えたルート定数(`RCModelDrawItem`)は、実行が終わった後は「最後の1件の値」になっている(どの値かは決まっていない、と考える)。
- その後に普通の描画で `RCModelDrawItem` を使うなら、自分で設定し直す必要がある(このエンジンでは、モデルの描画はすべて ExecuteIndirect なので問題ない)。

## ファイル一覧

### 新規

| ファイル | 内容 |
|---|---|
| `Graphics/Render/Model/DrawItem/ModelDrawCommandSignature.h/.cpp` | コマンドシグネチャ |

### 変更

| ファイル | 変更 |
|---|---|
| `Graphics/Command/List/Direct/DirectCommandList.h/.cpp` | `ExecuteIndirect` |
| `Graphics/Render/Model/DrawItem/ModelDrawItemList.h/.cpp` | フレームごとの引数のバッファ。`RecordDraw` でコマンドシグネチャを受け取り、ExecuteIndirect にする |
| `Graphics/Render/Model/ModelRenderSystem.h/.cpp` | `Create` に Renderer のルートシグネチャの map を渡し、RCModelDrawItem を持つものごとにコマンドシグネチャを作る。描くときに選んで渡す |
| `Graphics/Render/Renderer.cpp` | `m_modelRenderSystem.Create` を、ルートシグネチャを作った後へ移す |

### 登録

- Framework.h: `ModelDrawCommandSignature.h` を `ModelDrawItemList.h` の前。

---

## コード

### Graphics/Command/List/Direct/DirectCommandList.h(変更)

`DispatchMesh` の後ろに足す。

```cpp
        void DispatchMesh(const UINT a_threadCountGroupX, const UINT a_threadCountGroupY, const UINT a_threadCountGroupZ) const;

        void ExecuteIndirect(const TypeAlias::ComPtr<ID3D12CommandSignature>& a_commandSignature,
                             const TypeAlias::ComPtr<ID3D12Resource2>&        a_argumentBuffer,
                             const UINT64&                                    a_argumentBufferOffset,
                             const UINT                                       a_maxCommandCount) const;
```

### Graphics/Command/List/Direct/DirectCommandList.cpp(変更・写経)

```cpp
void FWK::Graphics::DirectCommandList::ExecuteIndirect(const TypeAlias::ComPtr<ID3D12CommandSignature>& a_commandSignature,
                                                       const TypeAlias::ComPtr<ID3D12Resource2>&        a_argumentBuffer,
                                                       const UINT64&                                    a_argumentBufferOffset,
                                                       const UINT                                       a_maxCommandCount) const
{
    const auto& l_directCommandList = GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、ExecuteIndirectに失敗しました。");
    FWK_ASSERT_RETURN_IF(!a_commandSignature,  "コマンドシグネチャが無効のため、ExecuteIndirectに失敗しました。");
    FWK_ASSERT_RETURN_IF(!a_argumentBuffer,    "引数のバッファが無効のため、ExecuteIndirectに失敗しました。");

    // 引数のバッファに並んだ「描画1回ぶんの引数」を、GPUが読みながら描画を繰り返す命令を積む
    // CPUが積む命令はこの1つだけで、何件描くかはa_maxCommandCount(と、件数のバッファがあればその値)で決まる
    // ExecuteIndirect(コマンドシグネチャ(1件の中身の説明書)、
    //                 最大で何件実行するか、
    //                 引数のバッファ、
    //                 引数のバッファの何バイト目から読むか、
    //                 件数のバッファ(今回は使わないのでnullptr)、
    //                 件数のバッファの何バイト目か(使わないので0));
    l_directCommandList->ExecuteIndirect(a_commandSignature.Get(),
                                         a_maxCommandCount,
                                         a_argumentBuffer.Get(),
                                         a_argumentBufferOffset,
                                         nullptr,
                                         k_unusedCountBufferOffset);
}
```

> `k_unusedCountBufferOffset` は `.h` の private に `static constexpr UINT64 k_unusedCountBufferOffset = 0ULL;` を足す。

### Graphics/Render/Model/DrawItem/ModelDrawCommandSignature.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelDrawCommandSignature final
    {
    public:

         ModelDrawCommandSignature() = default;
        ~ModelDrawCommandSignature() = default;

        ModelDrawCommandSignature(const ModelDrawCommandSignature&)  = delete;
        ModelDrawCommandSignature(      ModelDrawCommandSignature&&) = delete;

        ModelDrawCommandSignature& operator=(const ModelDrawCommandSignature&)  = delete;
        ModelDrawCommandSignature& operator=(      ModelDrawCommandSignature&&) = delete;

        bool Create(const Device& a_device, const RootSignature& a_rootSignature);

        const auto& GetREFCommandSignature() const { return m_commandSignature; }

    private:

        static constexpr std::size_t k_argumentDescCount = 2ULL;
        static constexpr std::size_t k_constantArgumentIndex     = 0ULL;
        static constexpr std::size_t k_dispatchMeshArgumentIndex = 1ULL;

        static constexpr UINT k_rootConstantDestinationOffset = 0U;
        static constexpr UINT k_defaultNodeMask               = 0U;

        TypeAlias::ComPtr<ID3D12CommandSignature> m_commandSignature = nullptr;
    };
}
```

### Graphics/Render/Model/DrawItem/ModelDrawCommandSignature.cpp(新規・写経)

```cpp
#include "ModelDrawCommandSignature.h"

// ExecuteIndirectで使う「引数のバッファの1件の中身の説明書」(コマンドシグネチャ)を作るクラス
// 1件 = ルート定数RCModelDrawItem(12バイト) + DispatchMeshの引数(12バイト) = 24バイト
// Struct::ModelDrawItemと同じ並び・同じ大きさにしてあるため、描画項目の一覧をそのまま引数のバッファへ写せる
bool FWK::Graphics::ModelDrawCommandSignature::Create(const Device& a_device, const RootSignature& a_rootSignature)
{
    // 描画項目の構造体に値を足して大きさがずれたら、ここでコンパイルエラーにして気づけるようにする
    // (ずれたまま動かすと、GPUが1件の区切りを間違えて読み、壊れた描画やデバイスの消失になる)
    static_assert(sizeof(Struct::ModelDrawItem) == sizeof(Struct::RCModelDrawItem) + sizeof(D3D12_DISPATCH_MESH_ARGUMENTS), "Struct::ModelDrawItemの大きさが、コマンドシグネチャの1件の大きさと一致していません。");

    const auto& l_device           = a_device.GetREFDevice              ();
    const auto& l_d3dRootSignature = a_rootSignature.GetREFRootSignature();

    FWK_ASSERT_RETURN_VALUE_IF(!l_device,           "デバイスが作成されておらず、コマンドシグネチャの作成に失敗しました。",           false);
    FWK_ASSERT_RETURN_VALUE_IF(!l_d3dRootSignature, "ルートシグネチャが作成されておらず、コマンドシグネチャの作成に失敗しました。", false);

    // ルートシグネチャの中で、RCModelDrawItem(描画ごとの番号)のルートパラメーターが何番目かを探す
    // ModelStandardなら1番、ModelCascadeShadowなら1番(GraphicsCONFIG.jsonのRootParameterIndexMap)
    const auto& l_rootParameterIndex = a_rootSignature.FindVALRootParameterIndex(Enum::RootParameterType::RCModelDrawItem);

    FWK_ASSERT_RETURN_VALUE_IF(l_rootParameterIndex == Converter::RootSignatureJsonConverter::k_invalidRootParameterIndex, "RCModelDrawItemのルートパラメーターが無いため、コマンドシグネチャの作成に失敗しました。", false);

    std::array<D3D12_INDIRECT_ARGUMENT_DESC, k_argumentDescCount> l_argumentDescList = {};

    // 1つ目 : ルート定数を書き換える
    // Type                             : ルート定数(32ビットの値をいくつか)を書き換える種類
    // Constant.RootParameterIndex      : どのルートパラメーターへ書くか
    // Constant.DestOffsetIn32BitValues : そのルート定数の何番目の値から書くか(先頭から)
    // Constant.Num32BitValuesToSet     : 何個の値を書くか(RCModelDrawItemはuint × 3)
    auto& l_constantArgumentDesc = l_argumentDescList[k_constantArgumentIndex];

    l_constantArgumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;

    l_constantArgumentDesc.Constant.RootParameterIndex = l_rootParameterIndex;

    l_constantArgumentDesc.Constant.DestOffsetIn32BitValues = k_rootConstantDestinationOffset;

    l_constantArgumentDesc.Constant.Num32BitValuesToSet = static_cast<UINT>(sizeof(Struct::RCModelDrawItem) / sizeof(std::uint32_t));

    // 2つ目 : DispatchMeshを実行する
    // 引数(D3D12_DISPATCH_MESH_ARGUMENTS = X / Y / Z)は、引数のバッファの中の値を使う
    auto& l_dispatchMeshArgumentDesc = l_argumentDescList[k_dispatchMeshArgumentIndex];

    l_dispatchMeshArgumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH;

    D3D12_COMMAND_SIGNATURE_DESC l_commandSignatureDesc = {};

    // 1件の大きさ(次の1件まで何バイト進むか)
    // Struct::ModelDrawItemの大きさ(24バイト)と必ず同じにする
    l_commandSignatureDesc.ByteStride = static_cast<UINT>(sizeof(Struct::ModelDrawItem));

    // 1件の中身の種類の数と、その配列
    l_commandSignatureDesc.NumArgumentDescs = static_cast<UINT>(l_argumentDescList.size());

    l_commandSignatureDesc.pArgumentDescs = l_argumentDescList.data();

    // どのGPUで使うか(GPUが1つなら0)
    l_commandSignatureDesc.NodeMask = k_defaultNodeMask;

    // CreateCommandSignature(コマンドシグネチャの設定、
    //                        ルートシグネチャ(ルート定数を書き換えるときは必須)、
    //                        受け取りたいCOMインターフェース型のID、
    //                        作成結果のポインタを書き込むアドレス);
    const auto& l_hr = l_device->CreateCommandSignature(&l_commandSignatureDesc,
                                                        l_d3dRootSignature.Get(),
                                                        IID_PPV_ARGS(m_commandSignature.ReleaseAndGetAddressOf()));

    FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "コマンドシグネチャの作成に失敗しました。", false);

    return true;
}
```


### Graphics/Render/Model/DrawItem/ModelDrawItemList.h(変更)

```cpp
        bool Create(const Device& a_device, const std::size_t& a_frameCount);

        void RecordDraw(const ModelDrawCommandSignature& a_commandSignature, const DirectCommandList& a_directCommandList, const std::size_t& a_frameIndex);
```

private:

```cpp
        void RebuildFrameDrawItemListIfNeeded();

        bool UploadArgumentBufferIfNeeded(const std::size_t& a_frameIndex);

        static constexpr std::size_t k_emptyFrameCount = 0ULL;
        static constexpr std::size_t k_noErasedCount   = 0ULL;

        static constexpr std::uint64_t k_firstRegistrationID = 0ULL;
        static constexpr std::uint64_t k_initialVersion      = 0ULL;

        static constexpr UINT64 k_argumentBufferOffset = 0ULL;

        static constexpr std::size_t k_initialArgumentBufferCapacity = 1024ULL;
        static constexpr std::size_t k_argumentBufferGrowthRate      = 2ULL;

        FrameDrawItemList m_frameDrawItemList;

        std::vector<UploadBuffer> m_argumentBufferList;

        std::vector<std::size_t> m_argumentBufferCapacityList;

        std::vector<std::uint64_t> m_uploadedVersionList;

        RegistrationMap m_registrationMap;

        std::size_t m_frameCount;

        std::uint64_t m_nextRegistrationID;
        std::uint64_t m_version;

        bool m_isDirty;
```

> - `RecordDraw` からルートシグネチャの引数が消え、代わりにコマンドシグネチャを受け取る。
> - コマンドシグネチャは一覧には持たせない。一覧は「メッシュの種類 × マテリアル」の数だけあり(マテリアルを足すと増える)、
>   影のパスは同じ一覧を別のルートシグネチャ(ModelCascadeShadow)で描くので、一覧ごとに1つでは足りないため。
>   ModelRenderSystem が「ルートシグネチャごとに1つ」持ち、描くときに渡す(下の ModelRenderSystem)。

### Graphics/Render/Model/DrawItem/ModelDrawItemList.cpp(変更・写経)

コンストラクタの初期化子リストに、足したメンバ(`m_argumentBufferList()` / `m_argumentBufferCapacityList()` / `m_uploadedVersionList()` / `m_version(k_initialVersion)`)を宣言順に足す。

```cpp
bool FWK::Graphics::ModelDrawItemList::Create(const Device& a_device, const std::size_t& a_frameCount)
{
    FWK_ASSERT_RETURN_VALUE_IF(a_frameCount == k_emptyFrameCount, "フレーム数が0のため、ModelDrawItemListの作成に失敗しました。", false);

    m_frameCount = a_frameCount;

    m_frameDrawItemList.resize(a_frameCount);

    // 引数のバッファ(UPLOADヒープ)を、フレームの数だけ用意する
    // 最初は1024件(24KB)ぶんを作り、足りなくなったら2倍にして作り直す
    m_argumentBufferList.resize        (a_frameCount);
    m_argumentBufferCapacityList.resize(a_frameCount, k_initialArgumentBufferCapacity);
    m_uploadedVersionList.resize       (a_frameCount, k_initialVersion);

    const auto& l_argumentBufferSize = static_cast<UINT64>(k_initialArgumentBufferCapacity * sizeof(Struct::ModelDrawItem));

    for (auto& l_argumentBuffer : m_argumentBufferList)
    {
        FWK_ASSERT_RETURN_VALUE_IF(!l_argumentBuffer.Create(a_device, l_argumentBufferSize), "引数のバッファの作成に失敗したため、ModelDrawItemListの作成に失敗しました。", false);
    }

    return true;
}

void FWK::Graphics::ModelDrawItemList::RecordDraw(const ModelDrawCommandSignature& a_commandSignature, const DirectCommandList& a_directCommandList, const std::size_t& a_frameIndex)
{
    // 登録・解除があったら、CPU側の一覧を詰め直す
    RebuildFrameDrawItemListIfNeeded();

    FWK_ASSERT_RETURN_IF(a_frameIndex >= m_frameDrawItemList.size(), "フレームの番号が範囲外のため、描画項目の描画に失敗しました。");

    const auto& l_drawItemList = m_frameDrawItemList[a_frameIndex];

    // 描くものが無ければ、何もしない
    if (l_drawItemList.empty()) { return; }

    // このフレームの引数のバッファが古ければ(一覧が変わった後、まだ写していなければ)写す
    if (!UploadArgumentBufferIfNeeded(a_frameIndex)) { return; }

    const auto& l_argumentBuffer = m_argumentBufferList[a_frameIndex];

    // 一覧の件数だけ、GPUに描画を繰り返させる
    // 例 : 500件なら、CPUが積む命令はこの1つだけで、GPUが500回「ルート定数を設定してDispatchMesh」を行う
    // コマンドシグネチャは、今のパスのルートシグネチャ用のもの(ModelRenderSystemが選んで渡す)
    a_directCommandList.ExecuteIndirect(a_commandSignature.GetREFCommandSignature(),
                                        l_argumentBuffer.GetREFUploadBuffer      (),
                                        k_argumentBufferOffset,
                                        static_cast<UINT>(l_drawItemList.size()));
}
```

`Register` / `Unregister` で `m_isDirty = true;` の隣に `++m_version;` を足す(一覧の「版」を進める)。

```cpp
bool FWK::Graphics::ModelDrawItemList::UploadArgumentBufferIfNeeded(const std::size_t& a_frameIndex)
{
    // このフレームの引数のバッファに写した一覧の版が、今の版と同じなら、写し直さない
    // 一覧が変わらない限り、毎フレームここで終わる
    if (m_uploadedVersionList[a_frameIndex] == m_version) { return true; }

    const auto& l_drawItemList   = m_frameDrawItemList[a_frameIndex];
          auto& l_argumentBuffer = m_argumentBufferList[a_frameIndex];
          auto& l_capacity       = m_argumentBufferCapacityList[a_frameIndex];

    // 件数が容量を超えたら、収まるまで2倍にして作り直す
    // 例 : 容量1024で1500件なら、2048件(48KB)のバッファを作り直す
    // このフレームの引数のバッファは、前回このフレームリソースを使ったときの命令がGPUで終わっている(フェンスで待った後)ので、すぐに作り直してよい
    if (l_drawItemList.size() > l_capacity)
    {
        while (l_capacity < l_drawItemList.size())
        {
            l_capacity *= k_argumentBufferGrowthRate;
        }

        const auto& l_graphicsManager    = GraphicsManager::GetInstance  ();
        const auto& l_device             = l_graphicsManager.GetREFDevice();
        const auto& l_argumentBufferSize = static_cast<UINT64>           (l_capacity * sizeof(Struct::ModelDrawItem));

        FWK_ASSERT_RETURN_VALUE_IF(!l_argumentBuffer.Create(l_device, l_argumentBufferSize), "引数のバッファの作り直しに失敗しました。", false);
    }

    auto* l_mappedData = l_argumentBuffer.FetchPTRMappedData();

    FWK_ASSERT_RETURN_VALUE_IF(!l_mappedData, "引数のバッファのMap済みデータが無効のため、引数のバッファへの書き込みに失敗しました。", false);

    // Struct::ModelDrawItemは、コマンドシグネチャの1件と同じ並び(24バイト)なので、変換せずにそのまま写せる
    std::memcpy(l_mappedData, l_drawItemList.data(), l_drawItemList.size() * sizeof(Struct::ModelDrawItem));

    m_uploadedVersionList[a_frameIndex] = m_version;

    return true;
}
```

> `UploadBuffer::Create` は中で `Release()` してから作り直すので、古いバッファは自動で手放される。

### Graphics/Render/Model/ModelRenderSystem.h(変更)

```cpp
    private:

        using RootSignatureMap = std::unordered_map<Enum::RootSignatureType, std::shared_ptr<RootSignature>>;

        using ModelRenderTableMap = std::unordered_map<TypeAlias::StaticTypeID, std::shared_ptr<GPUElementTable>>;

        using ModelDrawItemListMap         = std::unordered_map<TypeAlias::StaticTypeID, std::shared_ptr<ModelDrawItemList>>;
        using ModelMeshTypeDrawItemListMap = std::unordered_map<Enum::ModelMeshType, ModelDrawItemListMap>;

        using ModelDrawCommandSignatureMap = std::unordered_map<const RootSignature*, ModelDrawCommandSignature>;

    public:
        ...

        bool Create(const RootSignatureMap&                   a_rootSignatureMap,
                    const Device&                             a_device,
                    const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
                    const std::size_t&                        a_frameCount,
                          TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);
        ...

    private:

        void RecordMaterialDraw(const RootSignature&          a_rootSignature,
                                const DirectCommandList&      a_directCommandList,
                                const std::size_t&            a_frameIndex,
                                const Enum::ModelMeshType     a_meshType,
                                const TypeAlias::StaticTypeID a_materialTableStaticTypeID) const;

        const ModelDrawCommandSignature* FindPTRCommandSignature(const RootSignature& a_rootSignature) const;

        ModelRenderTableMap          m_tableMap                = {};
        ModelMeshTypeDrawItemListMap m_meshTypeDrawItemListMap = {};
        ModelDrawCommandSignatureMap m_commandSignatureMap     = {};

        Converter::ModelRenderSystemJsonConverter m_jsonConverter = {};
```

> - `RootSignatureMap` は Renderer の `m_rootSignatureMap` と同じ型(Renderer の別名は private なので、こちらにも同じ別名を置く)。
>   引数の並びは「配列(map)→ 自作クラス → 数値 → 参照」(規約 20-4)。
> - コマンドシグネチャの map のキーは、ルートシグネチャの**アドレス**(`const RootSignature*`)。
>   パスは `SetupGraphicsRenderPipeline` で受け取ったルートシグネチャ(の参照)を渡してくるので、そのアドレスでそのまま探せる。
>   ルートシグネチャの実体は Renderer の map が `shared_ptr` でアプリの終了まで持つので、アドレスは変わらない。
> - `ModelDrawCommandSignature` はコピー・ムーブ禁止。`std::unordered_map` は要素を動かさない(再ハッシュでも要素の場所は変わらない)ので、
>   `try_emplace(キー)` で map の中に直接作れば、そのまま入れられる。

### Graphics/Render/Model/ModelRenderSystem.cpp(変更・写経)

**Create の先頭(テーブルを作るループの前)に追加:**

```cpp
    // ExecuteIndirectのコマンドシグネチャを、RCModelDrawItem(描画ごとの番号)を持つルートシグネチャごとに1つ作る
    // コマンドシグネチャは「ルート定数を、どのルートパラメーターの番号へ書くか」を覚えるため、ルートシグネチャごとに別のものが要る
    // 例 : ModelStandard(Lit / UnLitのパス)と ModelCascadeShadow(影のパス)の2つ
    // モデルを描くルートシグネチャを増やしても(トゥーンなど)、GraphicsCONFIG.jsonにRCModelDrawItemを書けば、ここで自動で作られる
    for (const auto& [l_rootSignatureType, l_rootSignature] : a_rootSignatureMap)
    {
        if (!l_rootSignature) { continue; }

        // RCModelDrawItemを持たないルートシグネチャ(スプライトなど)には作らない
        const auto l_rootParameterIndex = l_rootSignature->FindVALRootParameterIndex(Enum::RootParameterType::RCModelDrawItem);

        if (l_rootParameterIndex == Converter::RootSignatureJsonConverter::k_invalidRootParameterIndex) { continue; }

        // try_emplaceにキーだけを渡すと、値(コマンドシグネチャ)をmapの中で直接作る(コピー・ムーブ禁止のクラスでも入れられる)
        // 戻り値の.firstは、入れた要素(既にあればその要素)を指すイテレータ
        const auto  l_commandSignatureITR = m_commandSignatureMap.try_emplace(l_rootSignature.get()).first;
              auto& l_commandSignature    = l_commandSignatureITR->second;

        FWK_ASSERT_RETURN_VALUE_IF(!l_commandSignature.Create(a_device, *l_rootSignature), "コマンドシグネチャの作成に失敗したため、ModelRenderSystemの作成に失敗しました。", false);
    }
```

> `l_rootSignature.get()` : `shared_ptr` が持っている実体のアドレス(生のポインタ)を返す。所有はしないので、map のキーに使うだけなら問題ない。

**Create の描画項目の一覧を作るループ(S5 で、テーブルを作るループの後に足したもの)** : `l_drawItemList->Create(a_frameCount)` を `l_drawItemList->Create(a_device, a_frameCount)` にする(引数のバッファを作るため)。

**RecordDrawWithoutMaterial / RecordMaterialDraw(変更・写経)** : どちらも、`SetupRoot32BitConstants` の前にコマンドシグネチャを探し、最後に渡す。

```cpp
    // このパスのルートシグネチャ用のコマンドシグネチャ(Createで作ったもの)を探す
    // パスがセットしたルートシグネチャと違うコマンドシグネチャを使うと、ルート定数が違う番号へ書かれてしまう
    const auto* l_commandSignature = FindPTRCommandSignature(a_rootSignature);

    FWK_ASSERT_RETURN_IF(!l_commandSignature, "ルートシグネチャにRCModelDrawItemが無いため、モデルの描画に失敗しました。");
```

```cpp
    // RecordDrawWithoutMaterialのループの中
    l_drawItemList->RecordDraw(*l_commandSignature, a_directCommandList, a_frameIndex);
```

```cpp
    // RecordMaterialDrawの最後
    l_drawItemList->RecordDraw(*l_commandSignature, a_directCommandList, a_frameIndex);
```

**FindPTRCommandSignature(写経):**

```cpp
const FWK::Graphics::ModelDrawCommandSignature* FWK::Graphics::ModelRenderSystem::FindPTRCommandSignature(const RootSignature& a_rootSignature) const
{
    // パスが今セットしたルートシグネチャのアドレスで、そのルートシグネチャ用のコマンドシグネチャを探す
    // 見つからなければ(RCModelDrawItemを持たないルートシグネチャなら)nullptr
    const auto& l_commandSignatureITR = m_commandSignatureMap.find(&a_rootSignature);

    if (l_commandSignatureITR == m_commandSignatureMap.end()) { return nullptr; }

    return &l_commandSignatureITR->second;
}
```

> 影のパス(ModelCascadeShadow のルートシグネチャ)と Lit のパス(ModelStandard)は、同じ「Static × StandardLit」の一覧を、
> それぞれのコマンドシグネチャで ExecuteIndirect する。引数のバッファ(24 バイト × 件数)は一覧に1つだけで、共有できる
> (コマンドシグネチャが違っても、1件の並びは同じ「ルート定数 3 つ + DispatchMesh の引数」なので)。

### Graphics/Render/Renderer.cpp(変更・写経)

`m_modelRenderSystem.Create` を、ルートシグネチャの作成ループの後(パイプラインステートの作成の前でよい)へ移す。

```cpp
    // モデルの描画に使うテーブル・描画項目の一覧・ExecuteIndirectのコマンドシグネチャを作る
    // コマンドシグネチャはルートシグネチャから作るため、ルートシグネチャを作った後に行う
    // どのルートシグネチャに作るかは、ModelRenderSystemがRCModelDrawItemの有無で決める(ここで種類を選ばない)
    const bool l_isModelRenderSystemCreated = m_modelRenderSystem.Create(m_rootSignatureMap,
                                                                         a_device,
                                                                         l_gpuMemoryAllocator,
                                                                         m_frameResourceList.size(),
                                                                         l_cbvSRVUAVDescriptorPool);

    FWK_ASSERT_RETURN_VALUE_IF(!l_isModelRenderSystemCreated, "ModelRenderSystemの作成処理に失敗しました。", false);
```

---

## ビルドと動作の確認(ここでこちらがビルドする)

1. MSBuild(VS 18、Debug | x64)でビルドし、S0 ~ S6 のコンパイルエラーを直す。
2. checker の `runall.py` / `filtercheck.py` で規約とフィルターを確認する。
3. ユーザーに起動してもらい、次を確かめる。
   - モデル(Static / Skeletal)・影・マテリアルのスロットが、S5 の確認項目どおりに動く。
   - D3D12 のデバッグレイヤーのエラー(出力ウィンドウの `D3D12 ERROR`)が出ない。
   - プロファイラーの CPU「描画命令の記録」が、モデルの数を増やしてもほとんど増えない。
   - GPU の各パスの時間を、S1 の時点の数字と比べる。

## 次(フェーズ2)

`P_Phase2.md` の P1(GPU でのカリング)から。ExecuteIndirect のカウントバッファを使い、見えるものだけを GPU が引数のバッファへ詰める。
