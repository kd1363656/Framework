# S5 描画の登録(状態が変わった時だけ描画項目を作る)

> **2026-10-10 改訂** : 描画項目の一覧を「パスの種類(`Enum::ModelRenderPassType`)」ではなく、
> 「メッシュの種類(`Enum::ModelMeshType`)× マテリアルのテーブル(GPU データの型の StaticTypeID)」で分ける形にした。
> 一覧は、GraphicsCONFIG.json の TableMap に書いたマテリアルの数だけ自動で作られ、描き方の「マテリアル → パス」の switch も無くなった(下の「改訂前との比較」)。

## 目的

今の描画パスは、毎フレーム、描画申請(オブジェクト)→ メッシュの二重ループで、オブジェクトの行列から CB を作っている。
S3・S4 で、行列・メッシュ・マテリアルはテーブルに置いたので、描画に必要なのは**番号3つと AS のグループ数だけ**になった。

そこで、

- ModelComponent の描き方が、**モデル・マテリアルが決まったとき(状態が変わったときだけ)**、「描画項目」(`Struct::ModelDrawItem`)を作って、パスの種類ごとの一覧へ登録する。
- パスは一覧を先頭から回して、`SetGraphicsRoot32BitConstants`(番号3つ)と `DispatchMesh` を積むだけにする。
- 毎フレームの行列の計算・CB の書き込み・weak_ptr の lock・マテリアルの種類の判定は、パスからすべて無くなる。

### 「lock は必須」(以前の指示)との関係

- 以前の合意 : 「参照が消えたら自動で描画リストから外れるように、lock は必須」。
- この形では、**描画項目の一覧を `shared_ptr` で ModelRenderSystem が持ち、登録した側(描き方)は `weak_ptr` + 登録番号を持つ**。
  描き方が破棄されるとき(デストラクタ)に `lock()` して登録を外すので、「参照が消えたら自動で外れる」は守られる。
- 「毎フレーム、全項目を lock して確かめる」はしない。S6 の ExecuteIndirect では CPU が項目を1つずつ回さない(GPU がバッファから読む)ため、
  毎フレームの lock を残すと、ExecuteIndirect の意味が無くなる(1万項目なら毎フレーム1万回の lock が残る)。
- Undo で生き続けるコンポーネントは、S0 の Attach / Detach で登録が外れる。

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

ModelComponent の描き方(Static / Skeletal)
 └─ 登録の一覧 std::vector<Struct::ModelDrawRegistration>(weak_ptr<ModelDrawItemList> + 登録番号)
```

- **フレームごとの項目** : Skeletal は、メッシュの要素をフレームの数だけ持つ(S3)。そのため、項目の一覧もフレームごとに持つ。
  Static はどのフレームも同じ項目を入れる。
- **一覧の分け方** : 「メッシュの種類(`Enum::ModelMeshType` = Static / Skeletal)× マテリアルのテーブル(GPU データの型の StaticTypeID)」。
  - `ModelRenderSystem::AddTable`(S3、CONFIG の `TableMap` の1件ごとに Deserialize から呼ばれる)が、**MATERIAL 版のマクロを書いた型** なら、メッシュの種類ごとに一覧を入れる。中身は `Create` で作る。
    マテリアルの種類を足して CONFIG に1行書くと、一覧も自動で増える。
  - 描き方は、メッシュのマテリアルの StaticTypeID(S4 の `Struct::ModelDrawMaterial::m_tableStaticTypeID`)の一覧へ、そのまま入れる。
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
| `Definition/Struct/Graphics/ModelRenderSystemStruct.h`(S3 のファイルに追記) | `Struct::ModelDrawRegistration` |
| `Graphics/Render/Model/DrawItem/ModelDrawItemList.h/.cpp` | 描画項目の一覧 |

### 変更

| ファイル | 変更 |
|---|---|
| `Graphics/Render/Model/ModelRenderSystem.h/.cpp` | 描画項目の一覧を持つ・`RecordDraw` |
| `Graphics/Render/Graph/Pass/Model/Static|Skeletal/Standard/Lit|UnLit/*Pass.cpp`(4つ) | 描画申請をやめて `RecordDraw` |
| `Graphics/Render/Graph/Pass/Model/Shadow/Cascade/ModelCascadeShadowPass.cpp` | 同上 |
| `GameObject/Component/Model/Renderer/*`(Static / Skeletal / Base) | `Register` / `Unregister` / `ApplyMaterialList` を描画項目の登録にする |
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
- `GameObjectModelComponentRendererBase.cpp` を vcxproj / filters へ登録する。
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
        Invalid,
        Static,
        Skeletal,
        Count,
    };
}
```

> - JSON には保存しないので、`FWK_JSON_SERIALIZE_ENUM` は書かない。
> - Static と Skeletal は頂点の形(MS のシェーダー・PSO)が違うので、同じ一覧には入れられない。
>   マテリアルと違って、種類は増えない(増やすときは、シェーダーとパスも一緒に作る)。

### Definition/Struct/Graphics/ModelRenderSystemStruct.h(追記)

```cpp
    struct ModelDrawRegistration final
    {
        static constexpr std::uint64_t k_invalidRegistrationID = std::numeric_limits<std::uint64_t>::max();

        std::weak_ptr<Graphics::ModelDrawItemList> m_drawItemList = {};

        std::uint64_t m_registrationID = k_invalidRegistrationID;
    };
```

> `Graphics::ModelDrawItemList` の前方宣言を、このファイルの先頭(別の namespace ブロック)に書く(規約 4-6)。

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
    FWK_ASSERT_RETURN_VALUE_IF(a_frameDrawItemList.size() != m_frameCount, "描画項目のフレームの数が一覧と違うため、描画項目の登録に失敗しました。", Struct::ModelDrawRegistration::k_invalidRegistrationID);

    // 登録番号は増えていくだけの通し番号(64ビットなので、使い切る心配はない)
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

        using ModelDrawItemListMap         = std::unordered_map<TypeAlias::StaticTypeID, std::shared_ptr<ModelDrawItemList>>;
        using ModelMeshTypeDrawItemListMap = std::unordered_map<Enum::ModelMeshType, ModelDrawItemListMap>;

    public:
        ...

        void RecordUpload(const CopyCommandList& a_copyCommandList, const std::size_t& a_frameIndex) const;

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

            RecordMaterialDraw(a_rootSignature,
                               a_directCommandList,
                               a_frameIndex,
                               a_meshType,
                               l_materialTableINFO.k_typeINFO->k_staticTypeID);
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

        ModelRenderTableMap          m_tableMap                = {};
        ModelMeshTypeDrawItemListMap m_meshTypeDrawItemListMap = {};

        Converter::ModelRenderSystemJsonConverter m_jsonConverter = {};
```

> - `RecordDraw` をテンプレートにして、型 → StaticTypeID の変換だけをヘッダーで行い、本体(`RecordMaterialDraw`)は .cpp に書く。
> - `ModelMeshTypeDrawItemListMap` : 「メッシュの種類 → (マテリアルの StaticTypeID → 一覧)」の2段の map。
>   影のパスは1段目だけで探して、2段目を全部回す。

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

    for (auto l_meshTypeValue = static_cast<std::size_t>(Enum::ModelMeshType::Static); l_meshTypeValue < static_cast<std::size_t>(Enum::ModelMeshType::Count); ++l_meshTypeValue)
    {
        // operator[]は、まだ無いキー(メッシュの種類)なら空のmapを作ってから参照を返す
        // その中へ、マテリアルのテーブルのStaticTypeIDをキーにして一覧を入れる
        const auto  l_meshType        = static_cast<Enum::ModelMeshType>(l_meshTypeValue);
              auto& l_drawItemListMap = m_meshTypeDrawItemListMap[l_meshType];

        l_drawItemListMap.try_emplace(l_tableStaticTypeID, std::make_shared<ModelDrawItemList>());
    }
```

**Create のテーブルを作るループの後(オブジェクトとメッシュのテーブルの確認の前)に追加:**

```cpp
    // AddTableで入れた描画項目の一覧を、フレームの数で作る
    // 1段目 = メッシュの種類、2段目 = マテリアルのテーブルのStaticTypeID
    for (const auto& [l_meshType, l_drawItemListMap] : m_meshTypeDrawItemListMap)
    {
        for (const auto& [l_materialTableStaticTypeID, l_drawItemList] : l_drawItemListMap)
        {
            FWK_ASSERT_RETURN_VALUE_IF(!l_drawItemList,                       "描画項目の一覧が無効のため、ModelRenderSystemの作成に失敗しました。",           false);
            FWK_ASSERT_RETURN_VALUE_IF(!l_drawItemList->Create(a_frameCount), "描画項目の一覧の作成に失敗したため、ModelRenderSystemの作成に失敗しました。", false);
        }
    }
```

> - S3 の形(Deserialize → `AddTable` で map に入れる → `Create` で map を回して作る)に合わせた。
>   一覧はテーブルと同じ `AddTable` の中で入れるので、「テーブルはあるのに一覧が無い / 一覧はあるのにテーブルが無い」が起きない。
> - `return` の後にマテリアル用の処理を書くのは、マテリアルではない種類(オブジェクト・メッシュ)を先に外して、字下げを浅くするため。
> - CONFIG にマテリアルが1つも無いと、`m_meshTypeDrawItemListMap` にメッシュの種類のキーができない。そのときは描画で
>   「メッシュの種類に対応する描画項目の一覧が無い」のアサートが出る(マテリアルが無いとモデルを描けないので、設定の誤りとして知らせる)。

**RecordDrawWithoutMaterial(写経):**

```cpp
void FWK::Graphics::ModelRenderSystem::RecordDrawWithoutMaterial(const RootSignature&      a_rootSignature,
                                                                 const DirectCommandList&  a_directCommandList,
                                                                 const std::size_t&        a_frameIndex,
                                                                 const Enum::ModelMeshType a_meshType) const
{
    const auto& l_meshTypeDrawItemListITR = m_meshTypeDrawItemListMap.find(a_meshType);

    FWK_ASSERT_RETURN_IF(l_meshTypeDrawItemListITR == m_meshTypeDrawItemListMap.end(), "メッシュの種類に対応する描画項目の一覧が無いため、モデルの描画に失敗しました。");

    // オブジェクトとメッシュのテーブルのSRVの番号を、パスの最初に1回だけ送る
    const auto& l_rcModelTable = FetchVALRCModelTable();

    a_directCommandList.SetupRoot32BitConstants(l_rcModelTable, a_rootSignature, Enum::RootParameterType::RCModelTable);

    // 影のパスのように、マテリアルを使わないパスは、このメッシュの種類の一覧を全部描く
    // 例 : Staticの影なら、Static × StandardLit と Static × StandardUnLit の両方を描く
    // マテリアルのテーブルの番号(RCModelMaterialTable)は送らない(影のルートシグネチャには無い)
    const auto& l_drawItemListMap = l_meshTypeDrawItemListITR->second;

    for (const auto& [l_materialTableStaticTypeID, l_drawItemList] : l_drawItemListMap)
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
    const auto& l_meshTypeDrawItemListITR = m_meshTypeDrawItemListMap.find(a_meshType);

    if (l_meshTypeDrawItemListITR == m_meshTypeDrawItemListMap.end()) { return {}; }

    // 2. そのmapの中で、マテリアルのテーブル(GPUデータの型のStaticTypeID)の一覧を探す
    const auto& l_drawItemListMap = l_meshTypeDrawItemListITR->second;
    const auto& l_drawItemListITR = l_drawItemListMap.find(a_materialTableStaticTypeID);

    if (l_drawItemListITR == l_drawItemListMap.end()) { return {}; }

    return l_drawItemListITR->second;
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

### ModelComponent の描き方(変更・写経)

**GameObjectModelComponentRendererBase.h** に共通の処理を足す。

```cpp
        virtual std::vector<std::wstring> FetchVALSubMeshNameList() const = 0;

        virtual void ApplyMaterialList(const std::vector<Struct::ModelDrawMaterial>& a_drawMaterialList) = 0;

    protected:

        void RegisterDrawItemList(const Enum::ModelMeshType                               a_meshType,
                                  const TypeAlias::StaticTypeID                           a_materialTableStaticTypeID,
                                        std::vector<std::vector<Struct::ModelDrawItem>>&& a_frameDrawItemList);

        void UnregisterDrawItemList();

        std::size_t FetchVALFrameCount() const;

        static constexpr UINT k_dispatchMeshThreadGroupCountY = 1U;
        static constexpr UINT k_dispatchMeshThreadGroupCountZ = 1U;

    private:

        std::vector<Struct::ModelDrawRegistration> m_drawRegistrationList = {};

        FWK_DEFINE_TYPE_INFO_ROOT(GameObjectModelComponentRendererBase)
```

> - 基底クラスにメンバと .cpp の処理が増えるので、`GameObjectModelComponentRendererBase.cpp` を新しく作る(vcxproj に登録)。
>   デストラクタで `UnregisterDrawItemList()` を呼ぶため、デストラクタも .cpp に書く。
> - 引数の並びは「const の値(メッシュの種類・StaticTypeID)→ 右辺値参照(一覧)」(規約 20-4)。

**GameObjectModelComponentRendererBase.cpp(新規・写経)**

```cpp
#include "GameObjectModelComponentRendererBase.h"

// 描き方(Static / Skeletal)の基底クラス
// 描画項目の登録と解除(どの一覧に、どの登録番号で入れたか)を、ここでまとめて覚える
// 描き方が破棄されると、デストラクタで登録を外す(参照が消えたら自動で描画から外れる)
FWK::GameObjectModelComponentRendererBase::~GameObjectModelComponentRendererBase()
{
    UnregisterDrawItemList();
}

void FWK::GameObjectModelComponentRendererBase::RegisterDrawItemList(const Enum::ModelMeshType                               a_meshType,
                                                                     const TypeAlias::StaticTypeID                           a_materialTableStaticTypeID,
                                                                           std::vector<std::vector<Struct::ModelDrawItem>>&& a_frameDrawItemList)
{
    // メッシュの種類 × マテリアルのテーブルの一覧を探す
    // 例 : Static × StandardLit の一覧(StaticModelStandardLitPassと、Staticの影のパスが描く)
    const auto& l_graphicsManager   = Graphics::GraphicsManager::GetInstance ();
    const auto& l_renderer          = l_graphicsManager.GetREFRenderer       ();
    const auto& l_modelRenderSystem = l_renderer.GetREFModelRenderSystem     ();
    const auto& l_drawItemListWeak  = l_modelRenderSystem.FindVALDrawItemList(a_meshType, a_materialTableStaticTypeID);
    const auto& l_drawItemList      = l_drawItemListWeak.lock                ();

    FWK_ASSERT_RETURN_IF(!l_drawItemList, "メッシュの種類とマテリアルに対応する描画項目の一覧が無いため、描画項目の登録に失敗しました。");

    Struct::ModelDrawRegistration l_drawRegistration = {};

    l_drawRegistration.m_drawItemList   = l_drawItemListWeak;
    l_drawRegistration.m_registrationID = l_drawItemList->Register(std::move(a_frameDrawItemList));

    if (l_drawRegistration.m_registrationID == Struct::ModelDrawRegistration::k_invalidRegistrationID) { return; }

    m_drawRegistrationList.emplace_back(std::move(l_drawRegistration));
}

void FWK::GameObjectModelComponentRendererBase::UnregisterDrawItemList()
{
    for (const auto& l_drawRegistration : m_drawRegistrationList)
    {
        // 一覧が先に破棄されている(アプリの終了時)なら、外す必要がない
        const auto& l_drawItemList = l_drawRegistration.m_drawItemList.lock();

        if (!l_drawItemList) { continue; }

        l_drawItemList->Unregister(l_drawRegistration.m_registrationID);
    }

    m_drawRegistrationList.clear();
}

std::size_t FWK::GameObjectModelComponentRendererBase::FetchVALFrameCount() const
{
    // フレームリソースの数(描画項目をフレームごとに持つため、一覧の数を合わせる)
    const auto& l_graphicsManager   = Graphics::GraphicsManager::GetInstance();
    const auto& l_renderer          = l_graphicsManager.GetREFRenderer      ();
    const auto& l_frameResourceList = l_renderer.GetREFFrameResourceList    ();

    return l_frameResourceList.size();
}
```

**GameObjectModelComponentStaticRenderer(変更・写経)**

ヘッダーのメンバ(S3 の分に足す):

```cpp
        std::vector<Struct::ModelDrawMaterial> m_drawMaterialList                  = {};
        std::vector<UINT>                      m_amplificationShaderGroupCountList = {};
```

`Load` のメッシュのループで、AS のグループ数も覚える:

```cpp
        // ASのグループ数 = ceil(Meshletの数 ÷ 32)
        // 割り切れないときは1グループ足す(余りのMeshletを担当するグループ)
        // 例 : 100 Meshletなら 100 ÷ 32 = 3 余り 4 なので 4 グループ
        const auto l_amplificationShaderMeshletCount = static_cast<UINT>(l_modelMesh.m_meshletData.m_meshletList.size());
              auto l_amplificationShaderGroupCount   = l_amplificationShaderMeshletCount / Constant::k_meshletCountPerAmplificationShaderGroup;

        if (l_amplificationShaderMeshletCount % Constant::k_meshletCountPerAmplificationShaderGroup != Constant::k_noRemainder)
        {
            ++l_amplificationShaderGroupCount;
        }

        m_amplificationShaderGroupCountList.emplace_back(l_amplificationShaderGroupCount);
```

> S3 の改訂で、同じループの中に `const auto l_meshletCount`(`std::uint32_t`)を作ったので、こちらは `l_amplificationShaderMeshletCount`(`UINT`)という名前にして重ならないようにした。

`Register` / `Unregister` / `ApplyMaterialList`:

```cpp
void FWK::GameObjectModelComponentStaticRenderer::Register()
{
    if (m_isRegistered) { return; }

    // モデルかマテリアルがまだ決まっていなければ、登録するものがない
    if (m_meshIndexList.empty() ||
        m_drawMaterialList.size() != m_meshIndexList.size())
    {
        return;
    }

    const auto& l_frameCount = FetchVALFrameCount();

    // マテリアルの種類(テーブルのStaticTypeID)ごとに、描画項目を分けて集める
    // 例 : Body・FaceがStandardLit、EyeがStandardUnLitなら、StandardLitに2項目、StandardUnLitに1項目
    std::unordered_map<TypeAlias::StaticTypeID, std::vector<Struct::ModelDrawItem>> l_materialDrawItemListMap = {};

    for (std::size_t l_meshListIndex = 0ULL; l_meshListIndex < m_meshIndexList.size(); ++l_meshListIndex)
    {
        const auto& l_drawMaterial = m_drawMaterialList[l_meshListIndex];

        // マテリアルの種類が決まっていないメッシュ(エラーマテリアルも作れなかったとき)は描かない
        if (l_drawMaterial.m_tableStaticTypeID == StaticTypeIDGenerator::k_invalidStaticTypeID) { continue; }

        Struct::ModelDrawItem l_drawItem = {};

        l_drawItem.m_rootConstant.m_objectIndex   = m_objectIndex;
        l_drawItem.m_rootConstant.m_meshIndex     = m_meshIndexList[l_meshListIndex];
        l_drawItem.m_rootConstant.m_materialIndex = l_drawMaterial.m_tableElementIndex;

        // DispatchMeshの引数(ASのグループの数)。Y・Zは常に1
        l_drawItem.m_dispatchMeshArguments.ThreadGroupCountX = m_amplificationShaderGroupCountList[l_meshListIndex];
        l_drawItem.m_dispatchMeshArguments.ThreadGroupCountY = k_dispatchMeshThreadGroupCountY;
        l_drawItem.m_dispatchMeshArguments.ThreadGroupCountZ = k_dispatchMeshThreadGroupCountZ;

        // マテリアルのStaticTypeIDの一覧へ入れる
        // その一覧は、そのマテリアルのパス(StaticModelStandardLitPassなど)と、Staticの影のパスが描く
        // operator[]は、まだ無いキーなら空の一覧を作ってから参照を返す
        l_materialDrawItemListMap[l_drawMaterial.m_tableStaticTypeID].emplace_back(l_drawItem);
    }

    for (const auto& [l_materialTableStaticTypeID, l_drawItemList] : l_materialDrawItemListMap)
    {
        // Staticはどのフレームも同じ項目なので、1フレーム分をフレームの数だけ複製する
        // std::vectorを「フレームの数」個、同じ中身で作る(個数と値で作るので( )を使う、規約 9-11)
        std::vector<std::vector<Struct::ModelDrawItem>> l_frameDrawItemList(l_frameCount, l_drawItemList);

        RegisterDrawItemList(Enum::ModelMeshType::Static, l_materialTableStaticTypeID, std::move(l_frameDrawItemList));
    }

    m_isRegistered = true;
}
void FWK::GameObjectModelComponentStaticRenderer::Unregister()
{
    if (!m_isRegistered) { return; }

    UnregisterDrawItemList();

    m_isRegistered = false;
}

void FWK::GameObjectModelComponentStaticRenderer::ApplyMaterialList(const std::vector<Struct::ModelDrawMaterial>& a_drawMaterialList)
{
    // マテリアルが変わると、入れる一覧(Lit / UnLit)とマテリアルの番号が変わるため、
    // 登録済みなら一度外して、新しいマテリアルで登録し直す
    const bool l_wasRegistered = m_isRegistered;

    Unregister();

    m_drawMaterialList = a_drawMaterialList;

    if (!l_wasRegistered) { return; }

    Register();
}
```

> - 「マテリアル → パス」の switch(旧 `FetchVALStandardPassType`)は無い。マテリアルの StaticTypeID が、そのまま一覧のキーになる。
> - 影の一覧へ別に入れる必要も無い(影のパスが、Static の一覧を全部描く)。

**GameObjectModelComponentSkeletalRenderer::Register(写経)**

Skeletal は、フレームごとにメッシュの要素の番号が違う(スキニング後の頂点のバッファがフレームごとに別)ため、複製せずにフレームごとに作る。
`Unregister` / `ApplyMaterialList` は Static と同じ。

```cpp
void FWK::GameObjectModelComponentSkeletalRenderer::Register()
{
    if (m_isRegistered) { return; }

    if (m_frameMeshIndexList.empty()) { return; }

    // マテリアルがまだ決まっていなければ(メッシュの数と合わなければ)、登録するものがない
    const auto& l_firstFrameMeshIndexList = m_frameMeshIndexList.front();

    if (m_drawMaterialList.size() != l_firstFrameMeshIndexList.size()) { return; }

    const auto& l_frameCount = FetchVALFrameCount();

    // マテリアルの種類ごとに、「フレームごとの項目の一覧」を集める
    // 例 : フレーム3つ、Body(StandardLit)・Eye(StandardUnLit)なら
    //      StandardLit   → [ [Body(フレーム0)], [Body(フレーム1)], [Body(フレーム2)] ]
    //      StandardUnLit → [ [Eye (フレーム0)], [Eye (フレーム1)], [Eye (フレーム2)] ]
    std::unordered_map<TypeAlias::StaticTypeID, std::vector<std::vector<Struct::ModelDrawItem>>> l_materialFrameDrawItemListMap = {};

    for (std::size_t l_frameIndex = 0ULL; l_frameIndex < m_frameMeshIndexList.size(); ++l_frameIndex)
    {
        const auto& l_meshIndexList = m_frameMeshIndexList[l_frameIndex];

        for (std::size_t l_meshListIndex = 0ULL; l_meshListIndex < l_meshIndexList.size(); ++l_meshListIndex)
        {
            const auto& l_drawMaterial = m_drawMaterialList[l_meshListIndex];

            if (l_drawMaterial.m_tableStaticTypeID == StaticTypeIDGenerator::k_invalidStaticTypeID) { continue; }

            Struct::ModelDrawItem l_drawItem = {};

            // メッシュの番号だけがフレームごとに違う(オブジェクト・マテリアルの番号は同じ)
            l_drawItem.m_rootConstant.m_objectIndex   = m_objectIndex;
            l_drawItem.m_rootConstant.m_meshIndex     = l_meshIndexList[l_meshListIndex];
            l_drawItem.m_rootConstant.m_materialIndex = l_drawMaterial.m_tableElementIndex;

            l_drawItem.m_dispatchMeshArguments.ThreadGroupCountX = m_amplificationShaderGroupCountList[l_meshListIndex];
            l_drawItem.m_dispatchMeshArguments.ThreadGroupCountY = k_dispatchMeshThreadGroupCountY;
            l_drawItem.m_dispatchMeshArguments.ThreadGroupCountZ = k_dispatchMeshThreadGroupCountZ;

            auto& l_frameDrawItemList = l_materialFrameDrawItemListMap[l_drawMaterial.m_tableStaticTypeID];

            // 初めて出てきたマテリアルの種類なら、フレームの数だけ空の一覧を用意する
            // (一覧の登録では、フレームの数が描画項目の一覧と同じである必要がある)
            if (l_frameDrawItemList.empty())
            {
                l_frameDrawItemList.resize(l_frameCount);
            }

            l_frameDrawItemList[l_frameIndex].emplace_back(l_drawItem);
        }
    }

    for (auto& [l_materialTableStaticTypeID, l_frameDrawItemList] : l_materialFrameDrawItemListMap)
    {
        RegisterDrawItemList(Enum::ModelMeshType::Skeletal, l_materialTableStaticTypeID, std::move(l_frameDrawItemList));
    }

    m_isRegistered = true;
}
```

> - `m_frameMeshIndexList` のフレームの数は、S3 で `l_frameDataList.size()`(= フレームリソースの数)にしてあるので、`l_frameCount` と同じ。
> - 最後のループは、一覧を `std::move` で渡す(中身を移すので、`const` を付けずに `auto&` で受ける)。

### Definition/Enum/Graphics/RootParameterEnum.h(変更)

`CBModelPerObject` の行(enum と `FWK_JSON_SERIALIZE_ENUM` の2か所)を消す。

### CONFIG/Graphics/GraphicsCONFIG.json(変更)

- `DrawRequestPerObjectList` から、`StaticModel...` / `SkeletalAnimationModel...` の6つを消す(`SpriteScreenPerObjectDrawRequest` だけ残る)。
- `DynamicBufferUploaderList` から `ModelPerObjectDynamicConstantBufferUploader`(CreateCount 300000)を消す。
  - これだけで、フレームリソースごとに 300000 × 256 バイト = **約 73MB** の UPLOAD が要らなくなる(× 3 フレームで約 220MB)。

---

## 動作の確認(ここで初めて、S3 からの変更が動く。ビルドは S6 の後)

- モデルが今までどおり描かれる(Static / Skeletal / 影)。
- マテリアルのスロットで UnLit の .mat をドロップすると、そのメッシュだけ UnLit のパスで描かれる。
- コンポーネントの削除 / Undo、GameObject の削除 / Undo で、描画が消えたり戻ったりする。
- S1 のプロファイラーで、CPU の「描画命令の記録」が S1 の時点より減っていること。

## 次のステップへのつながり

- S6 : `ModelDrawItemList::RecordDraw` の中身を、ExecuteIndirect(引数のバッファ + コマンドシグネチャ)に置き換える。パスと描き方は変えない。
  コマンドシグネチャは ModelRenderSystem が「RCModelDrawItem を持つルートシグネチャ」ごとに持ち、描くときに渡す(一覧は種類が増えるので、一覧ごとには持たない)。
