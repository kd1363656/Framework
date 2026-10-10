# S5 描画の登録(状態が変わった時だけ描画項目を作る)

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
 ├─ テーブル(S3・S4)                               std::shared_ptr<GPUElementTable> × 種類
 └─ 描画項目の一覧(パスの種類ごと)                  std::shared_ptr<ModelDrawItemList> × パスの種類
       ModelDrawItemList
        ├─ 登録の一覧   std::unordered_map<登録番号, フレームごとの項目の一覧>
        └─ 詰め直した一覧 std::vector<std::vector<ModelDrawItem>>(フレームごと。パスはこれを回す)

ModelComponent の描き方(Static / Skeletal)
 └─ 登録の一覧 std::vector<Struct::ModelDrawRegistration>(weak_ptr<ModelDrawItemList> + 登録番号)
```

- **フレームごとの項目** : Skeletal は、メッシュの要素をフレームの数だけ持つ(S3)。そのため、項目の一覧もフレームごとに持つ。
  Static はどのフレームも同じ項目を入れる。
- **パスの種類** : `Enum::ModelRenderPassType`
  - Lit / UnLit は、メッシュのマテリアルの種類(テーブル)で決まる。例 : StandardLit のマテリアル → `StaticStandardLit`。
  - 影は、マテリアルに関係なく全メッシュ(`StaticCascadeShadow` / `SkeletalCascadeShadow`)。

## ファイル一覧

### 新規

| ファイル | 内容 |
|---|---|
| `Definition/Enum/Graphics/ModelRenderSystemEnum.h`(S3 のファイルに追記) | `Enum::ModelRenderPassType` |
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
- 削除したファイルは vcxproj / filters / Framework.h からも消す(フィルターが空になったら、フィルターも消す)。

---

## コード

### Definition/Enum/Graphics/ModelRenderSystemEnum.h(追記)

```cpp
    enum class ModelRenderPassType
    {
        Invalid,
        StaticStandardLit,
        StaticStandardUnLit,
        SkeletalStandardLit,
        SkeletalStandardUnLit,
        StaticCascadeShadow,
        SkeletalCascadeShadow,
        Count,
    };
```

> JSON には保存しないので、`FWK_JSON_SERIALIZE_ENUM` は書かない。

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

        using ModelRenderTableMap = std::unordered_map<Enum::ModelRenderTableType, std::shared_ptr<GPUElementTable>>;

        using ModelDrawItemListMap = std::unordered_map<Enum::ModelRenderPassType, std::shared_ptr<ModelDrawItemList>>;

    public:
        ...

        void RecordDraw(const RootSignature&             a_rootSignature,
                        const DirectCommandList&         a_directCommandList,
                        const std::size_t&               a_frameIndex,
                        const Enum::ModelRenderPassType  a_passType,
                        const Enum::ModelRenderTableType a_materialTableType) const;

        ...

        std::weak_ptr<GPUElementTable> FindVALTable(const Enum::ModelRenderTableType a_type) const;

        std::weak_ptr<ModelDrawItemList> FindVALDrawItemList(const Enum::ModelRenderPassType a_passType) const;

    private:
        ...

        ModelRenderTableMap  m_tableMap        = {};
        ModelDrawItemListMap m_drawItemListMap = {};
```

### Graphics/Render/Model/ModelRenderSystem.cpp(変更・写経)

**Create の最後(テーブルを作った後)に追加:**

```cpp
    // パスの種類ごとに、描画項目の一覧を作る
    // 例 : StaticStandardLit(1) ~ SkeletalCascadeShadow(6)
    for (auto l_passTypeValue = static_cast<std::size_t>(Enum::ModelRenderPassType::StaticStandardLit); l_passTypeValue < static_cast<std::size_t>(Enum::ModelRenderPassType::Count); ++l_passTypeValue)
    {
        auto l_drawItemList = std::make_shared<ModelDrawItemList>();

        FWK_ASSERT_RETURN_VALUE_IF(!l_drawItemList->Create(a_frameCount), "描画項目の一覧の作成に失敗したため、ModelRenderSystemの作成に失敗しました。", false);

        m_drawItemListMap.try_emplace(static_cast<Enum::ModelRenderPassType>(l_passTypeValue), std::move(l_drawItemList));
    }
```

**RecordDraw(写経):**

```cpp
void FWK::Graphics::ModelRenderSystem::RecordDraw(const RootSignature&             a_rootSignature,
                                                  const DirectCommandList&         a_directCommandList,
                                                  const std::size_t&               a_frameIndex,
                                                  const Enum::ModelRenderPassType  a_passType,
                                                  const Enum::ModelRenderTableType a_materialTableType) const
{
    const auto& l_drawItemListITR = m_drawItemListMap.find(a_passType);

    FWK_ASSERT_RETURN_IF(l_drawItemListITR == m_drawItemListMap.end(), "パスの種類に対応する描画項目の一覧が無いため、モデルの描画に失敗しました。");

    const auto& l_drawItemList = l_drawItemListITR->second;

    FWK_ASSERT_RETURN_IF(!l_drawItemList, "描画項目の一覧が無効のため、モデルの描画に失敗しました。");

    // オブジェクトとメッシュのテーブルのSRVの番号を、パスの最初に1回だけ送る
    // 描画ごとに変わらないので、描画項目のループの外で送る
    const auto& l_rcModelTable = FetchVALRCModelTable();

    a_directCommandList.SetupRoot32BitConstants(l_rcModelTable, a_rootSignature, Enum::RootParameterType::RCModelTable);

    // Lit / UnLitのパスは、自分の種類のマテリアルのテーブルの番号も送る
    // 影のパスはマテリアルを使わないため、Invalidが渡され、送らない
    if (a_materialTableType != Enum::ModelRenderTableType::Invalid)
    {
        Struct::RCModelMaterialTable l_rcModelMaterialTable = {};

        l_rcModelMaterialTable.m_materialTableSRVDescriptorIndex = FetchVALTableSRVDescriptorIndex(a_materialTableType);

        a_directCommandList.SetupRoot32BitConstants(l_rcModelMaterialTable, a_rootSignature, Enum::RootParameterType::RCModelMaterialTable);
    }

    l_drawItemList->RecordDraw(a_rootSignature, a_directCommandList, a_frameIndex);
}
```

> `const auto&` で一時オブジェクト(戻り値)を受けると、その参照の寿命まで一時オブジェクトが延命される(C++ の決まり)。

**FindVALDrawItemList(写経):**

```cpp
std::weak_ptr<FWK::Graphics::ModelDrawItemList> FWK::Graphics::ModelRenderSystem::FindVALDrawItemList(const Enum::ModelRenderPassType a_passType) const
{
    const auto& l_drawItemListITR = m_drawItemListMap.find(a_passType);

    if (l_drawItemListITR == m_drawItemListMap.end()) { return {}; }

    return l_drawItemListITR->second;
}
```

### パス(5つ)の書き換え(写経)

**StaticModelStandardLitPass::Execute** の最後の行を次に置き換え、描画申請を探す行(`l_staticModelStandardPerObjectDrawRequest`)と、その確認の行を消す。

```cpp
    const auto& l_modelRenderSystem         = a_renderer.GetREFModelRenderSystem       ();
    const auto& l_currentFrameResourceIndex = a_renderer.GetREFCurrentFrameResourceIndex();

    // StaticModelでStandardLitのマテリアルを使うメッシュを、登録済みの描画項目の一覧から描く
    l_modelRenderSystem.RecordDraw(*l_rootSignature,
                                   l_directCommandList,
                                   l_currentFrameResourceIndex,
                                   Enum::ModelRenderPassType::StaticStandardLit,
                                   Enum::ModelRenderTableType::StandardLitMaterial);
```

| パス | `ModelRenderPassType` | マテリアルのテーブル |
|---|---|---|
| `StaticModelStandardLitPass` | `StaticStandardLit` | `StandardLitMaterial` |
| `StaticModelStandardUnLitPass` | `StaticStandardUnLit` | `StandardUnLitMaterial` |
| `SkeletalAnimationModelStandardLitPass` | `SkeletalStandardLit` | `StandardLitMaterial` |
| `SkeletalAnimationModelStandardUnLitPass` | `SkeletalStandardUnLit` | `StandardUnLitMaterial` |

**ModelCascadeShadowPass::Execute** は、描画申請を探す2行と確認を消し、カスケードのループの中の2か所を次にする。

```cpp
        l_directCommandList.SetupConstantBufferView(*l_staticModelRootSignature, l_gpuVirtualAddress, Enum::RootParameterType::CBModelCascadeShadowPass);

        // 影はマテリアルを使わないので、マテリアルのテーブルは送らない(Invalid)
        l_modelRenderSystem.RecordDraw(*l_staticModelRootSignature,
                                       l_directCommandList,
                                       l_currentFrameResourceIndex,
                                       Enum::ModelRenderPassType::StaticCascadeShadow,
                                       Enum::ModelRenderTableType::Invalid);
```

(Skeletal も同じ形で `SkeletalCascadeShadow`)

### ModelComponent の描き方(変更・写経)

**GameObjectModelComponentRendererBase.h** に共通の処理を足す。

```cpp
        virtual std::vector<std::wstring> FetchVALSubMeshNameList() const = 0;

        virtual void ApplyMaterialList(const std::vector<Struct::ModelDrawMaterial>& a_drawMaterialList) = 0;

    protected:

        void RegisterDrawItemList(std::vector<std::vector<Struct::ModelDrawItem>>&& a_frameDrawItemList, const Enum::ModelRenderPassType a_passType);

        void UnregisterDrawItemList();

    private:

        std::vector<Struct::ModelDrawRegistration> m_drawRegistrationList = {};

        FWK_DEFINE_TYPE_INFO_ROOT(GameObjectModelComponentRendererBase)
```

> 基底クラスにメンバと .cpp の処理が増えるので、`GameObjectModelComponentRendererBase.cpp` を新しく作る(vcxproj に登録)。
> デストラクタで `UnregisterDrawItemList()` を呼ぶため、デストラクタも .cpp に書く。

**GameObjectModelComponentRendererBase.cpp(新規・写経)**

```cpp
#include "GameObjectModelComponentRendererBase.h"

// 描き方(Static / Skeletal)の基底クラス
// 描画項目の登録と解除(どのパスの一覧に、どの登録番号で入れたか)を、ここでまとめて覚える
// 描き方が破棄されると、デストラクタで登録を外す(参照が消えたら自動で描画から外れる)
FWK::GameObjectModelComponentRendererBase::~GameObjectModelComponentRendererBase()
{
    UnregisterDrawItemList();
}

void FWK::GameObjectModelComponentRendererBase::RegisterDrawItemList(std::vector<std::vector<Struct::ModelDrawItem>>&& a_frameDrawItemList, const Enum::ModelRenderPassType a_passType)
{
    const auto& l_graphicsManager   = Graphics::GraphicsManager::GetInstance  ();
    const auto& l_renderer          = l_graphicsManager.GetREFRenderer        ();
    const auto& l_modelRenderSystem = l_renderer.GetREFModelRenderSystem       ();
    const auto& l_drawItemListWeak  = l_modelRenderSystem.FindVALDrawItemList (a_passType);
    const auto& l_drawItemList      = l_drawItemListWeak.lock                 ();

    FWK_ASSERT_RETURN_IF(!l_drawItemList, "パスの種類に対応する描画項目の一覧が無いため、描画項目の登録に失敗しました。");

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
```

**GameObjectModelComponentStaticRenderer(変更・写経)**

ヘッダーのメンバ(S3 の分に足す):

```cpp
        std::vector<Struct::ModelDrawMaterial> m_drawMaterialList                 = {};
        std::vector<UINT>                      m_amplificationShaderGroupCountList = {};
```

`Load` のメッシュのループで、AS のグループ数も覚える:

```cpp
        // ASのグループ数 = ceil(Meshletの数 ÷ 32)
        // 割り切れないときは1グループ足す(余りのMeshletを担当するグループ)
        // 例 : 100 Meshletなら 100 ÷ 32 = 3 余り 4 なので 4 グループ
        const auto& l_meshletCount                = static_cast<UINT>(l_modelMesh.m_meshletData.m_meshletList.size());
              auto  l_amplificationShaderGroupCount = l_meshletCount / Constant::k_meshletCountPerAmplificationShaderGroup;

        if (l_meshletCount % Constant::k_meshletCountPerAmplificationShaderGroup != Constant::k_noRemainder)
        {
            ++l_amplificationShaderGroupCount;
        }

        m_amplificationShaderGroupCountList.emplace_back(l_amplificationShaderGroupCount);
```

`Register` / `Unregister` / `ApplyMaterialList` / `FetchVALSubMeshNameList`:

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

    // パスの種類ごとに、フレームごとの項目の一覧を作る
    // Staticはどのフレームも同じ項目なので、1フレーム分を作ってからフレームの数だけ複製する
    std::unordered_map<Enum::ModelRenderPassType, std::vector<Struct::ModelDrawItem>> l_passDrawItemListMap = {};

    for (std::size_t l_meshListIndex = 0ULL; l_meshListIndex < m_meshIndexList.size(); ++l_meshListIndex)
    {
        const auto& l_drawMaterial = m_drawMaterialList[l_meshListIndex];

        Struct::ModelDrawItem l_drawItem = {};

        l_drawItem.m_rootConstant.m_objectIndex   = m_objectIndex;
        l_drawItem.m_rootConstant.m_meshIndex     = m_meshIndexList[l_meshListIndex];
        l_drawItem.m_rootConstant.m_materialIndex = l_drawMaterial.m_tableElementIndex;

        // DispatchMeshの引数(ASのグループの数)。Y・Zは常に1
        l_drawItem.m_dispatchMeshArguments.ThreadGroupCountX = m_amplificationShaderGroupCountList[l_meshListIndex];
        l_drawItem.m_dispatchMeshArguments.ThreadGroupCountY = k_dispatchMeshThreadGroupCountY;
        l_drawItem.m_dispatchMeshArguments.ThreadGroupCountZ = k_dispatchMeshThreadGroupCountZ;

        // 見た目のパスは、マテリアルの種類で決まる
        // 例 : StandardLitのマテリアル → StaticStandardLitのパスで描く
        const auto l_passType = FetchVALStandardPassType(l_drawMaterial.m_tableType);

        if (l_passType != Enum::ModelRenderPassType::Invalid)
        {
            l_passDrawItemListMap[l_passType].emplace_back(l_drawItem);
        }

        // 影は、マテリアルに関係なく全メッシュを描く(マテリアルの番号は使われない)
        l_passDrawItemListMap[Enum::ModelRenderPassType::StaticCascadeShadow].emplace_back(l_drawItem);
    }

    for (auto& [l_passType, l_drawItemList] : l_passDrawItemListMap)
    {
        // std::vectorを「フレームの数」個、同じ中身で作る(個数と値で作るので( )を使う、規約 9-11)
        std::vector<std::vector<Struct::ModelDrawItem>> l_frameDrawItemList(l_frameCount, l_drawItemList);

        RegisterDrawItemList(std::move(l_frameDrawItemList), l_passType);
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
    // マテリアルが変わると、使うパス(Lit / UnLit)とマテリアルの番号が変わるため、
    // 登録済みなら一度外して、新しいマテリアルで登録し直す
    const bool l_wasRegistered = m_isRegistered;

    Unregister();

    m_drawMaterialList = a_drawMaterialList;

    if (!l_wasRegistered) { return; }

    Register();
}
```

> `k_dispatchMeshThreadGroupCountY` / `Z`(どちらも `1U`)は描き方の基底クラス(`GameObjectModelComponentRendererBase`)の protected の `static constexpr UINT` に置き、Static / Skeletal の両方から使う。
> `FetchVALStandardPassType(tableType)` は private の関数(StandardLit → `StaticStandardLit`、StandardUnLit → `StaticStandardUnLit`、それ以外 → `Invalid`)。
> `FetchVALFrameCount()` は基底の protected の関数で、`Renderer::GetREFFrameResourceList().size()` を返す(Skeletal も使う)。

**GameObjectModelComponentSkeletalRenderer** は、`l_frameDrawItemList[フレーム]` を、`m_frameMeshIndexList[フレーム][メッシュ]` を使って
フレームごとに作る(複製しない)ところだけが違う。パスの種類は `SkeletalStandardLit` / `SkeletalStandardUnLit` / `SkeletalCascadeShadow`。

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

- S6 : `ModelDrawItemList::RecordDraw` の中身だけを、ExecuteIndirect(引数のバッファ + コマンドシグネチャ)に置き換える。パスと描き方は変えない。
