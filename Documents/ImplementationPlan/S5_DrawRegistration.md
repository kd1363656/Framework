# S5 描画の登録(状態が変わった時だけ描画項目を作る)

> **2026-10-10 改訂** : 描画項目の一覧を「パスの種類(`Enum::ModelRenderPassType`)」ではなく、
> 「メッシュの種類(`Enum::ModelMeshType`)× マテリアルのテーブル(GPU データの型の StaticTypeID)」で分ける形にした。
> 一覧は、GraphicsCONFIG.json の TableMap に書いたマテリアルの数だけ自動で作られ、描き方の「マテリアル → パス」の switch も無くなった(下の「改訂前との比較」)。

> **2026-10-11 改訂(ModelComponent を設計から外した)** : ユーザー指示「ModelComponent はまだ実装しないので設計に加えないで、S4 ~ S6 を実装する」。
> - 描画項目を作って一覧へ登録・解除する処理を、ModelComponent の描き方(`GameObjectModelComponentRendererBase` と Static / Skeletal)から、
>   Graphics の新しいクラス **`ModelDrawRegistration`**(`Graphics/Render/Model/DrawItem/`)へ移した。S5 では ModelComponent のファイルを1つも触らない。
> - `Struct::ModelDrawRegistration`(一覧 + 登録番号)は `ModelDrawRegistration` の private の入れ子 `DrawItemListRegistration` にし、
>   無効な登録番号の定数は `ModelDrawItemList::k_invalidRegistrationID`(public)にした。
> - `Struct::ModelDrawMaterial` は廃止。`ModelDrawRegistration::Register` がマテリアルのハンドル(`Graphics::ModelMaterial`)をメッシュの数だけ受け取る。
> - 描画項目を出す側(ModelComponent)が無いので、S6 のビルドの後もモデルはまだ描かれない。描かれるのは S0 の後。

> **2026-10-11 改訂(ユーザー指示)**
> - 「TypeINFO でもポインタならすべて nullptr を確認する」: `k_typeINFO->` の前に必ず確認する(テンプレート・`BuildMaterialFrameDrawItemListMap`)。
> - 「コンストラクタかデストラクタのどちらかを .cpp に定義するなら、メンバは初期化子リストで初期化する」: `ModelDrawRegistration` のコンストラクタを .cpp へ。
> - 「メッシュの種類が Static / Skeletal しかないなら std::array で持つ。Invalid は消す」: `Enum::ModelMeshType` は `Static = 0 / Skeletal = 1 / Count`、
>   ModelRenderSystem の一覧は `std::array<マテリアル → 一覧の map, 2>`(`m_meshTypeDrawItemListMapList`)。添字の範囲の確認は `FindPTRDrawItemListMap` の1か所。

## 目的

今の描画パスは、毎フレーム、描画申請(オブジェクト)→ メッシュの二重ループで、オブジェクトの行列から CB を作っている。
S3・S4 で、行列・メッシュ・マテリアルはテーブルに置いたので、描画に必要なのは**番号3つと AS のグループ数だけ**になった。

そこで、

- `ModelDrawRegistration`(1体のモデルの登録をまとめて持つクラス)が、**モデル・マテリアルが決まったとき(状態が変わったときだけ)**、「描画項目」(`Struct::ModelDrawItem`)を作って、メッシュの種類 × マテリアルの種類ごとの一覧へ登録する。
- パスは一覧を先頭から回して、`SetGraphicsRoot32BitConstants`(番号3つ)と `DispatchMesh` を積むだけにする。
- 毎フレームの行列の計算・CB の書き込み・weak_ptr の lock・マテリアルの種類の判定は、パスからすべて無くなる。

### 「lock は必須」(以前の指示)との関係

- 以前の合意 : 「参照が消えたら自動で描画リストから外れるように、lock は必須」。
- この形では、**描画項目の一覧を `shared_ptr` で ModelRenderSystem が持ち、登録した側(`ModelDrawRegistration`)は `weak_ptr` + 登録番号を持つ**。
  `ModelDrawRegistration` が破棄されるとき(デストラクタ)に `lock()` して登録を外すので、「参照が消えたら自動で外れる」は守られる。
- 「毎フレーム、全項目を lock して確かめる」はしない。S6 の ExecuteIndirect では CPU が項目を1つずつ回さない(GPU がバッファから読む)ため、
  毎フレームの lock を残すと、ExecuteIndirect の意味が無くなる(1万項目なら毎フレーム1万回の lock が残る)。
- Undo で生き続けるコンポーネントの扱い(Attach / Detach で `Register` / `Unregister` を呼ぶ)は、S0 で ModelComponent を作るときに決める。

### 数値の例(100 体 × 5 メッシュ、Lit + 影 3 枚)

| | S2 より前 | S5 の後 |
|---|---|---|
| 毎フレームの CPU の処理(パスの中) | 2000 回 × (行列の逆行列・CB 作成・256 バイト書き込み・CBV 設定・DispatchMesh) | 2000 回 × (12 バイトのルート定数・DispatchMesh) |
| 状態が変わったとき | — | その1体の項目だけ作り直し、一覧を詰め直す(500 項目のコピー程度) |

## DirectX12 の解説

### `DispatchMesh` の引数(AS のグループ数)

- `DispatchMesh(X, Y, Z)` は「AS のスレッドグループを X × Y × Z 個起動する」命令。
- このエンジンの AS は、1 グループで 32 個の Meshlet を調べる(`k_meshletCountPerAmplificationShaderGroup = 32`)。
- そのため X = ceil(Meshlet の数 ÷ 32)。例 : 100 Meshlet なら 100 ÷ 32 = 3 余り 4 なので 4 グループ。
- この値はメッシュごとに決まっていて変わらないので、描画項目に入れておく(毎フレーム計算しない)。

### 描画の順番

- 一覧の並び = GPU に積む順番。不透明なモデルは深度テストで前後が決まるので、順番は見た目に影響しない。
- 後で、同じマテリアルが続くように並べ替えると、テクスチャのキャッシュが効きやすくなる(フェーズ2で検討)。

## 設計

```
ModelRenderSystem(Renderer のメンバ)
 ├─ テーブル(S3・S4)       std::shared_ptr<GPUElementTable> × マクロで登録された型の数
 └─ 描画項目の一覧         std::shared_ptr<ModelDrawItemList> × (メッシュの種類 × マテリアルのテーブル)
       例 : Static × StandardLit / Static × StandardUnLit / Skeletal × StandardLit / Skeletal × StandardUnLit
       ModelDrawItemList
        ├─ 登録の一覧     std::unordered_map<登録番号, フレームごとの項目の一覧>
        └─ 詰め直した一覧 std::vector<std::vector<ModelDrawItem>>(フレームごと。パスはこれを回す)

ModelDrawRegistration(1体のモデルにつき1つ。持ち主は後で作る ModelComponent)
 └─ 登録の一覧 std::vector<DrawItemListRegistration>(weak_ptr<ModelDrawItemList> + 登録番号)
```

- **フレームごとの項目** : Skeletal は、メッシュの要素をフレームの数だけ持つ(S3)。そのため、項目の一覧もフレームごとに持つ。
  Static はどのフレームも同じ項目を入れる。
- **一覧の分け方** : 「メッシュの種類(`Enum::ModelMeshType` = Static / Skeletal)× マテリアルのテーブル(GPU データの型の StaticTypeID)」。
  - `ModelRenderSystem::AddTable`(S3、CONFIG の `TableMap` の1件ごとに Deserialize から呼ばれる)が、**MATERIAL 版のマクロを書いた型** なら、メッシュの種類ごとに一覧を入れる。中身は `Create` で作る。
  - メッシュの種類は Static / Skeletal の2つで増えないので、1段目は map ではなく `std::array`(添字 = `Enum::ModelMeshType` の値)にする。
    マテリアルの種類を足して CONFIG に1行書くと、一覧も自動で増える。
  - `ModelDrawRegistration` は、メッシュのマテリアルの StaticTypeID(`FetchREFTableINFO().k_typeINFO->k_staticTypeID`)の一覧へ、そのまま入れる。
  - Lit / UnLit のパス : `RecordDraw<ModelStandardLitMaterialGPUData>(..., Enum::ModelMeshType::Static)` のように、**描くマテリアルの型をパスが指定する**。
    パスはシェーダー(PSO)と1対1なので、パスのクラスの中に書けばよい(定義した場所に書く)。
  - 影のパス : マテリアルに関係なく全部描くので、`RecordDrawWithoutMaterial(..., Enum::ModelMeshType::Static)` で、そのメッシュの種類の一覧を**全部**描く。
- これで、「テーブルの種類 → パスの種類」の switch(旧 `FetchVALStandardPassType`)と `Enum::ModelRenderPassType` は無くなった。

### 改訂前との比較(トゥーンのマテリアルを足すとき、書き足す場所)

| | 改訂前 | 改訂後 |
|---|---|---|
| GPU データの型 | 構造体を足す | クラスを足し、一番下にマクロを1行書く |
| テーブルの種類 | `ModelRenderTableType` と JSON の enum に1行ずつ | 書かない |
| 1要素の大きさ | `FetchVALElementByteStride` の switch に case | 書かない(マクロの `sizeof`) |
| パスの種類 | `ModelRenderPassType` に Static / Skeletal の2行 | 書かない(マテリアルの数だけ一覧が自動で増える) |
| マテリアル → パス | 描き方の `FetchVALStandardPassType` の switch に case(Static / Skeletal の2か所) | 書かない |
| コマンドシグネチャ(S6) | `l_isShadowPass ? ... : ...` の分岐 | 書かない(RCModelDrawItem を持つルートシグネチャ全部に自動で作る) |
| マテリアルのクラス | `FetchVALTableType` | `FetchREFTableINFO`(GPU データの型を返す1行) |
| パス | パスのクラス(シェーダーごとに必要) | 同じ(`RecordDraw<ModelToonMaterialGPUData>(...)` と書く) |

### 影のパスで一覧を複数描くコスト

- 影は「メッシュの種類の一覧を全部」描くので、マテリアルの種類が2つなら、1回の影の描画で一覧を2つ描く。
- S6 では一覧1つにつき ExecuteIndirect が1回なので、影のカスケード3枚 × マテリアル2種類 = 6回(Static / Skeletal でそれぞれ)。
  項目の数(1万など)には関係なく、マテリアルの種類の数だけ増える。トゥーンを足しても 9回で、CPU の負担はほぼ変わらない。

## ファイル一覧

### 新規

| ファイル | 内容 |
|---|---|
| `Definition/Enum/Graphics/ModelRenderSystemEnum.h`(新規。S3 の改訂で一度消したファイル) | `Enum::ModelMeshType`(Static / Skeletal) |
| `Graphics/Render/Model/DrawItem/ModelDrawItemList.h/.cpp` | 描画項目の一覧 |
| `Graphics/Render/Model/DrawItem/ModelDrawRegistration.h/.cpp` | 1体のモデルの描画項目を作り、一覧へ登録・解除する(RAII) |

### 変更

| ファイル | 変更 |
|---|---|
| `Graphics/Render/Model/ModelRenderSystem.h/.cpp` | 描画項目の一覧を持つ・`RecordDraw` |
| `Graphics/Render/Graph/Pass/Model/Static|Skeletal/Standard/Lit|UnLit/*Pass.cpp`(4つ) | 描画申請をやめて `RecordDraw` |
| `Graphics/Render/Graph/Pass/Model/Shadow/Cascade/ModelCascadeShadowPass.cpp` | 同上 |
| `Definition/Enum/Graphics/RootParameterEnum.h` | `CBModelPerObject` を消す |
| `CONFIG/Graphics/GraphicsCONFIG.json` | 描画申請6つと `ModelPerObjectDynamicConstantBufferUploader` を消す |

### 削除

| ファイル | 理由 |
|---|---|
| `Graphics/Render/Graph/Request/Object/Model/Static/StaticModelPerObjectDrawRequestBase.h/.cpp` | 描画項目の一覧に置き換えた |
| `.../Static/Standard/Lit|UnLit/StaticModelStandard*PerObjectDrawRequest.h` / `.../Static/Shadow/Cascade/StaticModelCascadeShadowPerObjectDrawRequest.h` | 同上 |
| `.../Skeletal/SkeletalAnimationModelPerObjectDrawRequestBase.h/.cpp` と Skeletal の Lit / UnLit / Shadow の描画申請 | 同上 |
| `Graphics/Render/Graph/Request/Object/Model/Buffer/Constant/ModelPerObjectDynamicConstantBufferUploader.h` | CBModelPerObject をやめた |
| `Definition/Struct/Graphics/Buffer/Constant/CBModelPerObjectStruct.h` | 同上 |
| `Definition/Struct/Graphics/ModelCascadeShadowPassStruct.h` | 描画申請のデータ(`StaticModelPerObjectDrawRequestData` など)が不要になった |

> `SkeletalAnimationPerObjectComputeRequest`(スキニングの計算申請)は残す(フェーズ2 P1 で、画面外の間引きと一緒に見直す)。
> `DrawRequestPerObjectBase` は Sprite の描画申請が使っているので残す。

### 登録

- フィルター: `Source\Framework\Graphics\Render\Model\DrawItem`
- `Definition/Enum/Graphics/ModelRenderSystemEnum.h` を vcxproj / filters(`Source\Framework\Definition\Enum\Graphics`)/ Framework.h(「モデルの描画テーブル」の `ModelRenderSystemConstant.h` の次)へ登録し直す。
- Framework.h : 「モデルの描画テーブル」の区画で、`ModelRenderSystem.h` の前に `DrawItem/ModelDrawItemList.h`、後に `DrawItem/ModelDrawRegistration.h`
  (`ModelDrawRegistration` は `ModelDrawItemList::k_invalidRegistrationID` と `Enum::ModelMeshType` を使う。`ModelMaterial.h` は既に前の区画にある)。
- 削除したファイルは vcxproj / filters / Framework.h からも消す(フィルターが空になったら、フィルターも消す)。

---

## コード

### Definition/Enum/Graphics/ModelRenderSystemEnum.h(新規)

S3 の改訂で、テーブルの種類の enum(`ModelRenderTableType`)を消したので、このファイルは S5 で作り直す。

```cpp
#pragma once

namespace FWK::Enum
{
    enum class ModelMeshType
    {
        Static,
        Skeletal,
        Count,
    };
}
```

> - 配列(`std::array`)の添字にそのまま使うので、`Invalid` を置かずに 0 から始める(Static = 0 / Skeletal = 1、Count = 2 が要素数)。
> - JSON には保存しないので、`FWK_JSON_SERIALIZE_ENUM` は書かない。
> - Static と Skeletal は頂点の形(MS のシェーダー・PSO)が違うので、同じ一覧には入れられない。
>   マテリアルと違って、種類は増えない(増やすときは、シェーダーとパスも一緒に作る)。

### Graphics/Render/Model/DrawItem/ModelDrawItemList.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelDrawItemList final
    {
    private:

        using FrameDrawItemList = std::vector<std::vector<Struct::ModelDrawItem>>;

        using RegistrationMap = std::unordered_map<std::uint64_t, FrameDrawItemList>;

    public:

         ModelDrawItemList();
        ~ModelDrawItemList();

        ModelDrawItemList(const ModelDrawItemList&)  = delete;
        ModelDrawItemList(      ModelDrawItemList&&) = delete;

        ModelDrawItemList& operator=(const ModelDrawItemList&)  = delete;
        ModelDrawItemList& operator=(      ModelDrawItemList&&) = delete;

        bool Create(const std::size_t& a_frameCount);

        void RecordDraw(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const std::size_t& a_frameIndex);

        std::uint64_t Register(FrameDrawItemList&& a_frameDrawItemList);

        void Unregister(const std::uint64_t& a_registrationID);

        static constexpr std::uint64_t k_invalidRegistrationID = std::numeric_limits<std::uint64_t>::max();

    private:

        void RebuildFrameDrawItemListIfNeeded();

        static constexpr std::size_t k_emptyFrameCount = 0ULL;
        static constexpr std::size_t k_noErasedCount   = 0ULL;

        static constexpr std::uint64_t k_firstRegistrationID = 0ULL;

        FrameDrawItemList m_frameDrawItemList;

        RegistrationMap m_registrationMap;

        std::size_t m_frameCount;

        std::uint64_t m_nextRegistrationID;

        bool m_isDirty;
    };
}
```

### Graphics/Render/Model/DrawItem/ModelDrawItemList.cpp(新規・写経)

```cpp
#include "ModelDrawItemList.h"

// 1つのパス(例 : StaticModelのLit)で描く「描画項目」の一覧
// 描画項目 = ルート定数(オブジェクト・メッシュ・マテリアルの番号) + ASのグループ数
// 登録・解除は、モデルやマテリアルが変わったときだけ起きる
// パスは、毎フレーム「詰め直した一覧」を先頭から回すだけにする
// 例 : 100体 × 5メッシュなら、一覧は500項目。登録が変わらない限り、詰め直しは起きない
FWK::Graphics::ModelDrawItemList::ModelDrawItemList() :
    m_frameDrawItemList(),

    m_registrationMap(),

    m_frameCount(k_emptyFrameCount),

    m_nextRegistrationID(k_firstRegistrationID),

    m_isDirty(false)
{}
FWK::Graphics::ModelDrawItemList::~ModelDrawItemList() = default;

bool FWK::Graphics::ModelDrawItemList::Create(const std::size_t& a_frameCount)
{
    FWK_ASSERT_RETURN_VALUE_IF(a_frameCount == k_emptyFrameCount, "フレーム数が0のため、ModelDrawItemListの作成に失敗しました。", false);

    // Skeletalは、フレームリソースごとにメッシュの要素(スキニング後の頂点のバッファ)が違うため、
    // 描画項目もフレームごとに持つ(Staticはどのフレームも同じ項目)
    m_frameCount = a_frameCount;

    m_frameDrawItemList.resize(a_frameCount);

    return true;
}

void FWK::Graphics::ModelDrawItemList::RecordDraw(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const std::size_t& a_frameIndex)
{
    // 登録・解除があったフレームだけ、一覧を詰め直す
    RebuildFrameDrawItemListIfNeeded();

    FWK_ASSERT_RETURN_IF(a_frameIndex >= m_frameDrawItemList.size(), "フレームの番号が範囲外のため、描画項目の描画に失敗しました。");

    const auto& l_drawItemList = m_frameDrawItemList[a_frameIndex];

    for (const auto& l_drawItem : l_drawItemList)
    {
        // この描画のオブジェクト・メッシュ・マテリアルの番号を、ルート定数(12バイト)で送る
        // シェーダーはこの番号でテーブルを引く(Model.hlsliのRCModelDrawItem)
        a_directCommandList.SetupRoot32BitConstants(l_drawItem.m_rootConstant, a_rootSignature, Enum::RootParameterType::RCModelDrawItem);

        // ASをメッシュのMeshletの数に合わせた数だけ起動する
        // 例 : 100 Meshletなら、1グループ32 Meshletなので X = 4、Y = 1、Z = 1
        const auto& l_dispatchMeshArguments = l_drawItem.m_dispatchMeshArguments;

        a_directCommandList.DispatchMesh(l_dispatchMeshArguments.ThreadGroupCountX, l_dispatchMeshArguments.ThreadGroupCountY, l_dispatchMeshArguments.ThreadGroupCountZ);
    }
}

std::uint64_t FWK::Graphics::ModelDrawItemList::Register(FrameDrawItemList&& a_frameDrawItemList)
{
    FWK_ASSERT_RETURN_VALUE_IF(a_frameDrawItemList.size() != m_frameCount, "描画項目のフレームの数が一覧と違うため、描画項目の登録に失敗しました。", k_invalidRegistrationID);

    // 登録番号は増えていくだけの通し番号(64ビットなので、使い切る心配はない)
    // k_invalidRegistrationID(64ビットの最大値)は失敗を表す番号で、通し番号がそこまで届くことはない
    // 解除するときは、この番号を渡す
    const auto l_registrationID = m_nextRegistrationID;

    ++m_nextRegistrationID;

    m_registrationMap.try_emplace(l_registrationID, std::move(a_frameDrawItemList));

    // 次に描くときに、一覧を詰め直す
    m_isDirty = true;

    return l_registrationID;
}

void FWK::Graphics::ModelDrawItemList::Unregister(const std::uint64_t& a_registrationID)
{
    // 登録が無ければ(既に解除済みなど)何もしない
    // eraseは消した個数(0か1)を返す
    if (m_registrationMap.erase(a_registrationID) == k_noErasedCount) { return; }

    m_isDirty = true;
}

void FWK::Graphics::ModelDrawItemList::RebuildFrameDrawItemListIfNeeded()
{
    if (!m_isDirty) { return; }

    // 登録の一覧(登録番号ごとに分かれている)を、フレームごとに1本の配列へ詰め直す
    // パスはこの配列を回すだけなので、登録番号ごとのmapをたどるより速い
    // (S6のExecuteIndirectでは、この配列をそのままGPUのバッファへ写す)
    for (std::size_t l_frameIndex = 0ULL; l_frameIndex < m_frameCount; ++l_frameIndex)
    {
        auto& l_drawItemList = m_frameDrawItemList[l_frameIndex];

        // clearは要素を消すだけで、確保済みのメモリは残す(詰め直しのたびに確保し直さない)
        l_drawItemList.clear();

        for (const auto& [l_registrationID, l_frameDrawItemList] : m_registrationMap)
        {
            const auto& l_registeredDrawItemList = l_frameDrawItemList[l_frameIndex];

            l_drawItemList.insert(l_drawItemList.end(), l_registeredDrawItemList.begin(), l_registeredDrawItemList.end());
        }
    }

    m_isDirty = false;
}
```


### Graphics/Render/Model/ModelRenderSystem.h(変更)

```cpp
    private:

        using ModelRenderTableMap = std::unordered_map<TypeAlias::StaticTypeID, std::shared_ptr<GPUElementTable>>;

        using ModelDrawItemListMap             = std::unordered_map<TypeAlias::StaticTypeID, std::shared_ptr<ModelDrawItemList>>;
        using ModelMeshTypeDrawItemListMapList = std::array<ModelDrawItemListMap, static_cast<std::size_t>(Enum::ModelMeshType::Count)>;

    public:
        ...

        void RecordUpload(const DirectCommandList& a_directCommandList, const std::size_t& a_frameIndex) const;

        template <Concept::IsModelMaterialRenderTableElementConcept MaterialElementType>
        void RecordDraw(const RootSignature&      a_rootSignature,
                        const DirectCommandList&  a_directCommandList,
                        const std::size_t&        a_frameIndex,
                        const Enum::ModelMeshType a_meshType) const
        {
            // パスが描くマテリアルの種類を、マテリアルのGPUデータの型で指定する
            // 例 : StaticModelStandardLitPassなら RecordDraw<ModelStandardLitMaterialGPUData>(..., Enum::ModelMeshType::Static)
            //      → 「Static × StandardLit」の描画項目の一覧だけを描く
            // Conceptで、MATERIAL版のマクロを書いた型だけを受け付ける(ModelObjectGPUDataなどを渡すとコンパイルエラーになる)
            const auto& l_materialTableINFO = MaterialElementType::GetREFModelRenderTableINFO();
            const auto* l_typeINFO          = l_materialTableINFO.k_typeINFO;

            if (!l_typeINFO) { return; }

            RecordMaterialDraw(a_rootSignature,
                               a_directCommandList,
                               a_frameIndex,
                               a_meshType,
                               l_typeINFO->k_staticTypeID);
        }

        void RecordDrawWithoutMaterial(const RootSignature&      a_rootSignature,
                                       const DirectCommandList&  a_directCommandList,
                                       const std::size_t&        a_frameIndex,
                                       const Enum::ModelMeshType a_meshType) const;

        nlohmann::json Serialize() const;

        ...

        std::weak_ptr<GPUElementTable> FindVALTable(const TypeAlias::StaticTypeID a_tableStaticTypeID) const;

        std::weak_ptr<ModelDrawItemList> FindVALDrawItemList(const Enum::ModelMeshType a_meshType, const TypeAlias::StaticTypeID a_materialTableStaticTypeID) const;

        const auto& GetREFTableMap() const { return m_tableMap; }

    private:

        void RecordMaterialDraw(const RootSignature&          a_rootSignature,
                                const DirectCommandList&      a_directCommandList,
                                const std::size_t&            a_frameIndex,
                                const Enum::ModelMeshType     a_meshType,
                                const TypeAlias::StaticTypeID a_materialTableStaticTypeID) const;

        const ModelDrawItemListMap* FindPTRDrawItemListMap(const Enum::ModelMeshType a_meshType) const;

        ModelRenderTableMap              m_tableMap                    = {};
        ModelMeshTypeDrawItemListMapList m_meshTypeDrawItemListMapList = {};

        Converter::ModelRenderSystemJsonConverter m_jsonConverter = {};
```

> - `RecordDraw` をテンプレートにして、型 → StaticTypeID の変換だけをヘッダーで行い、本体(`RecordMaterialDraw`)は .cpp に書く。
> - `ModelMeshTypeDrawItemListMapList` : 「メッシュの種類(配列の添字)→ (マテリアルの StaticTypeID → 一覧)」。1段目は `std::array`、2段目は map。
>   影のパスは1段目だけで探して、2段目を全部回す。
> - 1段目を探すのは `FindPTRDrawItemListMap` だけにして、添字が範囲外(Count など)のときは nullptr を返す(配列の外を読まない)。
> - 既存のテンプレート `FetchVALTableSRVDescriptorIndex<ElementType>` / `FindVALTable<ElementType>`(S3)にも、`k_typeINFO` の nullptr の確認を足す(下の「S3 の写経済みのコードの直し」)。

### Graphics/Render/Model/ModelRenderSystem.cpp(変更・写経)

**AddTable の最後(`m_tableMap.try_emplace(l_tableStaticTypeID, std::move(l_table));` の後)に追加:**

```cpp
    // マテリアルのテーブル(FWK_DEFINE_MODEL_MATERIAL_RENDER_TABLE_INFOを書いた型)なら、
    // メッシュの種類(Static / Skeletal)ごとに、描画項目の一覧を入れる
    // 例 : CONFIGにStandardLitとStandardUnLitが書かれていれば、
    //      Static × StandardLit / Static × StandardUnLit / Skeletal × StandardLit / Skeletal × StandardUnLit の4つ
    // テーブルと同じく、CONFIGに書いたマテリアルの分だけ入る(パスの種類のenumは要らない)
    // ここでは入れるだけで、一覧の中身(フレームの数だけの入れ物)はCreateで作る(テーブルと同じ流れ)
    if (!a_tableINFO.k_isMaterial) { return; }

    // 配列の添字がメッシュの種類(0 = Static / 1 = Skeletal)なので、配列を回せば全部の種類へ入れられる
    // それぞれのmapへ、マテリアルのテーブルのStaticTypeIDをキーにして一覧を入れる
    for (auto& l_drawItemListMap : m_meshTypeDrawItemListMapList)
    {
        l_drawItemListMap.try_emplace(l_tableStaticTypeID, std::make_shared<ModelDrawItemList>());
    }
```

**Create のテーブルを作るループの後(オブジェクトとメッシュのテーブルの確認の前)に追加:**

```cpp
    // AddTableで入れた描画項目の一覧を、フレームの数で作る
    // 1段目 = メッシュの種類(配列)、2段目 = マテリアルのテーブルのStaticTypeID(map)
    for (const auto& l_drawItemListMap : m_meshTypeDrawItemListMapList)
    {
        for (const auto& [l_materialTableStaticTypeID, l_drawItemList] : l_drawItemListMap)
        {
            FWK_ASSERT_RETURN_VALUE_IF(!l_drawItemList,                       "描画項目の一覧が無効のため、ModelRenderSystemの作成に失敗しました。",         false);
            FWK_ASSERT_RETURN_VALUE_IF(!l_drawItemList->Create(a_frameCount), "描画項目の一覧の作成に失敗したため、ModelRenderSystemの作成に失敗しました。", false);
        }
    }
```

> - S3 の形(Deserialize → `AddTable` で map に入れる → `Create` で map を回して作る)に合わせた。
>   一覧はテーブルと同じ `AddTable` の中で入れるので、「テーブルはあるのに一覧が無い / 一覧はあるのにテーブルが無い」が起きない。
> - `return` の後にマテリアル用の処理を書くのは、マテリアルではない種類(オブジェクト・メッシュ)を先に外して、字下げを浅くするため。
> - CONFIG にマテリアルが1つも無いと、配列の2つの map は空のまま。影のパスは何も描かず、Lit / UnLit のパスは
>   「メッシュの種類とマテリアルに対応する描画項目の一覧が無い」のアサートを出す(マテリアルが無いとモデルを描けないので、設定の誤りとして知らせる)。

**RecordDrawWithoutMaterial(写経):**

```cpp
void FWK::Graphics::ModelRenderSystem::RecordDrawWithoutMaterial(const RootSignature&      a_rootSignature,
                                                                 const DirectCommandList&  a_directCommandList,
                                                                 const std::size_t&        a_frameIndex,
                                                                 const Enum::ModelMeshType a_meshType) const
{
    // メッシュの種類の、マテリアルごとの一覧のmapを探す(範囲外の種類ならnullptr)
    const auto* l_drawItemListMap = FindPTRDrawItemListMap(a_meshType);

    FWK_ASSERT_RETURN_IF(!l_drawItemListMap, "メッシュの種類が範囲外のため、モデルの描画に失敗しました。");

    // オブジェクトとメッシュのテーブルのSRVの番号を、パスの最初に1回だけ送る
    const auto& l_rcModelTable = FetchVALRCModelTable();

    a_directCommandList.SetupRoot32BitConstants(l_rcModelTable, a_rootSignature, Enum::RootParameterType::RCModelTable);

    // 影のパスのように、マテリアルを使わないパスは、このメッシュの種類の一覧を全部描く
    // 例 : Staticの影なら、Static × StandardLit と Static × StandardUnLit の両方を描く
    // マテリアルのテーブルの番号(RCModelMaterialTable)は送らない(影のルートシグネチャには無い)
    for (const auto& [l_materialTableStaticTypeID, l_drawItemList] : *l_drawItemListMap)
    {
        if (!l_drawItemList) { continue; }

        l_drawItemList->RecordDraw(a_rootSignature, a_directCommandList, a_frameIndex);
    }
}
```

**FindVALDrawItemList(写経):**

```cpp
std::weak_ptr<FWK::Graphics::ModelDrawItemList> FWK::Graphics::ModelRenderSystem::FindVALDrawItemList(const Enum::ModelMeshType a_meshType, const TypeAlias::StaticTypeID a_materialTableStaticTypeID) const
{
    // 1. メッシュの種類(Static / Skeletal)で、マテリアルごとの一覧のmapを探す
    const auto* l_drawItemListMap = FindPTRDrawItemListMap(a_meshType);

    if (!l_drawItemListMap) { return {}; }

    // 2. そのmapの中で、マテリアルのテーブル(GPUデータの型のStaticTypeID)の一覧を探す
    const auto& l_drawItemListITR = l_drawItemListMap->find(a_materialTableStaticTypeID);

    if (l_drawItemListITR == l_drawItemListMap->end()) { return {}; }

    return l_drawItemListITR->second;
}
```

**FindPTRDrawItemListMap(写経、ファイルの一番最後):**

```cpp
const FWK::Graphics::ModelRenderSystem::ModelDrawItemListMap* FWK::Graphics::ModelRenderSystem::FindPTRDrawItemListMap(const Enum::ModelMeshType a_meshType) const
{
    // メッシュの種類(Static = 0 / Skeletal = 1)を、そのまま配列の添字に使う
    // Countなどの範囲外の値が渡されたときは、配列の外を読まないようにnullptrを返す
    const auto& l_meshTypeIndex = static_cast<std::size_t>(a_meshType);

    if (l_meshTypeIndex >= m_meshTypeDrawItemListMapList.size()) { return nullptr; }

    return &m_meshTypeDrawItemListMapList[l_meshTypeIndex];
}
```

> - 戻り値は「見つからないかもしれない参照」なので生のポインタ(FindPTR、規約 13-2)。使う側は必ず nullptr を確認する。
> - 戻り値の型 `ModelDrawItemListMap` はクラスの private の別名なので、.cpp では `FWK::Graphics::ModelRenderSystem::ModelDrawItemListMap` と書く。

**S3 の写経済みのコードの直し(ModelRenderSystem.h のテンプレート2つ):** `k_typeINFO` を使う前に nullptr を確認する。

```cpp
        template <Concept::IsModelRenderTableElementConcept ElementType>
        TypeAlias::DescriptorIndex FetchVALTableSRVDescriptorIndex() const
        {
            // 要素の型(ModelObjectGPUDataなど)から、マクロが作ったテーブルの情報を取り出し、
            // そのStaticTypeIDでテーブルを探して、SRVの番号を返す
            // 例 : FetchVALTableSRVDescriptorIndex<ModelObjectGPUData>() → オブジェクトのテーブルのSRVの番号
            const auto& l_tableINFO = ElementType::GetREFModelRenderTableINFO();
            const auto* l_typeINFO  = l_tableINFO.k_typeINFO;

            if (!l_typeINFO) { return DescriptorHeap::k_invalidDescriptorIndex; }

            return FetchVALTableSRVDescriptorIndex(l_typeINFO->k_staticTypeID);
        }
```

```cpp
        template <Concept::IsModelRenderTableElementConcept ElementType>
        std::weak_ptr<GPUElementTable> FindVALTable() const
        {
            // 要素の型から、その型のテーブルを探す
            // 例 : FindVALTable<ModelMeshGPUData>() → メッシュのテーブル
            // 型で指定するので、別の種類のテーブルを取り違えることがない(取り違えるとコンパイルエラーか、書き込みの大きさのアサートで気づける)
            const auto& l_tableINFO = ElementType::GetREFModelRenderTableINFO();
            const auto* l_typeINFO  = l_tableINFO.k_typeINFO;

            if (!l_typeINFO) { return {}; }

            return FindVALTable(l_typeINFO->k_staticTypeID);
        }
```

**RecordMaterialDraw(写経):**

```cpp
void FWK::Graphics::ModelRenderSystem::RecordMaterialDraw(const RootSignature&          a_rootSignature,
                                                          const DirectCommandList&      a_directCommandList,
                                                          const std::size_t&            a_frameIndex,
                                                          const Enum::ModelMeshType     a_meshType,
                                                          const TypeAlias::StaticTypeID a_materialTableStaticTypeID) const
{
    const auto& l_drawItemList = FindVALDrawItemList(a_meshType, a_materialTableStaticTypeID).lock();

    FWK_ASSERT_RETURN_IF(!l_drawItemList, "メッシュの種類とマテリアルに対応する描画項目の一覧が無いため、モデルの描画に失敗しました。");

    // オブジェクトとメッシュのテーブルのSRVの番号を、パスの最初に1回だけ送る
    // 描画ごとに変わらないので、描画項目のループの外で送る
    const auto& l_rcModelTable = FetchVALRCModelTable();

    a_directCommandList.SetupRoot32BitConstants(l_rcModelTable, a_rootSignature, Enum::RootParameterType::RCModelTable);

    // このパスが描くマテリアルのテーブルのSRVの番号も送る
    // PSは、描画ごとのマテリアルの番号(RCModelDrawItemのg_materialIndex)で、このテーブルを引く
    Struct::RCModelMaterialTable l_rcModelMaterialTable = {};

    l_rcModelMaterialTable.m_materialTableSRVDescriptorIndex = FetchVALTableSRVDescriptorIndex(a_materialTableStaticTypeID);

    a_directCommandList.SetupRoot32BitConstants(l_rcModelMaterialTable, a_rootSignature, Enum::RootParameterType::RCModelMaterialTable);

    l_drawItemList->RecordDraw(a_rootSignature, a_directCommandList, a_frameIndex);
}
```

> - `const auto&` で一時オブジェクト(戻り値)を受けると、その参照の寿命まで一時オブジェクトが延命される(C++ の決まり)。
> - `FindVALDrawItemList(...).lock()` : weak_ptr の取得と lock は、1行で続けて書いてよい(規約の例外)。

### パス(5つ)の書き換え(写経)

**StaticModelStandardLitPass::Execute** の最後の行を次に置き換え、描画申請を探す行(`l_staticModelStandardPerObjectDrawRequest`)と、その確認の行を消す。

```cpp
    const auto& l_modelRenderSystem         = a_renderer.GetREFModelRenderSystem        ();
    const auto& l_currentFrameResourceIndex = a_renderer.GetREFCurrentFrameResourceIndex();

    // StaticModelで、StandardLitのマテリアルを使うメッシュの描画項目の一覧を描く
    // 描くマテリアルは、マテリアルのGPUデータの型で指定する(このパスのPSが読む型)
    l_modelRenderSystem.RecordDraw<ModelStandardLitMaterialGPUData>(*l_rootSignature,
                                                                    l_directCommandList,
                                                                    l_currentFrameResourceIndex,
                                                                    Enum::ModelMeshType::Static);
```

| パス | テンプレート引数(マテリアルの GPU データの型) | `Enum::ModelMeshType` |
|---|---|---|
| `StaticModelStandardLitPass` | `ModelStandardLitMaterialGPUData` | `Static` |
| `StaticModelStandardUnLitPass` | `ModelStandardUnLitMaterialGPUData` | `Static` |
| `SkeletalAnimationModelStandardLitPass` | `ModelStandardLitMaterialGPUData` | `Skeletal` |
| `SkeletalAnimationModelStandardUnLitPass` | `ModelStandardUnLitMaterialGPUData` | `Skeletal` |

**ModelCascadeShadowPass::Execute** は、描画申請を探す2行と確認を消し、カスケードのループの中の2か所を次にする。

```cpp
        l_directCommandList.SetupConstantBufferView(*l_staticModelRootSignature, l_gpuVirtualAddress, Enum::RootParameterType::CBModelCascadeShadowPass);

        // 影はマテリアルに関係なく、Staticの一覧を全部描く(マテリアルのテーブルは送らない)
        l_modelRenderSystem.RecordDrawWithoutMaterial(*l_staticModelRootSignature,
                                                      l_directCommandList,
                                                      l_currentFrameResourceIndex,
                                                      Enum::ModelMeshType::Static);
```

(Skeletal も同じ形で `Enum::ModelMeshType::Skeletal`)

### Graphics/Render/Model/DrawItem/ModelDrawRegistration.h(新規)

ModelComponent を設計から外したため(2026-10-11)、描画項目を作って一覧へ登録・解除する処理は、描き方(コンポーネント)ではなく、
Graphics のこのクラスに置く。後で ModelComponent を作るときは、このクラスをメンバに1つ持ち、モデルやマテリアルが変わったときに `Register` を呼ぶだけになる。

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelDrawRegistration final
    {
    private:

        struct DrawItemListRegistration final
        {
            std::weak_ptr<ModelDrawItemList> m_drawItemList = {};

            std::uint64_t m_registrationID = ModelDrawItemList::k_invalidRegistrationID;
        };

        using FrameDrawItemList = std::vector<std::vector<Struct::ModelDrawItem>>;

        using MaterialFrameDrawItemListMap = std::unordered_map<TypeAlias::StaticTypeID, FrameDrawItemList>;

    public:

         ModelDrawRegistration();
        ~ModelDrawRegistration();

        ModelDrawRegistration(const ModelDrawRegistration&)  = delete;
        ModelDrawRegistration(      ModelDrawRegistration&&) = delete;

        ModelDrawRegistration& operator=(const ModelDrawRegistration&)  = delete;
        ModelDrawRegistration& operator=(      ModelDrawRegistration&&) = delete;

        void Register(const std::vector<std::vector<std::uint32_t>>& a_frameMeshIndexList,
                      const std::vector<std::uint32_t>&              a_meshletCountList,
                      const std::vector<ModelMaterial>&              a_materialList,
                      const Enum::ModelMeshType                      a_meshType,
                      const std::uint32_t                            a_objectIndex);

        void Unregister();

    private:

        MaterialFrameDrawItemListMap BuildMaterialFrameDrawItemListMap(const std::vector<std::vector<std::uint32_t>>& a_frameMeshIndexList,
                                                                       const std::vector<std::uint32_t>&              a_meshletCountList,
                                                                       const std::vector<ModelMaterial>&              a_materialList,
                                                                       const std::uint32_t                            a_objectIndex) const;

        void RegisterDrawItemList(const Enum::ModelMeshType a_meshType, const TypeAlias::StaticTypeID a_materialTableStaticTypeID, FrameDrawItemList&& a_frameDrawItemList);

        static UINT CalculateAmplificationShaderGroupCount(const std::uint32_t a_meshletCount);

        static constexpr UINT k_dispatchMeshThreadGroupCountY = 1U;
        static constexpr UINT k_dispatchMeshThreadGroupCountZ = 1U;

        std::vector<DrawItemListRegistration> m_drawItemListRegistrationList;
    };
}
```

> - `DrawItemListRegistration`(どの一覧に、どの登録番号で入れたか)は、このクラスの中だけで使うので private の入れ子にした(規約 1-3)。
>   改訂前の `Struct::ModelDrawRegistration`(`ModelRenderSystemStruct.h` への追記)は作らない。無効な登録番号の定数は `ModelDrawItemList` の public に置く。
> - 引数の並び : const 参照の配列3つ → const の値(列挙型 → 数値)→ 右辺値参照(規約 20-4)。
> - `Register` は、前の登録を外してから登録し直す。マテリアルを変えたときも、同じ関数をもう一度呼ぶだけでよい。
> - コピーとムーブは禁止。コピーできると、2つの実体が同じ登録番号を持ち、片方の破棄で、もう片方の描画まで外れてしまうため。
> - デストラクタを .cpp に書くので、コンストラクタも .cpp に書き、メンバは初期化子リストで初期化する(既定メンバ初期化子 `= {}` は書かない)。
>   入れ子の構造体 `DrawItemListRegistration` は構造体なので、これまでどおり宣言の位置で初期化する(規約 7-11)。

### Graphics/Render/Model/DrawItem/ModelDrawRegistration.cpp(新規・写経)

```cpp
#include "ModelDrawRegistration.h"

// 1体のモデルの「描画項目の登録」をまとめて持つクラス
// 描画項目 = ルート定数(オブジェクト・メッシュ・マテリアルの番号) + ASのグループ数(Struct::ModelDrawItem)
// 描画項目は、マテリアルの種類ごとに、ModelRenderSystemの一覧(メッシュの種類 × マテリアルのテーブル)へ入れる
// 例 : Staticで、Body・FaceがStandardLit、EyeがStandardUnLitなら
//      「Static × StandardLit」の一覧に2項目、「Static × StandardUnLit」の一覧に1項目を登録する
// どの一覧に、どの登録番号で入れたかを覚えておき、破棄されるとき(デストラクタ)に自動で外す
// (参照が消えたら描画から自動で外れる。毎フレーム全項目をlockして確かめる必要はない)
FWK::Graphics::ModelDrawRegistration::ModelDrawRegistration() :
    m_drawItemListRegistrationList()
{}
FWK::Graphics::ModelDrawRegistration::~ModelDrawRegistration()
{
    Unregister();
}

void FWK::Graphics::ModelDrawRegistration::Register(const std::vector<std::vector<std::uint32_t>>& a_frameMeshIndexList,
                                                    const std::vector<std::uint32_t>&              a_meshletCountList,
                                                    const std::vector<ModelMaterial>&              a_materialList,
                                                    const Enum::ModelMeshType                      a_meshType,
                                                    const std::uint32_t                            a_objectIndex)
{
    // 登録し直すとき(マテリアルを変えたときなど)は、前の登録を先に外す
    // 外さないと、同じメッシュが古いマテリアルと新しいマテリアルで2回描かれる
    Unregister();

    // フレームごとのメッシュの番号が無ければ(モデルがまだ無い)、登録するものがない
    if (a_frameMeshIndexList.empty()) { return; }

    // メッシュの番号・Meshletの数・マテリアルは、どれも「メッシュの順番」で並んでいる必要がある
    // 例 : メッシュが3つなら、どのフレームのメッシュの番号も3つ、Meshletの数も3つ、マテリアルも3つ
    // 数が違うのは呼ぶ側の誤りなので、アサートで知らせる
    const bool l_isSameMeshCount = std::ranges::all_of(a_frameMeshIndexList,
                                                       [&a_materialList](const auto& a_meshIndexList)
                                                       {
                                                           return a_meshIndexList.size() == a_materialList.size();
                                                       });

    FWK_ASSERT_RETURN_IF(!l_isSameMeshCount,                                 "フレームごとのメッシュの数がマテリアルの数と違うため、描画項目の登録に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_meshletCountList.size() != a_materialList.size(), "Meshletの数の一覧がマテリアルの数と違うため、描画項目の登録に失敗しました。");

    // マテリアルの種類ごとに、フレームごとの描画項目の一覧を作る
    // 後で一覧をstd::moveで渡す(中身を移す)ので、constを付けずに値で受ける
    auto l_materialFrameDrawItemListMap = BuildMaterialFrameDrawItemListMap(a_frameMeshIndexList,
                                                                            a_meshletCountList,
                                                                            a_materialList,
                                                                            a_objectIndex);

    // マテリアルの種類ごとに、「メッシュの種類 × マテリアルのテーブル」の一覧へ登録する
    // 例 : StandardLit → 「Static × StandardLit」の一覧(StaticModelStandardLitPassと、Staticの影のパスが描く)
    for (auto& [l_materialTableStaticTypeID, l_frameDrawItemList] : l_materialFrameDrawItemListMap)
    {
        RegisterDrawItemList(a_meshType, l_materialTableStaticTypeID, std::move(l_frameDrawItemList));
    }
}

void FWK::Graphics::ModelDrawRegistration::Unregister()
{
    for (const auto& l_drawItemListRegistration : m_drawItemListRegistrationList)
    {
        // 一覧が先に破棄されている(アプリの終了時に、Rendererが先に消えた)なら、外す必要がない
        // weak_ptrで覚えているので、消えた一覧(壊れたメモリ)には触らない
        const auto& l_drawItemList = l_drawItemListRegistration.m_drawItemList.lock();

        if (!l_drawItemList) { continue; }

        l_drawItemList->Unregister(l_drawItemListRegistration.m_registrationID);
    }

    m_drawItemListRegistrationList.clear();
}

FWK::Graphics::ModelDrawRegistration::MaterialFrameDrawItemListMap FWK::Graphics::ModelDrawRegistration::BuildMaterialFrameDrawItemListMap(const std::vector<std::vector<std::uint32_t>>& a_frameMeshIndexList,
                                                                                                                                          const std::vector<std::uint32_t>&              a_meshletCountList,
                                                                                                                                          const std::vector<ModelMaterial>&              a_materialList,
                                                                                                                                          const std::uint32_t                            a_objectIndex) const
{
    const auto& l_graphicsManager     = GraphicsManager::GetInstance               ();
    const auto& l_resourceContext     = l_graphicsManager.GetREFResourceContext    ();
    const auto& l_modelMaterialSystem = l_resourceContext.GetREFModelMaterialSystem();
    const auto& l_errorMaterial       = l_modelMaterialSystem.GetREFErrorMaterial  ();
    const auto& l_frameCount          = a_frameMeshIndexList.size                  ();

    // キー = マテリアルの種類(GPUデータの型のStaticTypeID)、値 = フレームごとの描画項目の一覧
    // 例 : フレーム3つ、Body(StandardLit)・Eye(StandardUnLit)なら
    //      StandardLit   → [ [Body(フレーム0)], [Body(フレーム1)], [Body(フレーム2)] ]
    //      StandardUnLit → [ [Eye (フレーム0)], [Eye (フレーム1)], [Eye (フレーム2)] ]
    MaterialFrameDrawItemListMap l_materialFrameDrawItemListMap = {};

    for (std::size_t l_meshListIndex = 0ULL; l_meshListIndex < a_materialList.size(); ++l_meshListIndex)
    {
        // 1. このメッシュのマテリアルを、ハンドルから取り出す
        //    .matが読み込めていない(ファイルが無い・壊れている)ときはnullptrなので、エラーマテリアル(マゼンタ)にする
        const auto& l_modelMaterial = a_materialList[l_meshListIndex];
              auto  l_material      = l_modelMaterial.FetchVALMaterial();

        if (!l_material)
        {
            l_material = l_errorMaterial;
        }

        // エラーマテリアルも無い(作成に失敗している)ときは、このメッシュを描かない
        if (!l_material) { continue; }

        // 2. マテリアルの種類(GPUデータの型のStaticTypeID)と、テーブルの何番目かを取り出す
        //    種類は「どの一覧に入れるか」、番号は「PSがマテリアルのテーブルの何番目を読むか」に使う
        //    例 : StandardLitのテーブルの3番 → 「Static × StandardLit」の一覧へ、m_materialIndex = 3 で入れる
        const auto& l_tableINFO = l_material->FetchREFTableINFO();
        const auto* l_typeINFO  = l_tableINFO.k_typeINFO;

        // TypeINFOが無い(マクロの登録が壊れている)マテリアルは、どの一覧に入れるか決められないので、このメッシュを描かない
        if (!l_typeINFO) { continue; }

        const auto l_materialTableElementIndex = l_material->GetVALTableElementIndex();
        const auto l_materialTableStaticTypeID = l_typeINFO->k_staticTypeID;

        // 3. ASのグループ数(DispatchMeshのX)を、このメッシュのMeshletの数から求める
        //    メッシュごとに決まっていて変わらないので、ここで1回だけ計算して描画項目に入れておく
        const auto& l_amplificationShaderGroupCount = CalculateAmplificationShaderGroupCount(a_meshletCountList[l_meshListIndex]);

        // 4. 初めて出てきたマテリアルの種類なら、フレームの数だけ空の一覧を用意する
        //    operator[]は、まだ無いキーなら空の値(フレームの一覧)を作ってから参照を返す
        //    ModelDrawItemList::Registerは「フレームの数が一覧と同じ」ことを求めるので、ここで数を合わせる
        auto& l_frameDrawItemList = l_materialFrameDrawItemListMap[l_materialTableStaticTypeID];

        if (l_frameDrawItemList.empty())
        {
            l_frameDrawItemList.resize(l_frameCount);
        }

        // 5. フレームごとに描画項目を作って入れる
        //    Skeletalは、スキニング後の頂点のバッファがフレームごとに別なので、メッシュの番号がフレームごとに違う
        //    Staticは、どのフレームにも同じメッシュの番号が渡される(呼ぶ側が同じ一覧をフレームの数だけ並べる)
        //    例 : フレーム3つ、Skeletalのメッシュ0 → [フレーム0 : メッシュの10番] [フレーム1 : 11番] [フレーム2 : 12番]
        for (std::size_t l_frameIndex = 0ULL; l_frameIndex < l_frameCount; ++l_frameIndex)
        {
            const auto& l_meshIndexList = a_frameMeshIndexList[l_frameIndex];

            Struct::ModelDrawItem l_drawItem = {};

            // シェーダーは、この3つの番号でオブジェクト・メッシュ・マテリアルのテーブルを引く(Model.hlsliのRCModelDrawItem)
            l_drawItem.m_rootConstant.m_objectIndex   = a_objectIndex;
            l_drawItem.m_rootConstant.m_meshIndex     = l_meshIndexList[l_meshListIndex];
            l_drawItem.m_rootConstant.m_materialIndex = l_materialTableElementIndex;

            // DispatchMeshの引数(ASのグループの数)。Y・Zは常に1
            l_drawItem.m_dispatchMeshArguments.ThreadGroupCountX = l_amplificationShaderGroupCount;
            l_drawItem.m_dispatchMeshArguments.ThreadGroupCountY = k_dispatchMeshThreadGroupCountY;
            l_drawItem.m_dispatchMeshArguments.ThreadGroupCountZ = k_dispatchMeshThreadGroupCountZ;

            l_frameDrawItemList[l_frameIndex].emplace_back(l_drawItem);
        }
    }

    return l_materialFrameDrawItemListMap;
}

void FWK::Graphics::ModelDrawRegistration::RegisterDrawItemList(const Enum::ModelMeshType a_meshType, const TypeAlias::StaticTypeID a_materialTableStaticTypeID, FrameDrawItemList&& a_frameDrawItemList)
{
    // メッシュの種類 × マテリアルのテーブルの一覧を探す
    // 例 : Static × StandardLit の一覧(StaticModelStandardLitPassと、Staticの影のパスが描く)
    const auto& l_graphicsManager   = GraphicsManager::GetInstance           ();
    const auto& l_renderer          = l_graphicsManager.GetREFRenderer       ();
    const auto& l_modelRenderSystem = l_renderer.GetREFModelRenderSystem     ();
    const auto& l_drawItemListWeak  = l_modelRenderSystem.FindVALDrawItemList(a_meshType, a_materialTableStaticTypeID);
    const auto& l_drawItemList      = l_drawItemListWeak.lock                ();

    FWK_ASSERT_RETURN_IF(!l_drawItemList, "メッシュの種類とマテリアルに対応する描画項目の一覧が無いため、描画項目の登録に失敗しました。");

    // 一覧へ入れて、登録番号を受け取る(外すときに、この番号を渡す)
    DrawItemListRegistration l_drawItemListRegistration = {};

    l_drawItemListRegistration.m_drawItemList   = l_drawItemListWeak;
    l_drawItemListRegistration.m_registrationID = l_drawItemList->Register(std::move(a_frameDrawItemList));

    if (l_drawItemListRegistration.m_registrationID == ModelDrawItemList::k_invalidRegistrationID) { return; }

    m_drawItemListRegistrationList.emplace_back(std::move(l_drawItemListRegistration));
}

UINT FWK::Graphics::ModelDrawRegistration::CalculateAmplificationShaderGroupCount(const std::uint32_t a_meshletCount)
{
    // ASのグループ数 = ceil(Meshletの数 ÷ 32)
    // ASは1グループで32個のMeshletを調べる(k_meshletCountPerAmplificationShaderGroup。Model_AS.hlslと同じ値)
    // 割り切れないときは1グループ足す(余りのMeshletを担当するグループ)
    // 例 : 100 Meshletなら 100 ÷ 32 = 3 余り 4 なので 4 グループ / 64 Meshletなら 2 グループ
    auto l_amplificationShaderGroupCount = a_meshletCount / Constant::k_meshletCountPerAmplificationShaderGroup;

    if (a_meshletCount % Constant::k_meshletCountPerAmplificationShaderGroup != Constant::k_noRemainder)
    {
        ++l_amplificationShaderGroupCount;
    }

    return l_amplificationShaderGroupCount;
}
```

> - 改訂前は、この処理が ModelComponent の描き方(`GameObjectModelComponentRendererBase` / `StaticRenderer` / `SkeletalRenderer`)に
>   3つに分かれて書かれていた(Static と Skeletal でほぼ同じループが2回)。Static も「同じメッシュの番号をフレームの数だけ並べた一覧」を渡す形にして、
>   1つのループにまとめた。Static の項目の複製(フレームの数だけ)は、改訂前と同じく登録のときに1回だけ起きる。
> - マテリアルは `Struct::ModelDrawMaterial`(種類と番号の組)ではなく、ハンドル(`Graphics::ModelMaterial`)のまま受け取る。
>   エラーマテリアルへの切り替えも、ここ1か所で行う(呼ぶ側に書かせない)。
> - `l_material->FetchREFTableINFO()` のように、shared_ptr の変数から関数を呼ぶのは規約 11-11 の対象外(戻り値に続けて書いていない)。
> - `k_typeINFO` は TypeINFO への生のポインタなので、使う前に必ず nullptr を確認する(2026-10-11 ユーザー指示)。

### Definition/Enum/Graphics/RootParameterEnum.h(変更)

`CBModelPerObject` の行(enum と `FWK_JSON_SERIALIZE_ENUM` の2か所)を消す。

### CONFIG/Graphics/GraphicsCONFIG.json(変更)

- `DrawRequestPerObjectList` から、`StaticModel...` / `SkeletalAnimationModel...` の6つを消す(`SpriteScreenPerObjectDrawRequest` だけ残る)。
- `DynamicBufferUploaderList` から `ModelPerObjectDynamicConstantBufferUploader`(CreateCount 300000)を消す。
  - これだけで、フレームリソースごとに 300000 × 256 バイト = **約 73MB** の UPLOAD が要らなくなる(× 3 フレームで約 220MB)。

---

## 動作の確認(ビルドは S6 の後)

- ModelComponent(S0)が無いので、S6 のビルドの後もモデルは描かれない。S5 で確かめられるのは、起動時に
  `ModelRenderSystem::Create` が描画項目の一覧(Static / Skeletal × CONFIG のマテリアルの数)を作り、アサートが出ないことだけ。
- 次のことは S0 で ModelComponent を作った後に確かめる。
  - モデルが今までどおり描かれる(Static / Skeletal / 影)。
  - UnLit の .mat に切り替えると、そのメッシュだけ UnLit のパスで描かれる。
  - コンポーネントの削除 / Undo、GameObject の削除 / Undo で、描画が消えたり戻ったりする。

## 次のステップへのつながり

- S6 : `ModelDrawItemList::RecordDraw` の中身を、ExecuteIndirect(引数のバッファ + コマンドシグネチャ)に置き換える。パスと `ModelDrawRegistration` は変えない。
  コマンドシグネチャは ModelRenderSystem が「RCModelDrawItem を持つルートシグネチャ」ごとに持ち、描くときに渡す(一覧は種類が増えるので、一覧ごとには持たない)。
