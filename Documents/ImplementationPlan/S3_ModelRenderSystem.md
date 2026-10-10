# S3 ModelRenderSystem(オブジェクトのテーブル・メッシュのテーブル)とルート定数

> **2026-10-10 改訂(テーブルの種類をマクロで登録する形に変更)**
> 最初の形では、テーブルの種類を `Enum::ModelRenderTableType` で区別し、`FetchVALElementByteStride` の switch で1要素の大きさを返していた。
> これだと、マテリアルの種類を足すたびに、enum・JSON の enum・switch の3か所を書き足す必要がある。
> そこで、GPU へ送るデータをクラスにして、クラスの一番下に `FWK_DEFINE_MODEL_RENDER_TABLE_INFO(型)` を書く形にした(`FWK_DEFINE_TYPE_INFO` と同じ仕組み)。
> マクロが「型の名前・StaticTypeID・sizeof」を起動時にレジストリへ登録し、ModelRenderSystem はその一覧からテーブルを作る。**種類を増やしても ModelRenderSystem は書き換えない。**
>
> **2026-10-10 追加改訂** : マクロは `FWK_DEFINE_TYPE_INFO_SINGLE(Type)` を中で展開し、名前・StaticTypeID は TypeINFO に任せる。
> `Struct::ModelRenderTableINFO` は `TypeINFO` へのポインタ(`Struct::TypeINFO::k_baseINFO` と同じ生ポインタ。実体は static 変数でアプリの終了まで消えない) + 1要素の大きさ + マテリアルかどうか だけを持つ(使う側は `.k_typeINFO->k_staticTypeID` / `.k_typeINFO->k_name`)。
> 更新が多いときの対策(区間まとめ・全体コピー)は入れない(10,000 体全部動いても 1.36MB で、必要になってから足す)。
>
> **2026-10-10 追加改訂(テーブルのコピーを RenderGraph のパスにする)** : `Renderer::BeginFrame` で `RecordUpload` を呼ぶのをやめ、
> `ModelRenderTableUploadPass`(実行レイヤー `Upload` = 一番前)にした。理由と S5 / S6 / P1 への影響は下の「テーブルのコピーを RenderGraph のパスにする」。
>
> **写経し直すところ(骨組みはこちらで書き換え済み)**
>
> | ファイル | 関数 |
> |---|---|
> | `Graphics/Render/Model/Table/ModelRenderTableINFORegistry.cpp`(新規) | `Register` |
> | `Graphics/Render/Model/ModelRenderSystem.cpp` | クラスの説明のコメント / `Create` / `AddTable`(新規) / `FetchVALRCModelTable` |
> | `Graphics/Render/Model/Converter/Json/ModelRenderSystemJsonConverter.cpp` | `Deserialize` / `Serialize`(まだ空なので、新しいコードで書く) |
> | `Graphics/Render/Graph/Pass/Model/Table/ModelRenderTableUploadPass.cpp`(新規) | コンストラクタ / `Execute` |
> | `Graphics/Render/Model/Table/ModelRenderTableINFORegistry.cpp` | `Register` の最初に `k_typeINFO` が nullptr かの確認を足した(下のコード) |
> | `Graphics/Render/Model/Table/ModelRenderTableINFORegistry.cpp` | `Register` を vector + `any_of` から unordered_map + `try_emplace` に変更(2026-10-10、ヘッダーは書き換え済み) |
> | `Graphics/Render/Model/ModelRenderSystem.cpp` | `Create` は `m_tableMap`(Deserialize で `AddTable` されたもの)を回して GPU に作るだけ(`FetchVALCapacity` は削除) |
> | `Graphics/Resource/Buffer/Table/GPUElementTable.cpp`(S2) | `Create` の `a_capacity` / `a_elementByteStride` を `m_capacity` / `m_elementByteStride` に(S2 の冒頭の改訂を参照) |
> | `Shader/Model/Model.hlsli` | コメント2行の `Struct::ModelObjectGPUData` / `Struct::ModelMeshGPUData` を `Graphics::` に直す(名前空間が変わったため) |
>
> **2026-10-10 追加改訂(CONFIG に書いた種類だけテーブルを作る / 設定の vector をやめる、ユーザー指示)** :
> 以前はレジストリに登録された全部の種類のテーブルを作り、容量だけを CONFIG の vector(`m_tableSettingList`)から探していた。
> **Renderer の RootSignatureMap と同じ形**にした : Deserialize で CONFIG の `TableMap` の1件ごとに `AddTable` を呼び、
> テーブル(容量と1要素の大きさだけを持たせた `GPUElementTable`)を `m_tableMap` に入れる。Create は `m_tableMap` を回して GPU のメモリを作るだけ。
> map で持つものは map で保存・復元する(順番に意味が無いので vector は使わない)。Serialize は `m_tableMap` から書き出す
> (StaticTypeID は起動ごとに変わりうるので、名前は `TypeINFORegistry::FindPTRByID` で引いて書く)。
> レジストリの map(型の名前がキー)は「CONFIG の名前から情報を引く」ために使う。CONFIG に無い種類は作らない。書き間違い・重複・Object/Mesh の書き忘れはアサートで分かる。
> 影響 : `m_tableSettingList` / `SetTableSettingList` / `GetREFTableSettingList` / `Struct::ModelRenderTableSetting` / `FetchVALCapacity` を削除、
> `AddTable` / `GetREFTableMap` を追加、JSON のキーは `TableSettingList` → `TableMap`、既定の容量 1024 は `ModelRenderSystemJsonConverter::k_defaultCapacity` へ。
> S2 の `GPUElementTable` は `SetCapacity` / `SetElementByteStride` / `GetVALCapacity` を足し、`Create` の引数から容量と1要素の大きさを外した。
> S5 の描画項目の一覧は `AddTable` の中で(マテリアルの型だけ)作り、`Create` で GPU 側を作る。S6 のヘッダーからも消した。
>
> 消したもの : `Definition/Enum/Graphics/ModelRenderSystemEnum.h`(S5 で別の enum を入れて作り直す)、`ModelRenderSystem::FetchVALElementByteStride`、
> `ModelRenderSystemStruct.h` の4つの GPU データの構造体(クラスにして別のファイルへ移した)。

## 目的

`CBModelPerObject`(1回の描画ごとに 208 バイト → 256 バイトの CB を書く)をやめて、次の形にする。

| データ | 置き場所 | 書くタイミング |
|---|---|---|
| ワールド行列・逆転置行列・最大スケール・向き | **オブジェクトのテーブル**(1体につき 1 要素、136 バイト) | 行列が変わったときだけ |
| 頂点・Meshlet などのバッファの SRV 番号と Meshlet の数 | **メッシュのテーブル**(1メッシュにつき 1 要素、24 バイト) | モデルを読み込んだときに1回だけ |
| マテリアルの値 | マテリアルのテーブル(S4 で作る) | マテリアルを変えたときだけ |
| 「どのオブジェクトの・どのメッシュを・どのマテリアルで」 | **ルート定数**(4バイト × 3 = 12 バイト) | 描画のたび(GPU のコマンドに直接入る。CB は書かない) |
| テーブルの SRV の番号 | ルート定数 | パスの最初に1回 |

シェーダーは、ルート定数の番号でテーブルを引いて値を読む。

### 数値の例(100 体 × 5 メッシュ × 4 パス、動くのは 10 体)

| | 今 | S3 の後 |
|---|---|---|
| CPU が毎フレーム書くバイト数 | 2000 回 × 256 バイト = **512,000 バイト** | 10 体 × 136 バイト = **1,360 バイト** |
| 描画ごとの命令 | CB の書き込み + `SetGraphicsRootConstantBufferView` + `DispatchMesh` | `SetGraphicsRoot32BitConstants`(12 バイト) + `DispatchMesh` |

## ※ S3 ~ S5 の間はビルドが通らない

- S3 でシェーダーとルートシグネチャを新しい形にするが、描画する C++ 側(パス・描画申請)を書き換えるのは S5。
- ユーザーとの約束どおり、ビルドは S6(ExecuteIndirect)の後に行う。
- 途中で動かしたくなったら、S5 まで進めてからにする。

## DirectX12 の解説

### ルートシグネチャとルートパラメーター

- **ルートシグネチャ** : 「シェーダーへ、どのデータを、どの番号(レジスタ)で渡すか」の一覧表。PSO を作るときに一緒に指定する。
- 一覧表の1行を **ルートパラメーター** と呼ぶ。このエンジンでは GraphicsCONFIG.json の `RootSignatureMap` に書いてある。
- ルートパラメーターの種類(よく使うもの):

| 種類 | 何を渡すか | 1つあたりの大きさ(ルートシグネチャの上限は 64 DWORD) |
|---|---|---|
| `D3D12_ROOT_PARAMETER_TYPE_CBV` | 定数バッファの GPU アドレス | 2 DWORD |
| `D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS`(ルート定数) | 4 バイトの値そのものを、いくつか | 値の個数 DWORD |
| `D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE` | ディスクリプタヒープの範囲 | 1 DWORD |

### ルート定数(32BIT_CONSTANTS)

- **値そのものが、コマンドリストに直接書き込まれる。** 定数バッファのように「UPLOAD にデータを書いて、そのアドレスを渡す」必要がない。
- そのため、数個の値(番号など)を描画のたびに変えるのに向いている。
- HLSL では、普通の `cbuffer` として受け取る。

```hlsl
cbuffer RCModelDrawItem : register(b1)
{
    uint g_objectIndex;
    uint g_meshIndex;
    uint g_materialIndex;
};
```

- C++ からは `SetGraphicsRoot32BitConstants(ルートパラメーターの番号, 値の個数, 値の先頭アドレス, 書き始める位置)` で送る。
- **S6 の ExecuteIndirect では、この3つの値を GPU のバッファから読ませて、描画ごとに変えられる。** CB のアドレスより小さく、引数のバッファが小さくて済む。

### シェーダーの可視性(ShaderVisibility)

- ルートパラメーターごとに「どのシェーダーステージから見えるか」を決める。
- `RCModelDrawItem` は AS(カリング)・MS(頂点)・PS(マテリアル)の全部が読むので `D3D12_SHADER_VISIBILITY_ALL`。
- `RCModelMaterialTable`(S4)は PS だけが読むので `D3D12_SHADER_VISIBILITY_PIXEL`。

### Amplification Shader をまとめる

- 今の `StaticModel_AS.hlsl` と `SkeletalAnimationModel_AS.hlsl` は中身が同じ(違うのはファイル名だけ)。
- テーブルを読む形にすると、Static と Skeletal の違いは「メッシュのテーブルが指す頂点バッファ」だけになるので、
  **1つの `Model/Model_AS.hlsl` にまとめる。** PSO の `AmplificationShader.FilePath` を2つとも同じファイルにする。

### Skeletal のメッシュの要素は、フレームの数だけ持つ

- スキニング後の頂点バッファと Meshlet の境界のバッファは、フレームリソースごとに別のもの(`SkeletalAnimationPlayerFrameData`)。
- そのため Skeletal は、1メッシュにつき **フレームの数(3つ)** のメッシュの要素を持ち、描画のときに今のフレームの要素の番号を送る。
  - 例 : 5 メッシュのキャラなら、メッシュの要素は 5 × 3 = 15 個。読み込んだときに1回だけ書く。

## テーブルのコピーを RenderGraph のパスにする

### RenderGraph がしていること(今のコード)

- パスの順番は2つの決まりで決める(`RenderGraphPassSorter`)。
  1. **実行レイヤー**(`Enum::RenderGraphPassExecutionLayer`)が前のパスを先に実行する。例 : `Animation`(スキニング)→ `Shadow` → `Model`。
  2. 同じテクスチャ(RenderTarget / DepthStencil / ShadowMap)を「書くパス → 読むパス」の順にする(`ReadXxx` / `WriteXxx` の宣言)。
- 状態の遷移(バリア)を RenderGraph が張るのは、2 のテクスチャだけ。**バッファは RenderGraph の外**で、パスが自分でバリアを張る。
  先例 : `SkeletalAnimationComputePass` は、骨の行列・スキニング後の頂点・Meshlet の境界のバッファのバリアを、パスの中で自分で張っている。

### 形

```
RenderGraph::Execute(パスを実行順に)
  ModelRenderTableUploadPass        レイヤー Upload     … ModelRenderSystem::RecordUpload(書き換えた要素だけコピー)
  SkeletalAnimationComputePass      レイヤー Animation
  ModelCascadeShadowPass            レイヤー Shadow     … テーブルを読む
  Static / Skeletal の Lit / UnLit  レイヤー Model      … テーブルを読む
  ...
RenderGraph::ExecutePreviewView(AllViews のパスだけ)
  影・Lit / UnLit …                                      … メインビューでコピー済みのテーブルを読む
```

- **実行レイヤー `Upload` を一番前に足す**だけで、すべてのパスより前に動く。`ReadXxx` / `WriteXxx` は宣言しない(テーブルはテクスチャではない)。
- バリア(`COMMON → COPY_DEST → ALL_SHADER_RESOURCE`)は、今までどおり `GPUElementTable::RecordUpload` の中で張る(S2)。
- ビューの範囲は既定の `MainViewOnly`。プレビューはメインビューの**後**に同じコマンドリストで描くので、メインビューで1回コピーすれば足りる。

### `Renderer::BeginFrame` で呼ぶ形と比べて

| | BeginFrame で呼ぶ(前の案) | パスにする(この案) |
|---|---|---|
| 順番を決める所 | Renderer の関数の中の行の位置 | GraphicsCONFIG.json のパスの一覧 + 実行レイヤー(RenderGraph の決まりに揃う) |
| S1 のプロファイラー | 測られない(パスではないため) | パスとして GPU の時間が出る(`Upload` は `Animation` ではないので、ダイレクトキューで測る) |
| P1 の GPU カリング | カリングのパスとの前後を、Renderer と RenderGraph の両方で考える | `Upload` → `Animation` → `ModelCulling`(P1 で足す)→ `Shadow` と、レイヤーだけで決まる |
| 書く量 | 1行 | パス1つ(.h / .cpp)+ enum に1つ + CONFIG に1行 |

### S5 / S6 / P1 への影響(確認した結果)

- **S5(描画の登録)** : 変更なし。パスは今までどおり `ModelRenderSystem::RecordDraw` を呼ぶ。描画項目の一覧は CPU だけのデータ。
- **S6(ExecuteIndirect)** : 変更なし。引数のバッファは UPLOAD ヒープ(`GENERIC_READ` のまま、バリア不要)で、`RecordDraw` の中で写す。
  コマンドシグネチャも ModelRenderSystem が持つ。RenderGraph が見るものは増えない。
- **S1(GPU プロファイラー)** : 変更なし。`ExecutePass` は `Animation` 以外をダイレクトキューで測るので、`Upload` もそのまま測られる。
- **P1(GPU カリング)** : 予定の `ModelGPUCullingPass`(レイヤー `ModelCulling`)が、オブジェクトのテーブル(境界球)を読む。
  レイヤーの並びを `Upload → Animation → ModelCulling → Shadow → Model` にすれば、コピーの後にカリングが動く。
  カリングで書く「詰めた引数のバッファ・カウントバッファ」もバッファなので、今の RenderGraph ではパスが自分でバリアを張る(スキニングと同じ)。
  バッファも RenderGraph に宣言させる(`ReadBuffer` / `WriteBuffer`)かは、バッファを書くパスが3つ(スキニング・カリング・+1)になる P1 の時点で決める。

## C++ の解説 : テーブルの種類を、型に書いたマクロで登録する

### 何が困るのか(switch の形)

```cpp
switch (a_type)
{
    case Enum::ModelRenderTableType::Object:              { return sizeof(ModelObjectGPUData); } break;
    case Enum::ModelRenderTableType::StandardLitMaterial: { return sizeof(ModelStandardLitMaterialGPUData); } break;
    ...
}
```

- マテリアルの種類(トゥーンなど)を足すたびに、**定義した場所とは別のファイル**(enum・switch)を書き足す必要がある。書き忘れても、動かすまで気づけない。
- 「クラスを足すと増えていく if / switch」は、**定義した場所に1行書けば登録される形**(登録マクロ)にする(規約 18-9)。

### 登録の流れ

```
ModelStandardLitMaterialGPUData.h
  class ModelStandardLitMaterialGPUData final
  {
      ...
      FWK_DEFINE_MODEL_MATERIAL_RENDER_TABLE_INFO(ModelStandardLitMaterialGPUData)   ← 書くのはこの1行だけ
  };

        │ マクロが作るもの
        ├─ FWK_DEFINE_TYPE_INFO_SINGLE が TypeINFO { "ModelStandardLitMaterialGPUData", StaticTypeID = 7 } を TypeINFORegistry へ登録
        ├─ GetREFModelRenderTableINFO() … { 上の TypeINFO へのポインタ, 40 バイト, マテリアル }
        └─ inline static な変数 k_autoRegister… (作られるとき = main より前 に、コンストラクタが Register を呼ぶ)

ModelRenderTableINFORegistry(シングルトン)
  m_tableINFONameMap = { "ModelObjectGPUData"→Object(136) , "ModelMeshGPUData"→Mesh(24) , "ModelStandardLitMaterialGPUData"→StandardLit(40) , ... }

GraphicsCONFIG.json の TableMap
  [ { "TypeName": "ModelObjectGPUData", "Capacity": 4096 }, { "TypeName": "ModelMeshGPUData", ... }, ... ]

ModelRenderSystemJsonConverter::Deserialize
  1件ごとに、型の名前でレジストリの map から情報を探し、ModelRenderSystem::AddTable(情報, 容量) を呼ぶ
  AddTable : GPUElementTable を作って容量と1要素の大きさ(k_elementByteStride)を持たせ、m_tableMap[StaticTypeID] = テーブル
  (CONFIG に無い種類は作らない)

ModelRenderSystem::Create
  m_tableMap を回して、テーブルごとに GPU のメモリ(DEFAULT / UPLOAD / SRV)を作る

使う側
  FindVALTable<ModelMeshGPUData>()  … 型で指定。中で GetREFModelRenderTableINFO().k_typeINFO->k_staticTypeID を使って探す
```

### `inline static` なメンバ変数が main より前に作られる理由

- C++ では、グローバル変数と static なメンバ変数は、**main が始まる前に作られる**(初期化される)。
- `inline static const RegisterXxx k_autoRegister = {};` のように書くと、ヘッダーに書いても「プログラム全体で1つ」になる(C++17 の inline 変数)。
- その変数が作られるときにコンストラクタが動くので、そこで `Register` を呼べば「クラスを定義しただけで登録される」。
  `FWK_DEFINE_TYPE_INFO` や `FWK_REGISTER_FACTORY_METHOD` も同じ仕組み。
- レジストリをシングルトン(関数の中の static 変数)にしているのは、どのクラスが先に登録しに来ても、その時点でレジストリが必ず作られているようにするため。

### StaticTypeID で区別する

- `StaticTypeIDGenerator::GetVALTypeID<型>()` は、型ごとに起動時に1つ決まる番号(0, 1, 2, ...)を返す。
- テーブルの `std::unordered_map` のキーをこの番号にすると、enum を用意しなくても型ごとに区別できる。

### クラスにしても GPU へそのまま送れる理由

- メンバを private にしても、**全部のメンバが同じアクセス指定なら、メモリの並びは構造体と同じ**(standard layout)。
- static のメンバ・入れ子のクラス・関数は、1要素の大きさにも並びにも入らない(`sizeof` は今までと同じ 136 / 24 / 40 / 20 バイト)。
- マクロの `static_assert` で、`memcpy` してよい型(trivially copyable)か、並びが決まっている型(standard layout)かを、コンパイル時に確かめる。

## ファイル一覧

### 新規

| ファイル | 内容 |
|---|---|
| `Source/Framework/Definition/Macros/Graphics/ModelRenderTableINFORegistryMacros.h` | `FWK_DEFINE_MODEL_RENDER_TABLE_INFO` / `FWK_DEFINE_MODEL_MATERIAL_RENDER_TABLE_INFO` |
| `Source/Framework/Definition/Struct/Graphics/ModelRenderTableINFORegistryStruct.h` | `Struct::ModelRenderTableINFO`(型の名前・StaticTypeID・1要素の大きさ・マテリアルか) |
| `Source/Framework/Definition/Concept/IsModelRenderTableElement/IsModelRenderTableElementConcept.h` | `Concept::IsModelRenderTableElementConcept` / `IsModelMaterialRenderTableElementConcept` |
| `Source/Framework/Graphics/Render/Model/Table/ModelRenderTableINFORegistry.h/.cpp` | マクロが登録しに来る一覧(シングルトン) |
| `Source/Framework/Graphics/Render/Model/Table/ModelObjectGPUData.h` / `ModelMeshGPUData.h` | オブジェクト・メッシュのテーブルの1要素(クラス + マクロ) |
| `Source/Framework/Graphics/Resource/Model/Material/Standard/Lit/ModelStandardLitMaterialGPUData.h` | StandardLit のマテリアルのテーブルの1要素(S4 のマテリアルのクラスと同じフォルダ) |
| `Source/Framework/Graphics/Resource/Model/Material/Standard/UnLit/ModelStandardUnLitMaterialGPUData.h` | StandardUnLit のマテリアルのテーブルの1要素 |
| `Source/Framework/Definition/Constant/Graphics/ModelRenderSystemConstant.h` | 向きの符号などの定数(旧 `ModelPerObjectConstantBufferUploaderConstant.h` から移す) |
| `Source/Framework/Definition/Struct/Graphics/Buffer/Root/RCModelStruct.h` | `Struct::RCModelDrawItem` / `RCModelTable` / `RCModelMaterialTable` |
| `Source/Framework/Definition/Struct/Graphics/ModelRenderSystemStruct.h` | `Struct::ModelDrawItem` |
| `Source/Framework/Graphics/Render/Model/ModelRenderSystem.h/.cpp` | テーブルをまとめて持つクラス |
| `Source/Framework/Graphics/Render/Model/Converter/Json/ModelRenderSystemJsonConverter.h/.cpp` | 容量の設定の JSON |
| `Source/Framework/Graphics/Render/Graph/Pass/Model/Table/ModelRenderTableUploadPass.h/.cpp` | テーブルの書き換えた要素を GPU へコピーするパス(実行レイヤー `Upload`) |
| `Shader/Model/Model_AS.hlsl` | Static / Skeletal 共通の AS |

### 削除

| ファイル | 理由 |
|---|---|
| `Shader/Model/Static/StaticModel_AS.hlsl` | `Model_AS.hlsl` にまとめた |
| `Shader/Model/Skeletal/SkeletalAnimationModel_AS.hlsl` | 同上 |
| `Source/Framework/Definition/Constant/Graphics/ModelPerObjectConstantBufferUploaderConstant.h` | 定数を `ModelRenderSystemConstant.h` へ移した |

### 変更

| ファイル | 変更 |
|---|---|
| `Definition/Enum/Graphics/RootParameterEnum.h` | `RCModelDrawItem` / `RCModelTable` / `RCModelMaterialTable` を追加(`CBModelPerObject` は S5 で消す) |
| `Graphics/Command/List/Direct/DirectCommandList.h/.cpp` | `SetupRoot32BitConstants` |
| `Graphics/Render/Renderer.h/.cpp` | `ModelRenderSystem` を持つ。PostDeserialize で作る(コピーは `ModelRenderTableUploadPass`) |
| `Definition/Enum/Graphics/RenderGraphPassEnum.h` | `RenderGraphPassExecutionLayer::Upload` を `Animation` の前に追加(骨組みで追加済み) |
| `Graphics/Render/Converter/Json/RendererJsonConverter.h/.cpp` | `"ModelRenderSystem"` キー |
| `Graphics/Resource/Model/Skeletal/Player/SkeletalAnimationPlayer.h` | `GetREFFrameDataList` |
| `GameObject/Component/Model/Renderer/...StaticRenderer.h/.cpp` / `...SkeletalRenderer.h/.cpp` | テーブルの要素を持つ(番号の割り当て・行列が変わったときだけ書く・解放) |
| `Shader/Model/Model.hlsli` / `ModelMeshletCulling.hlsli` / `Standard/ModelStandard.hlsli` | CB をやめてテーブルを読む |
| `Shader/Model/Static|Skeletal/Standard/Lit|UnLit/*_MS.hlsl`(4つ) | 同上 |
| `Shader/Model/Shadow/Cascade/ModelCascadeShadowMeshletCulling.hlsli` / `ModelCascadeShadow_AS.hlsl` / Static・Skeletal の影の MS | 同上 |
| `CONFIG/Graphics/GraphicsCONFIG.json` | ルートシグネチャ2つ・PSO の AS のパス・`ModelRenderSystem` の容量・`RenderGraphPassList` に `ModelRenderTableUploadPass`(追加済み) |
| `Framework.vcxproj` / `.filters` | DXCTask の AS を差し替え |

### 登録

- フィルター: `Source\Framework\Definition\Struct\Graphics\Buffer\Root` / `Source\Framework\Graphics\Render\Model` / `...\Model\Converter` / `...\Model\Converter\Json`
  / `...\Render\Model\Table` / `Source\Framework\Definition\Macros\Graphics` / `Source\Framework\Definition\Concept\IsModelRenderTableElement`
  / `Source\Framework\Graphics\Resource\Model\Material\Standard` / `...\Standard\Lit` / `...\Standard\UnLit`(改訂の分は登録済み)
  / `Source\Framework\Graphics\Render\Graph\Pass\Model\Table`(登録済み)
- Framework.h(「モデルの描画テーブル」の見出しの下、`GPUElementTable.h` より後、`Renderer.h` より前):
  - Constant : `ModelRenderSystemConstant.h`(`ModelPerObjectConstantBufferUploaderConstant.h` の行を置き換える)
  - Struct : `RCModelStruct.h` → `ModelRenderSystemStruct.h` → `ModelRenderTableINFORegistryStruct.h`
  - Concept : `IsModelRenderTableElementConcept.h`
  - マクロ : `ModelRenderTableINFORegistryMacros.h`
  - `ModelRenderTableINFORegistry.h` → `ModelObjectGPUData.h` → `ModelMeshGPUData.h` → `ModelStandardLitMaterialGPUData.h` → `ModelStandardUnLitMaterialGPUData.h`
    (GPU データのクラスはマクロの中でレジストリを呼ぶので、レジストリより後)
  - `ModelRenderSystemJsonConverter.h` → `ModelRenderSystem.h`
- Framework.h(パス): `ModelRenderTableUploadPass.h` を `SkeletalAnimationComputePass.h` の前(登録済み)。
- DXCTask: `Shader\Model\Model_AS.hlsl`(ShaderType = Amplification)を足し、Static / Skeletal の AS を消す。

---

## コード(C++)

### Definition/Macros/Graphics/ModelRenderTableINFORegistryMacros.h(新規・骨組みでこちらが書く)

```cpp
#pragma once

// GPUのテーブル(GPUElementTable)の1要素として送るクラスに書くマクロ
// ※注意 : TypeINFOのマクロと同じく、クラスの一番下に書くこと(最後にprivateへ戻すため)
// 書くだけで、次の3つが行われる
// 1. FWK_DEFINE_TYPE_INFO_SINGLE : 型の名前・StaticTypeIDをTypeINFORegistryへ登録する(仮想関数は増えないので、GPUへ送る並びは変わらない)
// 2. GetREFModelRenderTableINFO() : 上のTypeINFOへのポインタ・1要素の大きさ(sizeof)・マテリアルかどうかを返す
// 3. アプリの起動時(mainより前)に、ModelRenderTableINFORegistryへ自動で登録する
// ModelRenderSystemは登録された一覧からテーブルを作るため、種類を増やしてもModelRenderSystemは書き換えない
// 例 : トゥーンを足すときは、ModelToonMaterialGPUDataにFWK_DEFINE_MODEL_MATERIAL_RENDER_TABLE_INFOを書くだけでテーブルが増える
#define FWK_DEFINE_MODEL_RENDER_TABLE_INFO_BASE(Type, IsMaterial)                                                                                                                  \
    FWK_DEFINE_TYPE_INFO_SINGLE(Type)                                                                                                                                              \
                                                                                                                                                                                   \
public:                                                                                                                                                                            \
                                                                                                                                                                                   \
    static const auto& GetREFModelRenderTableINFO()                                                                                                                                \
    {                                                                                                                                                                              \
        static_assert(std::is_trivially_copyable_v<Type>, "テーブルの要素の型は、memcpyでGPUへ送るため、triviallyCopyableである必要があります。");                                        \
        static_assert(std::is_standard_layout_v<Type>,    "テーブルの要素の型は、HLSLの構造体と並びを合わせるため、standardLayoutである必要があります。");                                        \
                                                                                                                                                                                   \
        static const auto l_modelRenderTableINFO = FWK::Struct::ModelRenderTableINFO{ &GetREFTypeINFO(), static_cast<UINT>(sizeof(Type)), IsMaterial };                           \
                                                                                                                                                                                   \
        return l_modelRenderTableINFO;                                                                                                                                             \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    static constexpr bool k_isModelMaterialRenderTableElement = IsMaterial;                                                                                                        \
                                                                                                                                                                                   \
private:                                                                                                                                                                           \
                                                                                                                                                                                   \
    class RegisterModelRenderTableINFO                                                                                                                                             \
    {                                                                                                                                                                              \
    public:                                                                                                                                                                        \
                                                                                                                                                                                   \
        RegisterModelRenderTableINFO()                                                                                                                                             \
        {                                                                                                                                                                          \
            FWK::Graphics::ModelRenderTableINFORegistry::GetInstance().Register(GetREFModelRenderTableINFO());                                                                     \
        }                                                                                                                                                                          \
        ~RegisterModelRenderTableINFO() = default;                                                                                                                                 \
    };                                                                                                                                                                             \
                                                                                                                                                                                   \
    inline static const RegisterModelRenderTableINFO k_autoRegisterModelRenderTableINFO = {};

// オブジェクト・メッシュのように、マテリアルではないテーブルの要素に書く
#define FWK_DEFINE_MODEL_RENDER_TABLE_INFO(Type) FWK_DEFINE_MODEL_RENDER_TABLE_INFO_BASE(Type, false)

// マテリアルのテーブルの要素に書く
// マテリアルのテーブルは、ModelRenderSystemが「メッシュの種類(Static / Skeletal) × マテリアルの種類」ごとに描画項目の一覧も作る
#define FWK_DEFINE_MODEL_MATERIAL_RENDER_TABLE_INFO(Type) FWK_DEFINE_MODEL_RENDER_TABLE_INFO_BASE(Type, true)
```

> - 仕組みは `FWK_DEFINE_TYPE_INFO` と同じ(上の「テーブルの種類を、型に書いたマクロで登録する」)。
> - `static_assert` を関数の中に書いているのは、クラスの `{ }` の中ではまだ型が完成していない(`sizeof` が使えない)ため。関数の本体の中は「型が完成した後」として扱われる。
> - `k_isModelMaterialRenderTableElement` は、S5 の `RecordDraw<型>` で「マテリアルではない型を渡したらコンパイルエラー」にするために使う(Concept)。

### Definition/Struct/Graphics/ModelRenderTableINFORegistryStruct.h(新規)

```cpp
#pragma once

namespace FWK::Struct
{
    struct ModelRenderTableINFO final
    {
        explicit ModelRenderTableINFO(const TypeINFO* const a_typeINFO,
                                      const UINT            a_elementByteStride,
                                      const bool            a_isMaterial) :
            k_typeINFO(a_typeINFO),

            k_elementByteStride(a_elementByteStride),

            k_isMaterial(a_isMaterial)
        {}
        ~ModelRenderTableINFO() = default;

        ModelRenderTableINFO(const ModelRenderTableINFO&)  = delete;
        ModelRenderTableINFO(      ModelRenderTableINFO&&) = delete;

        ModelRenderTableINFO& operator=(const ModelRenderTableINFO&)  = delete;
        ModelRenderTableINFO& operator=(      ModelRenderTableINFO&&) = delete;

        const TypeINFO* const k_typeINFO;

        const UINT k_elementByteStride;

        const bool k_isMaterial;
    };
}
```

> `Struct::TypeINFO` と同じ形(作った後は変えないので const のメンバ、コピー禁止)。
> 実体はマクロの関数の中の `static` 変数で、アプリの終了まで消えないので、レジストリはポインタで持ってよい。

### Definition/Concept/IsModelRenderTableElement/IsModelRenderTableElementConcept.h(新規)

```cpp
#pragma once

namespace FWK::Concept
{
    template <typename Type>
    concept IsModelRenderTableElementConcept = requires
    {
        { Type::GetREFModelRenderTableINFO() } -> std::same_as<const Struct::ModelRenderTableINFO&>;
    };

    template <typename Type>
    concept IsModelMaterialRenderTableElementConcept = IsModelRenderTableElementConcept<Type> &&
                                                       Type::k_isModelMaterialRenderTableElement;
}
```

> - `IsModelRenderTableElementConcept` : マクロを書いたクラスだけを受け付ける(`FindVALTable<型>()` など)。
> - `IsModelMaterialRenderTableElementConcept` : その中でも、マテリアルの版のマクロを書いたクラスだけを受け付ける(S5 の `RecordDraw<型>()`)。

### Graphics/Render/Model/Table/ModelRenderTableINFORegistry.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelRenderTableINFORegistry final : public Utility::SingletonBase<ModelRenderTableINFORegistry>
    {
    private:

        using ModelRenderTableINFONameMap = std::unordered_map<std::string_view, const Struct::ModelRenderTableINFO* const, Struct::StringHash, std::equal_to<>>;

        friend class SingletonBase<ModelRenderTableINFORegistry>;

         ModelRenderTableINFORegistry()          = default;
        ~ModelRenderTableINFORegistry() override = default;

    public:

        void Register(const Struct::ModelRenderTableINFO& a_tableINFO);

        const auto& GetREFTableINFONameMap() const { return m_tableINFONameMap; }

    private:

        ModelRenderTableINFONameMap m_tableINFONameMap = {};
    };
}
```

> シングルトンにする理由 : 登録は main より前(ModelRenderSystem も Renderer もまだ無い)に起きるので、
> 「最初に呼ばれたときに作られる」入れ物が要る(`TypeINFORegistry` と同じ)。
>
> `std::vector` ではなく、型の名前をキーにした `std::unordered_map` で持つ理由(2026-10-10 ユーザーの指摘で変更):
> - 同じ名前の登録を弾く処理が、`try_emplace(...).second` の1回で済む(vector では毎回 `any_of` で全部を比べていた)。
> - `TypeINFORegistry::m_typeINFONameMap` と同じ形になる(`Struct::StringHash` + `std::equal_to<>` で、`std::string_view` のまま探せる)。
> - 並び順は意味を持たない(静的な変数の初期化の順番は、別の .cpp の間では決まっていないので、vector でも順番は保証されていなかった)。

### Graphics/Render/Model/Table/ModelRenderTableINFORegistry.cpp(新規・写経)

```cpp
#include "ModelRenderTableINFORegistry.h"

// テーブルの要素の型(ModelObjectGPUDataなど)の情報を、起動時に集めておくクラス
// FWK_DEFINE_MODEL_RENDER_TABLE_INFOを書いたクラスが、mainより前に自分で登録しに来る
// ModelRenderSystemJsonConverterが、GraphicsCONFIG.jsonに書かれた型の名前でこの一覧から情報を探し、テーブルを追加する(種類ごとのswitchが要らない)
void FWK::Graphics::ModelRenderTableINFORegistry::Register(const Struct::ModelRenderTableINFO& a_tableINFO)
{
    FWK_ASSERT_RETURN_IF(!a_tableINFO.k_typeINFO,                                                                "TypeINFOが無効のため、テーブルの情報の登録に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_tableINFO.k_typeINFO->k_staticTypeID == StaticTypeIDGenerator::k_invalidStaticTypeID, "無効なStaticTypeIDを検出したため、テーブルの情報の登録に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_tableINFO.k_typeINFO->k_name.empty(),                                                 "型の名前が空のため、テーブルの情報の登録に失敗しました。");

    // 型の名前をキーにして登録する
    // GraphicsCONFIG.jsonの容量は型の名前で探すため、同じ名前が2つあると区別できない
    // (別のnamespaceに、同じ名前のクラスを作ったときなど)
    // try_emplaceは、同じキーが既にあれば何もせず、戻り値のsecondがfalseになる(重複の確認と登録が1回で済む)
    // 情報の実体は、マクロが作った関数の中のstatic変数(アプリの終了まで消えない)なので、ポインタで持ってよい
    // キーのstring_viewも、TypeINFOのk_name(static)を指すので消えない
    const bool l_isRegistered = m_tableINFONameMap.try_emplace(a_tableINFO.k_typeINFO->k_name, &a_tableINFO).second;

    FWK_ASSERT_RETURN_IF(!l_isRegistered, "同じ名前のテーブルの情報が登録済みのため、テーブルの情報の登録に失敗しました。");

    FWK_ADD_LOG(Constant::k_imguiDebugSuccessColor,
                "[テーブルの情報の登録]\nName : {}\nElementByteStride : {}\nテーブルの情報の登録に成功しました。\n",
                a_tableINFO.k_typeINFO->k_name.data(),
                a_tableINFO.k_elementByteStride);
}
```

### Definition/Constant/Graphics/ModelRenderSystemConstant.h(新規)

```cpp
#pragma once

namespace FWK::Constant
{
    inline constexpr float k_normalModelWorldOrientationSign          =  1.0F;
    inline constexpr float k_mirrorModelWorldOrientationSign          = -1.0F;
    inline constexpr float k_modelWorldOrientationDeterminantBoundary =  0.0F;
}
```

### Definition/Struct/Graphics/Buffer/Root/RCModelStruct.h(新規)

```cpp
#pragma once

namespace FWK::Struct
{
    struct RCModelDrawItem final
    {
        static constexpr std::uint32_t k_invalidIndex = std::numeric_limits<std::uint32_t>::max();

        std::uint32_t m_objectIndex   = k_invalidIndex;
        std::uint32_t m_meshIndex     = k_invalidIndex;
        std::uint32_t m_materialIndex = k_invalidIndex;
    };

    struct RCModelTable final
    {
        TypeAlias::DescriptorIndex m_objectTableSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshTableSRVDescriptorIndex   = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
    };

    struct RCModelMaterialTable final
    {
        TypeAlias::DescriptorIndex m_materialTableSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
    };
}
```

> ルート定数の構造体は、4 バイトの値だけを並べる(`SetupRoot32BitConstants` が 4 バイト単位で送るため)。
> HLSL の `cbuffer RCModelDrawItem` などと、並び順を必ず同じにする。

### Graphics/Render/Model/Table/ModelObjectGPUData.h / ModelMeshGPUData.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelObjectGPUData final
    {
    public:

         ModelObjectGPUData() = default;
        ~ModelObjectGPUData() = default;

        void SetWorldMatrix                (const TypeAlias::Math::Matrix& a_set) { m_worldMatrix                 = a_set; }
        void SetWorldInverseTransposeMatrix(const TypeAlias::Math::Matrix& a_set) { m_worldInverseTransposeMatrix = a_set; }

        void SetWorldMAXScale       (const float a_set) { m_worldMAXScale        = a_set; }
        void SetWorldOrientationSign(const float a_set) { m_worldOrientationSign = a_set; }

    private:

        static constexpr float k_initialWorldMAXScale = 1.0F;

        TypeAlias::Math::Matrix m_worldMatrix                 = TypeAlias::Math::Matrix::Identity;
        TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;

        float m_worldMAXScale        = k_initialWorldMAXScale;
        float m_worldOrientationSign = Constant::k_normalModelWorldOrientationSign;

        FWK_DEFINE_MODEL_RENDER_TABLE_INFO(ModelObjectGPUData)
    };
}
```

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelMeshGPUData final
    {
    public:

         ModelMeshGPUData() = default;
        ~ModelMeshGPUData() = default;

        void SetVertexBufferSRVDescriptorIndex           (const TypeAlias::DescriptorIndex a_set) { m_vertexBufferSRVDescriptorIndex            = a_set; }
        void SetMeshletBufferSRVDescriptorIndex          (const TypeAlias::DescriptorIndex a_set) { m_meshletBufferSRVDescriptorIndex           = a_set; }
        void SetUniqueVertexIndexBufferSRVDescriptorIndex(const TypeAlias::DescriptorIndex a_set) { m_uniqueVertexIndexBufferSRVDescriptorIndex = a_set; }
        void SetPrimitiveIndexBufferSRVDescriptorIndex   (const TypeAlias::DescriptorIndex a_set) { m_primitiveIndexBufferSRVDescriptorIndex    = a_set; }
        void SetMeshletBoundsBufferSRVDescriptorIndex    (const TypeAlias::DescriptorIndex a_set) { m_meshletBoundsBufferSRVDescriptorIndex     = a_set; }

        void SetMeshletCount(const std::uint32_t a_set) { m_meshletCount = a_set; }

    private:

        static constexpr std::uint32_t k_initialMeshletCount = 0U;

        TypeAlias::DescriptorIndex m_vertexBufferSRVDescriptorIndex            = DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBufferSRVDescriptorIndex           = DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_uniqueVertexIndexBufferSRVDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_primitiveIndexBufferSRVDescriptorIndex    = DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBoundsBufferSRVDescriptorIndex     = DescriptorHeap::k_invalidDescriptorIndex;

        std::uint32_t m_meshletCount = k_initialMeshletCount;

        FWK_DEFINE_MODEL_RENDER_TABLE_INFO(ModelMeshGPUData)
    };
}
```

### Graphics/Resource/Model/Material/Standard/Lit|UnLit/ModelStandard(Un)LitMaterialGPUData.h(新規)

マテリアルの GPU データは、S4 で作るマテリアルのクラスと同じフォルダに置く(マクロだけ MATERIAL 版)。

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelStandardLitMaterialGPUData final
    {
    public:

         ModelStandardLitMaterialGPUData() = default;
        ~ModelStandardLitMaterialGPUData() = default;

        void SetBaseColor(const TypeAlias::Math::Color& a_set) { m_baseColor = a_set; }

        void SetMetallic (const float a_set) { m_metallic  = a_set; }
        void SetRoughness(const float a_set) { m_roughness = a_set; }

        void SetBaseColorTextureSRVDescriptorIndex(const TypeAlias::DescriptorIndex a_set) { m_baseColorTextureSRVDescriptorIndex = a_set; }
        void SetNormalTextureSRVDescriptorIndex   (const TypeAlias::DescriptorIndex a_set) { m_normalTextureSRVDescriptorIndex    = a_set; }
        void SetMetallicTextureSRVDescriptorIndex (const TypeAlias::DescriptorIndex a_set) { m_metallicTextureSRVDescriptorIndex  = a_set; }
        void SetRoughnessTextureSRVDescriptorIndex(const TypeAlias::DescriptorIndex a_set) { m_roughnessTextureSRVDescriptorIndex = a_set; }

        static constexpr float k_defaultMetallic  = 0.0F;
        static constexpr float k_defaultRoughness = 1.0F;

    private:

        TypeAlias::Math::Color m_baseColor = Constant::k_whiteColor;

        float m_metallic  = k_defaultMetallic;
        float m_roughness = k_defaultRoughness;

        TypeAlias::DescriptorIndex m_baseColorTextureSRVDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_normalTextureSRVDescriptorIndex    = DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_metallicTextureSRVDescriptorIndex  = DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_roughnessTextureSRVDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;

        FWK_DEFINE_MODEL_MATERIAL_RENDER_TABLE_INFO(ModelStandardLitMaterialGPUData)
    };
}
```

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelStandardUnLitMaterialGPUData final
    {
    public:

         ModelStandardUnLitMaterialGPUData() = default;
        ~ModelStandardUnLitMaterialGPUData() = default;

        void SetBaseColor(const TypeAlias::Math::Color& a_set) { m_baseColor = a_set; }

        void SetBaseColorTextureSRVDescriptorIndex(const TypeAlias::DescriptorIndex a_set) { m_baseColorTextureSRVDescriptorIndex = a_set; }

    private:

        TypeAlias::Math::Color m_baseColor = Constant::k_whiteColor;

        TypeAlias::DescriptorIndex m_baseColorTextureSRVDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;

        FWK_DEFINE_MODEL_MATERIAL_RENDER_TABLE_INFO(ModelStandardUnLitMaterialGPUData)
    };
}
```

> - 大きさ: `ModelObjectGPUData` = 64 + 64 + 4 + 4 = **136 バイト**、`ModelMeshGPUData` = 4 × 6 = **24 バイト**、
>   `ModelStandardLitMaterialGPUData` = 16 + 4 + 4 + 4 × 4 = **40 バイト**、`ModelStandardUnLitMaterialGPUData` = 16 + 4 = **20 バイト**。
> - クラスにしてメンバを private にしても、**全部のメンバが同じアクセス指定(private)なら、メモリの並びは構造体と同じ**(standard layout)。
>   static のメンバ・入れ子のクラス・関数は、1要素の大きさにも並びにも入らない。マクロの `static_assert` がこれを確かめる。
> - StructuredBuffer は cbuffer と違って 16 バイト境界に揃えないので、パディングは入れない(規約 19-10)。
> - 値は Set 関数で詰める(規約 6-4)。取得関数は、使う所が無いので作らない(GPU が読むだけ)。
> - GPU のメモリ配置に合わせるクラスなので、メンバの並びは規約 20-6 の例外。

### Definition/Struct/Graphics/ModelRenderSystemStruct.h(新規)

```cpp
#pragma once

namespace FWK::Struct
{
    struct ModelDrawItem final
    {
        Struct::RCModelDrawItem m_rootConstant = {};

        D3D12_DISPATCH_MESH_ARGUMENTS m_dispatchMeshArguments = {};
    };
}
```

> `ModelDrawItem` は「描画1回ぶん」= ルート定数(12 バイト)+ `DispatchMesh` の引数(`D3D12_DISPATCH_MESH_ARGUMENTS` = X / Y / Z の 12 バイト)= **24 バイト**。
> S5 ではこれを CPU が読んで `SetGraphicsRoot32BitConstants` と `DispatchMesh` を呼ぶ。
> **S6 の ExecuteIndirect の引数のバッファの1件と同じ並びにしてある**ので、S6 では一覧をそのまま GPU のバッファへ memcpy するだけでよい(変換が要らない)。

### Graphics/Render/Model/ModelRenderSystem.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelRenderSystem final
    {
    private:

        using ModelRenderTableMap = std::unordered_map<TypeAlias::StaticTypeID, std::shared_ptr<GPUElementTable>>;

    public:

         ModelRenderSystem() = default;
        ~ModelRenderSystem() = default;

        ModelRenderSystem(const ModelRenderSystem&)  = delete;
        ModelRenderSystem(      ModelRenderSystem&&) = delete;

        ModelRenderSystem& operator=(const ModelRenderSystem&)  = delete;
        ModelRenderSystem& operator=(      ModelRenderSystem&&) = delete;

        void Deserialize(const nlohmann::json& a_rootJson);

        bool Create(const Device&                             a_device,
                    const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
                    const std::size_t&                        a_frameCount,
                          TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool);

        void RecordUpload(const DirectCommandList& a_directCommandList, const std::size_t& a_frameIndex) const;

        nlohmann::json Serialize() const;

        void AddTable(const Struct::ModelRenderTableINFO& a_tableINFO, const UINT a_capacity);

        Struct::RCModelTable FetchVALRCModelTable() const;

        template <Concept::IsModelRenderTableElementConcept ElementType>
        TypeAlias::DescriptorIndex FetchVALTableSRVDescriptorIndex() const
        {
            // 要素の型(ModelObjectGPUDataなど)から、マクロが作ったテーブルの情報を取り出し、
            // そのStaticTypeIDでテーブルを探して、SRVの番号を返す
            // 例 : FetchVALTableSRVDescriptorIndex<ModelObjectGPUData>() → オブジェクトのテーブルのSRVの番号
            const auto& l_tableINFO = ElementType::GetREFModelRenderTableINFO();

            return FetchVALTableSRVDescriptorIndex(l_tableINFO.k_typeINFO->k_staticTypeID);
        }

        TypeAlias::DescriptorIndex FetchVALTableSRVDescriptorIndex(const TypeAlias::StaticTypeID a_tableStaticTypeID) const;

        template <Concept::IsModelRenderTableElementConcept ElementType>
        std::weak_ptr<GPUElementTable> FindVALTable() const
        {
            // 要素の型から、その型のテーブルを探す
            // 例 : FindVALTable<ModelMeshGPUData>() → メッシュのテーブル
            // 型で指定するので、別の種類のテーブルを取り違えることがない(取り違えるとコンパイルエラーか、書き込みの大きさのアサートで気づける)
            const auto& l_tableINFO = ElementType::GetREFModelRenderTableINFO();

            return FindVALTable(l_tableINFO.k_typeINFO->k_staticTypeID);
        }

        std::weak_ptr<GPUElementTable> FindVALTable(const TypeAlias::StaticTypeID a_tableStaticTypeID) const;

        const auto& GetREFTableMap() const { return m_tableMap; }

    private:

        ModelRenderTableMap m_tableMap = {};

        Converter::ModelRenderSystemJsonConverter m_jsonConverter = {};
    };
}
```

> - テーブルは `shared_ptr` で持ち、使う側(ModelComponent の描き方・マテリアル)には `FindVALTable` で `weak_ptr` を渡す。
>   使う側は `lock()` してから番号の割り当て・書き込み・返却をする。
> - アプリの終了時は Renderer(このクラス)がマテリアル(ResourceContext)より先に破棄される。使う側が `weak_ptr` なら、
>   テーブルが先に消えていても `lock()` に失敗するだけで、壊れたメモリには触れない(撤回した設計で起きた終了時のアサートの対策)。
> - `FindVALTable` / `FetchVALTableSRVDescriptorIndex` は、型で指定するテンプレート版と、StaticTypeID で指定する版の2つがある。
>   型が決まっている所(描き方・パス)はテンプレート版、型が実行時に決まる所(マテリアルの基底クラス)は StaticTypeID 版を使う。

### Graphics/Render/Model/ModelRenderSystem.cpp(新規・写経)

> **改訂で書き直す所** : クラスの説明のコメント / `Create` / `AddTable`(新規) / `FetchVALRCModelTable`。`FetchVALCapacity` は削除した。
> `FetchVALTableSRVDescriptorIndex` / `FindVALTable` は、引数の名前を `a_tableStaticTypeID` に変えた(こちらで書き換え済み、中身は同じ)。
> `FetchVALElementByteStride` は消した(マクロが `sizeof` で決めるので要らない)。

```cpp
#include "ModelRenderSystem.h"

// モデルの描画に使うテーブル(GPUElementTable)を、種類ごとにまとめて持つクラス
// 種類 : FWK_DEFINE_MODEL_RENDER_TABLE_INFO(またはMATERIAL版)を書いたクラス1つにつき、テーブル1つ
//        例 : ModelObjectGPUData(行列) / ModelMeshGPUData(バッファのSRVの番号) / ModelStandardLitMaterialGPUData(マテリアルの値)
// 種類はStaticTypeID(型ごとに起動時に1つ決まる番号)で区別するため、種類が増えてもこのクラスは書き換えない
// 使う側(ModelComponentの描き方・マテリアル)は、FindVALTableでテーブルをweak_ptrで受け取り、
// 番号をもらって、値が変わったときだけ書く
// シェーダーは、ルート定数で受け取った番号でテーブルを引いて値を読む
// ※ 注意 : RendererはModelRenderSystemをコマンドキューより前に宣言している(テーブルの解放をGPUの完了後にするため)
void FWK::Graphics::ModelRenderSystem::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::ModelRenderSystem::Create(const Device&                             a_device,
                                              const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
                                              const std::size_t&                        a_frameCount,
                                                    TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
    // Deserialize(GraphicsCONFIG.jsonのTableMap)でAddTableされたテーブルだけを、GPUのメモリに作る
    // CONFIGに書いていない種類のテーブルは作られない(RenderGraphPassListに書いたパスだけが作られるのと同じ考え方)
    // 容量と1要素の大きさは、AddTableでテーブルに持たせてある
    for (const auto& [l_tableStaticTypeID, l_table] : m_tableMap)
    {
        FWK_ASSERT_RETURN_VALUE_IF(!l_table, "テーブルが無効のため、ModelRenderSystemの作成に失敗しました。", false);

        // DEFAULTヒープの本体・フレーム数分のUPLOADヒープ・SRVを作る(S2)
        const bool l_isCreated = l_table->Create(a_device,
                                                 a_gpuMemoryAllocator,
                                                 a_frameCount,
                                                 a_cbvSRVUAVDescriptorPool);

        FWK_ASSERT_RETURN_VALUE_IF(!l_isCreated, "ModelRenderSystemのテーブルの作成に失敗しました。", false);
    }

    // オブジェクトとメッシュのテーブルは、すべてのモデルの描画で使う
    // CONFIGに書き忘れていたら、毎フレームの描画で気づくのではなく、起動時に作成を失敗にする
    FWK_ASSERT_RETURN_VALUE_IF(FindVALTable<ModelObjectGPUData>().expired(), "GraphicsCONFIG.jsonにModelObjectGPUDataが無いため、ModelRenderSystemの作成に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF(FindVALTable<ModelMeshGPUData>  ().expired(), "GraphicsCONFIG.jsonにModelMeshGPUDataが無いため、ModelRenderSystemの作成に失敗しました。",   false);

    return true;
}

void FWK::Graphics::ModelRenderSystem::RecordUpload(const DirectCommandList& a_directCommandList, const std::size_t& a_frameIndex) const
{
    // すべてのテーブルについて、このフレームに書き換えた要素だけをGPUへコピーする命令を積む
    // 書き換えが無いテーブルは、何もしない
    for (const auto& [l_type, l_table] : m_tableMap)
    {
        if (!l_table) { continue; }

        l_table->RecordUpload(a_directCommandList, a_frameIndex);
    }
}

nlohmann::json FWK::Graphics::ModelRenderSystem::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::ModelRenderSystem::AddTable(const Struct::ModelRenderTableINFO& a_tableINFO, const UINT a_capacity)
{
    FWK_ASSERT_RETURN_IF(!a_tableINFO.k_typeINFO, "TypeINFOが無効のため、テーブルの追加に失敗しました。");

    // 種類はStaticTypeIDで区別する(FindVALTableもこの番号で探す)
    // 同じ型の名前がCONFIGに2回書かれていると、片方が使われないテーブルになるので弾く
    const auto& l_tableStaticTypeID = a_tableINFO.k_typeINFO->k_staticTypeID;

    FWK_ASSERT_RETURN_IF(m_tableMap.contains(l_tableStaticTypeID), "同じ種類のテーブルが追加済みのため、テーブルの追加に失敗しました。");

    // ここではGPUのメモリはまだ作らず、容量と1要素の大きさだけを持たせる(GPUのメモリはCreateで作る)
    // 1要素の大きさは、マクロがsizeof(型)で決めた値(HLSLのStructuredBufferの要素と同じ大きさ・同じ並び)
    auto l_table = std::make_shared<GPUElementTable>();

    l_table->SetCapacity         (a_capacity);
    l_table->SetElementByteStride(a_tableINFO.k_elementByteStride);

    m_tableMap.try_emplace(l_tableStaticTypeID, std::move(l_table));
}

FWK::Struct::RCModelTable FWK::Graphics::ModelRenderSystem::FetchVALRCModelTable() const
{
    // パスの最初に1回だけ送る、オブジェクトとメッシュのテーブルのSRVの番号
    // シェーダーはこの番号でテーブル(StructuredBuffer)を取り出し、描画ごとの番号で要素を読む
    // テーブルは要素の型で指定する(種類のenumを用意しなくてよい)
    Struct::RCModelTable l_rcModelTable = {};

    l_rcModelTable.m_objectTableSRVDescriptorIndex = FetchVALTableSRVDescriptorIndex<ModelObjectGPUData>();
    l_rcModelTable.m_meshTableSRVDescriptorIndex   = FetchVALTableSRVDescriptorIndex<ModelMeshGPUData>  ();

    return l_rcModelTable;
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::ModelRenderSystem::FetchVALTableSRVDescriptorIndex(const TypeAlias::StaticTypeID a_tableStaticTypeID) const
{
    const auto& l_tableITR = m_tableMap.find(a_tableStaticTypeID);

    FWK_ASSERT_RETURN_VALUE_IF(l_tableITR == m_tableMap.end(), "対応するテーブルが無いため、テーブルのSRVの番号の取得に失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);

    const auto& l_table = l_tableITR->second;

    FWK_ASSERT_RETURN_VALUE_IF(!l_table, "テーブルが無効のため、テーブルのSRVの番号の取得に失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);

    return l_table->GetVALSRVDescriptorIndex();
}

std::weak_ptr<FWK::Graphics::GPUElementTable> FWK::Graphics::ModelRenderSystem::FindVALTable(const TypeAlias::StaticTypeID a_tableStaticTypeID) const
{
    const auto& l_tableITR = m_tableMap.find(a_tableStaticTypeID);

    if (l_tableITR == m_tableMap.end()) { return {}; }

    return l_tableITR->second;
}
```

> - **テーブルを作るのは CONFIG に書いた種類だけ**(2026-10-10 ユーザー指示)。Renderer の RootSignatureMap と同じく、
>   Deserialize で map に入れ(`AddTable`)、Create では map を回して GPU のメモリを作るだけ。設定だけを持つ vector は持たない。
> - `AddTable` は Deserialize の中から呼ばれる(Device がまだ無いので、GPU のメモリは作れない)。そのため容量と1要素の大きさを
>   `SetCapacity` / `SetElementByteStride` で先に持たせておき、Create でそれを使う(S2 の改訂)。
> - 重複(`m_tableMap.contains`、C++20 の関数。キーがあれば true)と Object / Mesh の書き忘れはアサートで分かる。
>   マテリアルの書き忘れは、そのマテリアルの `CreateGPUData`(S4)のアサートで分かる。
> - `for (const auto& [l_tableStaticTypeID, l_table] : m_tableMap)` : unordered_map を回す順番は決まっていないので、SRV の番号の順番も決まっていない。
>   シェーダーは SRV の番号をルート定数(RCModelTable)で受け取るので、順番が変わっても描画は変わらない。

### Graphics/Render/Model/Converter/Json/ModelRenderSystemJsonConverter.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelRenderSystem;
}

namespace FWK::Converter
{
    class ModelRenderSystemJsonConverter final
    {
    public:

         ModelRenderSystemJsonConverter() = default;
        ~ModelRenderSystemJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Graphics::ModelRenderSystem& a_modelRenderSystem) const;

        nlohmann::json Serialize(const Graphics::ModelRenderSystem& a_modelRenderSystem) const;

    private:

        static constexpr std::string_view k_tableMapJsonKey = "TableMap";
        static constexpr std::string_view k_typeNameJsonKey = "TypeName";
        static constexpr std::string_view k_capacityJsonKey = "Capacity";

        static constexpr UINT k_defaultCapacity = 1024U;
    };
}
```

### Graphics/Render/Model/Converter/Json/ModelRenderSystemJsonConverter.cpp(新規・写経)

```cpp
#include "ModelRenderSystemJsonConverter.h"

void FWK::Converter::ModelRenderSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::ModelRenderSystem& a_modelRenderSystem) const
{
    if (a_rootJson.is_null()) { return; }

    if (!Utility::IsJsonArray(a_rootJson, k_tableMapJsonKey)) { return; }

    // 起動時(mainより前)に、マクロを書いたクラスが自分で登録した「型の名前 → テーブルの情報」のmap
    // 例 : "ModelObjectGPUData"(136バイト) / "ModelMeshGPUData"(24バイト) /
    //      "ModelStandardLitMaterialGPUData"(40バイト) / "ModelStandardUnLitMaterialGPUData"(20バイト)
    const auto& l_tableINFORegistry = Graphics::ModelRenderTableINFORegistry::GetInstance ();
    const auto& l_tableINFONameMap  = l_tableINFORegistry.GetREFTableINFONameMap();

    // 1件 = { "Capacity" : 4096, "TypeName" : "ModelObjectGPUData" }
    // 名前はマクロの#Type(クラスの名前そのもの)と同じ文字列にする
    for (const auto& l_json : a_rootJson[k_tableMapJsonKey])
    {
        // 型の名前で、マクロが登録したテーブルの情報を探す
        // mapのキーが型の名前なので、全部を比べずに1回で見つかる
        // (StringHashとstd::equal_to<>にしてあるので、std::stringのまま探せる)
        const auto& l_typeName     = l_json.value           (k_typeNameJsonKey, std::string{});
        const auto& l_tableINFOITR = l_tableINFONameMap.find(l_typeName);

        FWK_ASSERT_RETURN_IF(l_tableINFOITR == l_tableINFONameMap.end(), "GraphicsCONFIG.jsonに登録されていない型の名前が書かれているため、ModelRenderSystemの読み込みに失敗しました。");
        FWK_ASSERT_RETURN_IF(!l_tableINFOITR->second,                     "テーブルの情報が無効のため、ModelRenderSystemの読み込みに失敗しました。");

        // 容量を省いたときは1024にする
        const auto& l_tableINFO = *l_tableINFOITR->second;
        const auto  l_capacity  = l_json.value(k_capacityJsonKey, k_defaultCapacity);

        a_modelRenderSystem.AddTable(l_tableINFO, l_capacity);
    }
}

nlohmann::json FWK::Converter::ModelRenderSystemJsonConverter::Serialize(const Graphics::ModelRenderSystem& a_modelRenderSystem) const
{
          nlohmann::json l_rootJson         = {};
          nlohmann::json l_tableJsonArray   = nlohmann::json::array();
    const auto&          l_typeINFORegistry = TypeINFORegistry::GetInstance       ();
    const auto&          l_tableMap         = a_modelRenderSystem.GetREFTableMap();

    // テーブルのmap(StaticTypeID → テーブル)を、{ 型の名前, 容量 } の組で書き出す
    // StaticTypeIDは起動のたびに変わりうる番号なので、ファイルには型の名前で残す
    // 型の名前は、マクロの中のFWK_DEFINE_TYPE_INFO_SINGLEがTypeINFORegistryへ登録したものを、StaticTypeIDで引く
    for (const auto& [l_tableStaticTypeID, l_table] : l_tableMap)
    {
        if (!l_table) { continue; }

        const auto* l_typeINFO = l_typeINFORegistry.FindPTRByID(l_tableStaticTypeID);

        if (!l_typeINFO) { continue; }

        nlohmann::json l_json = {};

        l_json[k_typeNameJsonKey] = l_typeINFO->k_name;
        l_json[k_capacityJsonKey] = l_table->GetVALCapacity();

        l_tableJsonArray.emplace_back(std::move(l_json));
    }

    l_rootJson[k_tableMapJsonKey] = std::move(l_tableJsonArray);

    return l_rootJson;
}
```

> - `Utility::IsJsonArray(json, key)` は `Utility/Json/JsonUtility.h` にある(キーを渡すと、その子が配列かを調べる)。
> - 書き間違いの名前を見つけたら、そこで読み込みをやめる(アサートで知らせる)。そのあと Create の「Object / Mesh が無い」アサートが出ることもある。
> - `l_json.value(k_capacityJsonKey, k_defaultCapacity)` : キーが無ければ第2引数を返す。第2引数が `UINT` なので、戻り値も `UINT`。
> - Serialize の順番は unordered_map を回す順なので毎回同じとは限らない。順番に意味は無いので問題ない(読み込むときは名前で探す)。

### Graphics/Command/List/Direct/DirectCommandList.h(変更)

`SetupConstantBufferView` の後ろに追加する(Compute 側と同じ形)。

```cpp
        void SetupConstantBufferView(const RootSignature& a_rootSignature, const D3D12_GPU_VIRTUAL_ADDRESS& a_gpuVirtualAddress, const Enum::RootParameterType a_rootParameterType) const override;

        template <typename RootConstantType>
        void SetupRoot32BitConstants(const RootConstantType& a_rootConstantData, const RootSignature& a_rootSignature, const Enum::RootParameterType a_rootParameterType) const
        {
            // コマンドリストへバイト列として記録するため、memcpyできる単純なデータ型だけを許可する
            static_assert(std::is_trivially_copyable_v<RootConstantType>, "Root32BitConstantsへ渡す型は、triviallyCopyableである必要があります。");

            // ルート定数は32ビット(4バイト)単位で送る
            // そのため、構造体の大きさが4バイトで割り切れなければ使えない
            // 例 : RCModelDrawItem(uint × 3 = 12バイト)なら、3個の値として送る
            static_assert(sizeof(RootConstantType) % sizeof(std::uint32_t) == static_cast<std::size_t>(Constant::k_noRemainder));

            constexpr auto l_rootConstantCount = static_cast<UINT>(sizeof(RootConstantType) / sizeof(std::uint32_t));

            SetupRoot32BitConstants(a_rootSignature,
                                    &a_rootConstantData,
                                    a_rootParameterType,
                                    l_rootConstantCount,
                                    k_rootConstantStartOffset);
        }
```

private に追加:

```cpp
        void SetupRoot32BitConstants(const RootSignature&          a_rootSignature,
                                     const void*                   a_rootConstantData,
                                     const Enum::RootParameterType a_rootParameterType,
                                     const UINT                    a_rootConstantCount,
                                     const UINT                    a_destinationOffset) const;

        static constexpr UINT k_rootConstantStartOffset  = 0U;
        static constexpr UINT k_invalidRootConstantCount = 0U;
```

### Graphics/Command/List/Direct/DirectCommandList.cpp(変更・写経)

```cpp
void FWK::Graphics::DirectCommandList::SetupRoot32BitConstants(const RootSignature&          a_rootSignature,
                                                               const void*                   a_rootConstantData,
                                                               const Enum::RootParameterType a_rootParameterType,
                                                               const UINT                    a_rootConstantCount,
                                                               const UINT                    a_destinationOffset) const
{
    FWK_ASSERT_RETURN_IF(!a_rootConstantData,                               "ルート定数のデータが無効のため、ルート定数の設定に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_rootConstantCount == k_invalidRootConstantCount, "ルート定数の個数が0のため、ルート定数の設定に失敗しました。");

    const auto& l_directCommandList = GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、ルート定数の設定に失敗しました。");

    // ルートシグネチャの中で、この種類のルートパラメーターが何番目かを探す
    // 例 : ModelStandardのルートシグネチャなら、RCModelDrawItemは1番
    const auto& l_rootParameterIndex = a_rootSignature.FindVALRootParameterIndex(a_rootParameterType);

    FWK_ASSERT_RETURN_IF(l_rootParameterIndex == Converter::RootSignatureJsonConverter::k_invalidRootParameterIndex, "ルートパラメーターの番号が無効なため、ルート定数の設定に失敗しました。");

    const auto& l_rootParameterRecordList = a_rootSignature.GetREFRootParameterRecordList();

    FWK_ASSERT_RETURN_IF(l_rootParameterIndex >= l_rootParameterRecordList.size(), "ルートパラメーターの番号が一覧の範囲外のため、ルート定数の設定に失敗しました。");

    const auto& l_rootParameter = l_rootParameterRecordList[l_rootParameterIndex].m_rootParameter;

    // 指定したルートパラメーターが、本当にルート定数(32BIT_CONSTANTS)かを確かめる
    // CBVなどの別の種類へ値を送ると、GPUが値をアドレスとして読んでしまい、壊れた描画やデバイスの消失になる
    FWK_ASSERT_RETURN_IF(l_rootParameter.ParameterType != D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,                           "指定したルートパラメーターがルート定数ではないため、ルート定数の設定に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_destinationOffset > l_rootParameter.Constants.Num32BitValues,                                       "書き込み開始位置がルート定数の数を超えているため、ルート定数の設定に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_rootConstantCount > l_rootParameter.Constants.Num32BitValues - a_destinationOffset,                 "送る値の数がルート定数の数を超えているため、ルート定数の設定に失敗しました。");

    // SetGraphicsRoot32BitConstants(ルートパラメーターの番号、
    //                               送る32ビットの値の個数、
    //                               送る値の先頭アドレス、
    //                               ルート定数の何番目の値から書くか);
    // 値そのものがコマンドリストに書き込まれるため、定数バッファのようにUPLOADへ書く必要がない
    l_directCommandList->SetGraphicsRoot32BitConstants(l_rootParameterIndex,
                                                       a_rootConstantCount,
                                                       a_rootConstantData,
                                                       a_destinationOffset);
}
```

### Definition/Enum/Graphics/RootParameterEnum.h(変更)

`CBFinalPresentPass` の後ろ(`Count` の前)に3つ足し、`FWK_JSON_SERIALIZE_ENUM` にも同じ3行を足す。

```cpp
        CBFinalPresentPass,
        RCModelDrawItem,
        RCModelTable,
        RCModelMaterialTable,
        Count,
```

### Graphics/Render/Renderer.h(変更)

```cpp
        const auto& GetREFModelRenderSystem() const { return m_modelRenderSystem; }

        auto& GetMutableREFModelRenderSystem() { return m_modelRenderSystem; }
```

```cpp
        GPUTimestampProfiler m_directGPUTimestampProfiler  = {};
        GPUTimestampProfiler m_computeGPUTimestampProfiler = {};

        ModelRenderSystem m_modelRenderSystem = {};

        TypeAlias::DirectCommandQueue  m_directCommandQueue  = {};
```

> S1 のプロファイラーと同じ理由で、**コマンドキューより前**に宣言する。

### Graphics/Render/Renderer.cpp(変更・写経)

**PostDeserialize(フレームリソースを作った後、ShadowContext の前など):**

```cpp
    // モデルの描画に使うテーブル(オブジェクト / メッシュ / マテリアル)を作る
    // テーブルはフレームの数だけUPLOADバッファを持つため、フレームリソースの数を渡す
    FWK_ASSERT_RETURN_VALUE_IF(!m_modelRenderSystem.Create(a_device,
                                                           l_gpuMemoryAllocator,
                                                           m_frameResourceList.size(),
                                                           l_cbvSRVUAVDescriptorPool),
                                                           "ModelRenderSystemの作成処理に失敗しました。",
                                                           false);
```

> BeginFrame には何も足さない。テーブルのコピーは、下の `ModelRenderTableUploadPass` が RenderGraph の一番前で行う。

### Definition/Enum/Graphics/RenderGraphPassEnum.h(変更・骨組みで追加済み)

```cpp
    enum class RenderGraphPassExecutionLayer
    {
        Invalid,
        Upload,
        Animation,
        ...
```

`FWK_JSON_SERIALIZE_ENUM` にも `FWK_JSON_ENUM_VALUE(RenderGraphPassExecutionLayer::Upload),` を `Invalid` の次に足してある。

> レイヤーの順番 = enum の並び(`RenderGraphPassSorter` が「値の小さいレイヤーを先」にする)。`Upload` は一番前。
> 「CPU で書いたデータを GPU へ送る」という大まかな種類なので、enum の注意書き(大まかな種類で分ける)にも合う。

### Graphics/Render/Graph/Pass/Model/Table/ModelRenderTableUploadPass.h(新規・骨組みでこちらが書く)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelRenderTableUploadPass final : public RenderGraphPassBase
    {
    public:

         ModelRenderTableUploadPass();
        ~ModelRenderTableUploadPass() override;

        void Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph&) override;

        FWK_DEFINE_TYPE_INFO(ModelRenderTableUploadPass, RenderGraphPassBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::ModelRenderTableUploadPass)
```

> `FWK_REGISTER_FACTORY_METHOD` で、GraphicsCONFIG.json の `"RenderGraphPassTypeName": "ModelRenderTableUploadPass"` から作られる(ほかのパスと同じ)。

### Graphics/Render/Graph/Pass/Model/Table/ModelRenderTableUploadPass.cpp(新規・写経)

```cpp
#include "ModelRenderTableUploadPass.h"

// ModelRenderSystemが持つテーブル(GPUElementTable)のうち、このフレームまでに書き換えた要素だけをGPUへコピーするパス
// 実行レイヤーをUpload(一番前のレイヤー)にするので、RenderGraphがすべてのパスより前に実行する
// 例 : 100体のうち10体の行列が変わったフレームは、10要素(136バイト × 10)だけをコピーし、変わらないフレームは何もしない
// テーブルはRenderGraphが状態を管理するリソース(RenderTarget / DepthStencil / ShadowMap)ではないため、
// ReadXxx / WriteXxxは宣言せず、バリアはGPUElementTable::RecordUploadの中で張る(SkeletalAnimationComputePassのバッファと同じ)
FWK::Graphics::ModelRenderTableUploadPass::ModelRenderTableUploadPass()
{
    // すべてのモデルのパス(スキニング・影・Lit / UnLit)より前に実行する
    SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::Upload);

    // ビューの範囲は既定のMainViewOnlyのままにする
    // プレビューはメインビューの後に同じコマンドリストで描くので、メインビューで1回コピーすれば、プレビューも新しい値を読む
}
FWK::Graphics::ModelRenderTableUploadPass::~ModelRenderTableUploadPass() = default;

void FWK::Graphics::ModelRenderTableUploadPass::Execute(const ResourceContext&, Renderer& a_renderer, RenderGraph&)
{
    const auto& l_modelRenderSystem = a_renderer.GetREFModelRenderSystem        ();
    const auto& l_directCommandList = a_renderer.GetREFDirectCommandList        ();
    const auto& l_frameIndex        = a_renderer.GetREFCurrentFrameResourceIndex();

    // 書き換えた要素だけを、このフレームのUPLOADバッファ経由でテーブル本体へコピーする命令を積む
    // コピーとモデルのパスは同じダイレクトコマンドリストに積むので、GPUは積んだ順(コピー → 描画)に実行する
    l_modelRenderSystem.RecordUpload(l_directCommandList, l_frameIndex);
}
```

> - `GetREFCurrentFrameResourceIndex` : 今のフレームリソースの番号(0 ~ 2)。テーブルは UPLOAD バッファをフレームの数だけ持つので、どれを使うかをこの番号で選ぶ(S2)。
> - `RecordUpload` は `const` の関数(テーブルは `shared_ptr` の先を書き換えるので、ModelRenderSystem 自身は変わらない)なので、`GetREFModelRenderSystem`(const 版)でよい。

### Graphics/Render/Converter/Json/RendererJsonConverter(変更・写経)

`.h` の private 定数に `static constexpr std::string_view k_modelRenderSystemJsonKey = "ModelRenderSystem";` を足す。

`Deserialize`(ShadowContext の後):

```cpp
    // ModelRenderSystem(モデルのテーブルの容量)のデシリアライズ
    if (const auto& l_json = a_rootJson.value(k_modelRenderSystemJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_modelRenderSystem = a_renderer.GetMutableREFModelRenderSystem();

        l_modelRenderSystem.Deserialize(l_json);
    }
```

`Serialize`:

```cpp
    const auto& l_modelRenderSystem = a_renderer.GetREFModelRenderSystem();

    l_rootJson[k_modelRenderSystemJsonKey] = l_modelRenderSystem.Serialize();
```

### Graphics/Resource/Model/Skeletal/Player/SkeletalAnimationPlayer.h(変更)

```cpp
        const auto& GetREFFrameDataList() const { return m_frameDataList; }

        const auto& GetREFSkeletalAnimationModelRecord() const { return m_skeletalAnimationModelRecord; }
```

### GameObject/Component/Model/Renderer/Static/GameObjectModelComponentStaticRenderer.h(変更)

S0 のメンバに、テーブル(weak_ptr)・テーブルの番号・最後に書いた行列を足す。

```cpp
    public:

         GameObjectModelComponentStaticRenderer()          = default;
        ~GameObjectModelComponentStaticRenderer() override;

        ...

    private:

        void ReleaseTableElementList();

        std::vector<std::uint32_t> m_meshIndexList = {};

        std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData> m_drawRequestData = nullptr;

        std::weak_ptr<Graphics::GPUElementTable> m_objectTable = {};
        std::weak_ptr<Graphics::GPUElementTable> m_meshTable   = {};

        Graphics::StaticModel m_staticModel = {};

        TypeAlias::Math::Matrix m_lastWorldMatrix = TypeAlias::Math::Matrix::Identity;

        std::uint32_t m_objectIndex = Graphics::GPUElementTable::k_invalidElementIndex;

        bool m_isRegistered       = false;
        bool m_hasLastWorldMatrix = false;
```

> - デストラクタでテーブルの番号を返すため、デストラクタを `.cpp` に書く(`= default` をやめる)。
> - `m_drawRequestData` は S5 で消す(描画項目の登録に置き換える)。

### GameObject/Component/Model/Renderer/Static/GameObjectModelComponentStaticRenderer.cpp(変更・写経)

```cpp
FWK::GameObjectModelComponentStaticRenderer::~GameObjectModelComponentStaticRenderer()
{
    // テーブルの番号を返さないと、使用中のまま残り、いずれ割り当てられなくなる
    ReleaseTableElementList();
}

bool FWK::GameObjectModelComponentStaticRenderer::Load(const std::filesystem::path& a_filePath)
{
    if (!m_staticModel.Load(a_filePath)) { return false; }

    const auto& l_staticModelRecord = m_staticModel.GetREFStaticModelRecord().lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_staticModelRecord, "StaticModelRecordが無効のため、モデルの読み込みに失敗しました。", false);

    const auto& l_graphicsManager   = Graphics::GraphicsManager::GetInstance();
    const auto& l_renderer          = l_graphicsManager.GetREFRenderer      ();
    const auto& l_modelRenderSystem = l_renderer.GetREFModelRenderSystem    ();
    const auto& l_modelData         = l_staticModelRecord->GetREFModelData  ();

    // オブジェクトとメッシュのテーブルを受け取って覚えておく
    // weak_ptrなので、アプリの終了時にテーブルが先に消えても、このクラスが壊れたメモリに触ることはない
    // テーブルは、要素の型で指定する(型ごとにマクロが登録したテーブルを探す)
    m_objectTable = l_modelRenderSystem.FindVALTable<Graphics::ModelObjectGPUData>();
    m_meshTable   = l_modelRenderSystem.FindVALTable<Graphics::ModelMeshGPUData>  ();

    const auto& l_objectTable = m_objectTable.lock();
    const auto& l_meshTable   = m_meshTable.lock  ();

    FWK_ASSERT_RETURN_VALUE_IF(!l_objectTable, "オブジェクトのテーブルが無いため、モデルの読み込みに失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF(!l_meshTable,   "メッシュのテーブルが無いため、モデルの読み込みに失敗しました。",     false);

    // このモデル1体ぶんの、オブジェクトのテーブルの番号をもらう
    // 行列は、ApplyWorldMatrixで値が変わったときだけ、この番号の要素へ書く
    m_objectIndex = l_objectTable->AllocateElementIndex();

    FWK_ASSERT_RETURN_VALUE_IF(m_objectIndex == Graphics::GPUElementTable::k_invalidElementIndex, "オブジェクトのテーブルの番号の割り当てに失敗したため、モデルの読み込みに失敗しました。", false);

    // メッシュごとに、メッシュのテーブルの番号をもらい、バッファのSRVの番号とMeshletの数を書く
    // この値はモデルを捨てるまで変わらないので、ここで1回書いたら二度と書かない
    m_meshIndexList.reserve(l_modelData.m_meshList.size());

    for (const auto& l_modelMesh : l_modelData.m_meshList)
    {
        const auto& l_meshRuntimeData = l_modelMesh.m_meshRuntimeData;
        const auto  l_meshIndex       = l_meshTable->AllocateElementIndex();

        FWK_ASSERT_RETURN_VALUE_IF(l_meshIndex == Graphics::GPUElementTable::k_invalidElementIndex, "メッシュのテーブルの番号の割り当てに失敗したため、モデルの読み込みに失敗しました。", false);

        const auto l_vertexBufferSRVDescriptorIndex            = l_meshRuntimeData.m_vertexBuffer.GetVALSRVDescriptorIndex           ();
        const auto l_meshletBufferSRVDescriptorIndex           = l_meshRuntimeData.m_meshletBuffer.GetVALSRVDescriptorIndex          ();
        const auto l_uniqueVertexIndexBufferSRVDescriptorIndex = l_meshRuntimeData.m_uniqueVertexIndexBuffer.GetVALSRVDescriptorIndex();
        const auto l_primitiveIndexBufferSRVDescriptorIndex    = l_meshRuntimeData.m_primitiveIndexBuffer.GetVALSRVDescriptorIndex   ();
        const auto l_meshletBoundsBufferSRVDescriptorIndex     = l_meshRuntimeData.m_meshletBoundsBuffer.GetVALSRVDescriptorIndex    ();
        const auto l_meshletCount                              = static_cast<std::uint32_t>                                          (l_modelMesh.m_meshletData.m_meshletList.size());

        // メッシュのテーブルの1要素(ModelMeshGPUData)の値は、Set関数で詰める
        Graphics::ModelMeshGPUData l_meshGPUData = {};

        l_meshGPUData.SetVertexBufferSRVDescriptorIndex           (l_vertexBufferSRVDescriptorIndex);
        l_meshGPUData.SetMeshletBufferSRVDescriptorIndex          (l_meshletBufferSRVDescriptorIndex);
        l_meshGPUData.SetUniqueVertexIndexBufferSRVDescriptorIndex(l_uniqueVertexIndexBufferSRVDescriptorIndex);
        l_meshGPUData.SetPrimitiveIndexBufferSRVDescriptorIndex   (l_primitiveIndexBufferSRVDescriptorIndex);
        l_meshGPUData.SetMeshletBoundsBufferSRVDescriptorIndex    (l_meshletBoundsBufferSRVDescriptorIndex);
        l_meshGPUData.SetMeshletCount                             (l_meshletCount);

        l_meshTable->WriteElement(l_meshGPUData, l_meshIndex);

        m_meshIndexList.emplace_back(l_meshIndex);
    }

    // 描画申請のデータ(S5で描画項目の登録に置き換える)
    m_drawRequestData = std::make_shared<Struct::StaticModelPerObjectDrawRequestData>();

    m_drawRequestData->m_staticModelRecord = l_staticModelRecord;

    return true;
}
```

```cpp
void FWK::GameObjectModelComponentStaticRenderer::ApplyWorldMatrix(const TypeAlias::Math::Matrix& a_worldMatrix)
{
    if (m_objectIndex == Graphics::GPUElementTable::k_invalidElementIndex) { return; }

    // 行列が前回書いたものと同じなら、何もしない(動かないオブジェクトは、ここで毎フレーム終わる)
    // Matrixの==は16個の値をすべて比べる(64バイトの比較なので、テーブルへ書いてGPUへ送るよりずっと軽い)
    if (m_hasLastWorldMatrix &&
        m_lastWorldMatrix == a_worldMatrix)
    {
        return;
    }

    const auto& l_objectTable = m_objectTable.lock();

    if (!l_objectTable) { return; }

    m_lastWorldMatrix    = a_worldMatrix;
    m_hasLastWorldMatrix = true;

    // ワールド行列 : モデルの頂点(ローカル座標)をシーンの座標(ワールド座標)へ移す行列
    // 逆転置行列   : 法線を移すための行列。拡大縮小が均一でなくても、法線が面に垂直なまま保たれる
    // 最大の拡大率 : Meshletの境界球(カリング用)はローカル空間の半径なので、ワールドでの大きさにするため掛ける
    //                例 : X方向だけ2倍に伸ばしたら、半径も2倍として扱う(はみ出して消えるのを防ぐ)
    // 向きの符号   : 行列式が負 = 鏡に映したように裏返っている(拡大率に負の値がある)
    //                そのときは三角形の頂点の並び(表裏)が逆になるため、シェーダーで並びを入れ替えて元に戻す
    const auto& l_worldInverseTransposeMatrix = a_worldMatrix.Invert           ().Transpose();
    const auto  l_worldMAXScale               = Utility::CalculateWorldMAXScale(a_worldMatrix);
    const bool  l_isMirrored                  = a_worldMatrix.Determinant      () < Constant::k_modelWorldOrientationDeterminantBoundary;
    const float l_worldOrientationSign        = l_isMirrored ? Constant::k_mirrorModelWorldOrientationSign : Constant::k_normalModelWorldOrientationSign;

    // オブジェクトのテーブルの1要素(ModelObjectGPUData)の値は、Set関数で詰める
    Graphics::ModelObjectGPUData l_objectGPUData = {};

    l_objectGPUData.SetWorldMatrix                (a_worldMatrix);
    l_objectGPUData.SetWorldInverseTransposeMatrix(l_worldInverseTransposeMatrix);
    l_objectGPUData.SetWorldMAXScale              (l_worldMAXScale);
    l_objectGPUData.SetWorldOrientationSign       (l_worldOrientationSign);

    l_objectTable->WriteElement(l_objectGPUData, m_objectIndex);
}
```

```cpp
void FWK::GameObjectModelComponentStaticRenderer::ReleaseTableElementList()
{
    // テーブルが既に破棄されている(アプリの終了時)なら、返す先がないので番号を忘れるだけにする
    if (const auto& l_objectTable = m_objectTable.lock();
        l_objectTable)
    {
        l_objectTable->ReleaseElementIndex(m_objectIndex);
    }

    if (const auto& l_meshTable = m_meshTable.lock();
        l_meshTable)
    {
        for (const auto& l_meshIndex : m_meshIndexList)
        {
            l_meshTable->ReleaseElementIndex(l_meshIndex);
        }
    }

    m_objectIndex = Graphics::GPUElementTable::k_invalidElementIndex;

    m_meshIndexList.clear();

    m_hasLastWorldMatrix = false;
}
```

> `ReleaseElementIndex` は無効な番号(`k_invalidElementIndex`)を渡されると何もしないので、まだ割り当てていなくても呼んでよい。
> `Register` / `Unregister` は S5 で描画項目の登録に書き換える(S3 では S0 のまま)。

### GameObject/Component/Model/Renderer/Skeletal/GameObjectModelComponentSkeletalRenderer.h(変更)

```cpp
    private:

        void ReleaseTableElementList();

        std::vector<std::vector<std::uint32_t>> m_frameMeshIndexList = {};

        std::shared_ptr<Graphics::SkeletalAnimationPlayer>                       m_skeletalAnimationPlayer = nullptr;
        std::shared_ptr<Struct::SkeletalAnimationModelPerObjectDrawRequestData> m_drawRequestData         = nullptr;

        std::weak_ptr<Graphics::GPUElementTable> m_objectTable = {};
        std::weak_ptr<Graphics::GPUElementTable> m_meshTable   = {};

        Graphics::SkeletalAnimationModel m_skeletalAnimationModel = {};

        TypeAlias::Math::Matrix m_lastWorldMatrix = TypeAlias::Math::Matrix::Identity;

        std::uint32_t m_objectIndex = Graphics::GPUElementTable::k_invalidElementIndex;

        bool m_isRegistered       = false;
        bool m_hasLastWorldMatrix = false;
```

### GameObject/Component/Model/Renderer/Skeletal/GameObjectModelComponentSkeletalRenderer.cpp(変更・写経)

`Load` の、Player を作った後(`m_skeletalAnimationPlayer = std::move(...)` の後)に追加する。

```cpp
    const auto& l_graphicsManager   = Graphics::GraphicsManager::GetInstance        ();
    const auto& l_renderer          = l_graphicsManager.GetREFRenderer              ();
    const auto& l_modelRenderSystem = l_renderer.GetREFModelRenderSystem            ();
    const auto& l_frameDataList     = m_skeletalAnimationPlayer->GetREFFrameDataList();

    // テーブルは、要素の型で指定する(型ごとにマクロが登録したテーブルを探す)
    m_objectTable = l_modelRenderSystem.FindVALTable<Graphics::ModelObjectGPUData>();
    m_meshTable   = l_modelRenderSystem.FindVALTable<Graphics::ModelMeshGPUData>  ();

    const auto& l_objectTable = m_objectTable.lock();
    const auto& l_meshTable   = m_meshTable.lock  ();

    FWK_ASSERT_RETURN_VALUE_IF(!l_objectTable, "オブジェクトのテーブルが無いため、モデルの読み込みに失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF(!l_meshTable,   "メッシュのテーブルが無いため、モデルの読み込みに失敗しました。",     false);

    m_objectIndex = l_objectTable->AllocateElementIndex();

    FWK_ASSERT_RETURN_VALUE_IF(m_objectIndex == Graphics::GPUElementTable::k_invalidElementIndex, "オブジェクトのテーブルの番号の割り当てに失敗したため、モデルの読み込みに失敗しました。", false);

    // スキニング後の頂点とMeshletの境界は、フレームリソースごとに別のバッファ
    // そのため「フレーム × メッシュ」の数だけメッシュの要素を作り、描画のときは今のフレームの要素の番号を送る
    // 例 : フレームが3つ、メッシュが5つなら、15個の要素を作る(ここで1回書いたら二度と書かない)
    m_frameMeshIndexList.resize(l_frameDataList.size());

    for (std::size_t l_frameIndex = 0ULL; l_frameIndex < l_frameDataList.size(); ++l_frameIndex)
    {
        const auto& l_frameData     = l_frameDataList[l_frameIndex];
              auto& l_meshIndexList = m_frameMeshIndexList[l_frameIndex];

        l_meshIndexList.reserve(l_modelData.m_meshList.size());

        for (std::size_t l_meshListIndex = 0ULL; l_meshListIndex < l_modelData.m_meshList.size(); ++l_meshListIndex)
        {
            const auto& l_modelMesh       = l_modelData.m_meshList[l_meshListIndex];
            const auto& l_meshRuntimeData = l_modelMesh.m_meshRuntimeData;
            const auto  l_meshIndex       = l_meshTable->AllocateElementIndex();

            FWK_ASSERT_RETURN_VALUE_IF(l_meshIndex == Graphics::GPUElementTable::k_invalidElementIndex, "メッシュのテーブルの番号の割り当てに失敗したため、モデルの読み込みに失敗しました。", false);

            // 頂点は、スキニング後の頂点のバッファ(このフレーム用)を読む
            // Meshletの境界も、スキニング後の形に合わせて毎フレーム計算し直したもの(このフレーム用)を読む
            // Meshletの並びと三角形は、元のモデルと同じなので、モデルのバッファを読む
            const auto l_vertexBufferSRVDescriptorIndex            = l_frameData.m_skinnedVertexBufferList[l_meshListIndex].GetVALSRVDescriptorIndex();
            const auto l_meshletBufferSRVDescriptorIndex           = l_meshRuntimeData.m_meshletBuffer.GetVALSRVDescriptorIndex                     ();
            const auto l_uniqueVertexIndexBufferSRVDescriptorIndex = l_meshRuntimeData.m_uniqueVertexIndexBuffer.GetVALSRVDescriptorIndex           ();
            const auto l_primitiveIndexBufferSRVDescriptorIndex    = l_meshRuntimeData.m_primitiveIndexBuffer.GetVALSRVDescriptorIndex              ();
            const auto l_meshletBoundsBufferSRVDescriptorIndex     = l_frameData.m_meshletBoundsBufferList[l_meshListIndex].GetVALSRVDescriptorIndex();
            const auto l_meshletCount                              = static_cast<std::uint32_t>                                                     (l_modelMesh.m_meshletData.m_meshletList.size());

            Graphics::ModelMeshGPUData l_meshGPUData = {};

            l_meshGPUData.SetVertexBufferSRVDescriptorIndex           (l_vertexBufferSRVDescriptorIndex);
            l_meshGPUData.SetMeshletBufferSRVDescriptorIndex          (l_meshletBufferSRVDescriptorIndex);
            l_meshGPUData.SetUniqueVertexIndexBufferSRVDescriptorIndex(l_uniqueVertexIndexBufferSRVDescriptorIndex);
            l_meshGPUData.SetPrimitiveIndexBufferSRVDescriptorIndex   (l_primitiveIndexBufferSRVDescriptorIndex);
            l_meshGPUData.SetMeshletBoundsBufferSRVDescriptorIndex    (l_meshletBoundsBufferSRVDescriptorIndex);
            l_meshGPUData.SetMeshletCount                             (l_meshletCount);

            l_meshTable->WriteElement(l_meshGPUData, l_meshIndex);

            l_meshIndexList.emplace_back(l_meshIndex);
        }
    }
```

`ApplyWorldMatrix` / `ReleaseTableElementList` / デストラクタは Static と同じ形(メッシュの番号は `m_frameMeshIndexList` の二重ループで返す)。

---

## コード(HLSL)

### Shader/Model/Model.hlsli(全体を書き換え・写経)

```hlsl
#ifndef MODEL_HLSLI
#define MODEL_HLSLI
#include "ModelMeshlet.hlsli"

static const float k_modelPositionElementW  = 1.0F;
static const float k_modelDirectionElementW = 0.0F;

static const float k_modelNormalWorldOrientationSign = 1.0F;

// WorldMatrixのdeterminantが負となり
// TriangleのWindingが反転してる状態
static const float k_modelMirroredWorldOrientationSign = -1.0F;

// Frustumの側面Planeに対するSphere半径補正で使う
// sqrt(1.0 + tanFOV * tanFOV)の1.0部分
static const float k_modelFrustumPlaneNormalBaseLength = 1.0F;

static const uint k_modelMeshShaderThreadCountX = 32U;
static const uint k_modelMeshShaderThreadCountY = 1U;
static const uint k_modelMeshShaderThreadCountZ = 1U;

// 1個のAmplificationShaderGroupで
// 32個のMeshletを並列にカリングする
// C++側のConstant::k_meshletCountPerAmplificationShaderGroupと必ず同じ値にする
static const uint k_modelAmplificationShaderThreadCountX = 32U;
static const uint k_modelAmplificationShaderThreadCountY = 1U;
static const uint k_modelAmplificationShaderThreadCountZ = 1U;

static const uint k_modelAmplificationDispatchMeshGroupCountY = 1U;
static const uint k_modelAmplificationDispatchMeshGroupCountZ = 1U;

static const uint k_modelAmplificationInitialVisibleMeshletCount = 0U;

static const uint k_modelAmplificationVisibleMeshletCountIncrement = 1U;

static const uint k_modelAmplificationLeaderThreadIndex = 0U;

// 1つのAmplificationShaderGroupが可視判定を通過した
// MeshletIndexを子MeshShaderGroupへ渡すPayload
// 配列には可視MeshletIndexだけが先頭から連続して格納される
struct ModelAmplificationPayload
{
    uint meshletIndexList[k_modelAmplificationShaderThreadCountX];
};

// 1回の描画(メッシュ1つ)ごとに、C++がルート定数で送る番号
// C++側のStruct::RCModelDrawItemと同じ並びにする
// g_objectIndex   : オブジェクトのテーブルの何番目か(行列など)
// g_meshIndex     : メッシュのテーブルの何番目か(バッファのSRVの番号など)
// g_materialIndex : マテリアルのテーブルの何番目か(影のパスでは使わない)
cbuffer RCModelDrawItem : register(b1)
{
    uint g_objectIndex;
    uint g_meshIndex;
    uint g_materialIndex;
};

// オブジェクトとメッシュのテーブルのSRVの番号
// パスの最初に1回だけ送る
// C++側のStruct::RCModelTableと同じ並びにする
cbuffer RCModelTable : register(b5)
{
    uint g_objectTableSRVDescriptorIndex;
    uint g_meshTableSRVDescriptorIndex;
};

// オブジェクトのテーブルの1要素
// C++側のGraphics::ModelObjectGPUDataと同じ並び(136バイト)にする
// StructuredBufferの要素なので、cbufferのような16バイト境界のパディングは入れない
struct ModelObjectData
{
    row_major float4x4 worldMatrix;
    row_major float4x4 worldInverseTransposeMatrix;
    float              worldMAXScale;
    float              worldOrientationSign;
};

// メッシュのテーブルの1要素
// C++側のGraphics::ModelMeshGPUDataと同じ並び(24バイト)にする
struct ModelMeshData
{
    uint vertexBufferSRVDescriptorIndex;
    uint meshletBufferSRVDescriptorIndex;
    uint uniqueVertexIndexBufferSRVDescriptorIndex;
    uint primitiveIndexBufferSRVDescriptorIndex;
    uint meshletBoundsBufferSRVDescriptorIndex;
    uint meshletCount;
};

// この描画のオブジェクトの値(行列など)を、オブジェクトのテーブルから読む
ModelObjectData FetchModelObjectData()
{
    StructuredBuffer<ModelObjectData> l_objectTable = ResourceDescriptorHeap[g_objectTableSRVDescriptorIndex];

    return l_objectTable[g_objectIndex];
}

// この描画のメッシュの値(バッファのSRVの番号など)を、メッシュのテーブルから読む
ModelMeshData FetchModelMeshData()
{
    StructuredBuffer<ModelMeshData> l_meshTable = ResourceDescriptorHeap[g_meshTableSRVDescriptorIndex];

    return l_meshTable[g_meshIndex];
}

// 三角形1個分のPrimitiveIndexをuint3で取得する
// 3個Pack方式では、uint32_t1個に三角形1個分のPrimitiveIndexを入れている
// また、WorldMatrixのdeterminantが負の場合は、
// WorldTransformによってTriangleのWindingが反転する
// 戻り値のuint3は、元VertexBufferのIndexではなく、
// MeshShaderが出力したa_vertexListの何番目を使うかを表す
uint3 FetchModelPackedPrimitiveIndex(const ModelObjectData a_object, const ModelMeshData a_mesh, const uint a_packedPrimitiveIndex)
{
    StructuredBuffer<uint> l_packedPrimitiveIndexBuffer = ResourceDescriptorHeap[a_mesh.primitiveIndexBufferSRVDescriptorIndex];

    // uint一個に三角形一個分の
    // 三つのPrimitiveIndexがPackされている
    const uint  l_packedValue    = l_packedPrimitiveIndexBuffer[a_packedPrimitiveIndex];
    const uint3 l_primitiveIndex = DecodeModelPackedPrimitiveIndex(l_packedValue);

    // determinantが負のWorldMatrixでは
    // (0, 1, 2)のTriangleを、(0, 2, 1)
    // へ変更することでWindingを元に戻す
    if (a_object.worldOrientationSign == k_modelMirroredWorldOrientationSign) { return uint3(l_primitiveIndex.x, l_primitiveIndex.z, l_primitiveIndex.y); }

    return l_primitiveIndex;
}

// ModelのLocal座標をWorld座標へ変換する
// PBRではライト方向やカメラ方向をWorld空間で計算するため、worldPositionが必要
float3 TransformModelLocalPositionToWorld(const ModelObjectData a_object, const float3 a_localPosition)
{
    const float4 l_localPosition = float4(a_localPosition, k_modelPositionElementW);
    const float4 l_worldPosition = mul   (l_localPosition, a_object.worldMatrix);

    return l_worldPosition.xyz;
}

#endif // MODEL_HLSLI
```

### Shader/Model/ModelMeshletCulling.hlsli(変更・写経)

`g_worldMaxScale` / `g_worldInverseTransposeMatrix` / `g_meshletBoundsBufferSRVDescriptorIndex` を読んでいた所を、引数で受け取る形にする。

```hlsl
// 指定したMeshletがFrustum内にあるか判定する
bool IsVisibleModelMeshletByFrustum(const ModelObjectData a_object, const ModelMeshletBounds a_modelMeshletBounds)
{
    const float3 l_worldCenter = TransformModelLocalPositionToWorld(a_object, a_modelMeshletBounds.center);
    const float  l_worldRadius = a_modelMeshletBounds.radius * a_object.worldMAXScale;

    // (ここから下は今までと同じ)
    ...
}

bool IsBackfaceModelMeshletByCone(const ModelObjectData a_object, const ModelMeshletBounds a_meshletBounds)
{
    // (先頭は今までと同じ)
    ...

    // worldInverseTransposeMatrixには
    // transpose(inverse(WorldMatrix))が格納されている
    // 再びtransposeしてWorld逆行列へ戻し
    // カメラをModelLocal空間へ変換する
    const float4 l_worldCameraPosition = float4(g_cullingCameraWorldPosition, k_modelPositionElementW);
    const float4 l_localCameraPosition = float4(mul(l_worldCameraPosition, transpose(a_object.worldInverseTransposeMatrix)));

    // (ここから下は今までと同じ)
    ...
}

// FrustumCullingとBackfaceConeCullingを実行し、
// MeshShaderを起動する必要があるかを判定する
bool ShouldDispatchModelMeshlet(const ModelObjectData a_object, const ModelMeshData a_mesh, const uint a_meshletIndex)
{
    StructuredBuffer<ModelMeshletBounds> l_meshletBoundsBuffer = ResourceDescriptorHeap[a_mesh.meshletBoundsBufferSRVDescriptorIndex];

    const ModelMeshletBounds l_meshletBounds = l_meshletBoundsBuffer[a_meshletIndex];

    // Frustum外ならBackface判定を行わず描画しない
    if (!IsVisibleModelMeshletByFrustum(a_object, l_meshletBounds)) { return false; }

    // Frustum内でも全Triangleが裏向きなら描画しない
    if (IsBackfaceModelMeshletByCone(a_object, l_meshletBounds)) { return false; }

    return true;
}
```

### Shader/Model/Model_AS.hlsl(新規・写経)

```hlsl
#include "ModelMeshletCulling.hlsli"

groupshared ModelAmplificationPayload g_modelAmplificationPayload;
groupshared uint                      g_modelVisibleMeshletCount;

// Static / Skeletal 共通のAmplificationShader
// どちらも「メッシュのテーブルが指すバッファ」を読むだけなので、同じシェーダーで済む
// (Skeletalは、メッシュのテーブルがスキニング後の頂点と、計算し直したMeshletの境界を指している)
[numthreads(k_modelAmplificationShaderThreadCountX, k_modelAmplificationShaderThreadCountY, k_modelAmplificationShaderThreadCountZ)]
void main(const uint3 a_dispatchThreadID : SV_DispatchThreadID,
          const uint  a_groupThreadIndex : SV_GroupIndex)
{
    // Group内の代表Threadだけが可視Meshlet数を初期化する
    if (a_groupThreadIndex == k_modelAmplificationLeaderThreadIndex)
    {
        g_modelVisibleMeshletCount = k_modelAmplificationInitialVisibleMeshletCount;
    }

    // 他のthreadが初期化前の値へアクセスしないように、
    // Group内の全Threadをここで同期する
    GroupMemoryBarrierWithGroupSync();

    // この描画のオブジェクト(行列)とメッシュ(バッファの番号・Meshletの数)を、テーブルから読む
    // 32個のThreadが同じ要素を読むが、GPUのキャッシュに乗るため、読み込みは実質1回分で済む
    const ModelObjectData l_object = FetchModelObjectData();
    const ModelMeshData   l_mesh   = FetchModelMeshData  ();

    // SV_DispatchThreadID.xは、全てのAmplificationShaderGroupを通した連続ThreadIndex
    // 1つのThreadが1つのMeshletを担当するため、
    // この値をそのままMeshletIndexとして使用できる
    const uint l_meshletIndex   = a_dispatchThreadID.x;
          bool l_shouldDispatch = false;

    // 最後のAmplificationShaderGroupには、
    // 実際のMeshlet数を超えた余分なThreadが含まれる可能性がある
    // 範囲外のThreadはMeshletBoundsBufferへアクセスさせない
    if (l_meshletIndex < l_mesh.meshletCount)
    {
        l_shouldDispatch = ShouldDispatchModelMeshlet(l_object, l_mesh, l_meshletIndex);
    }

    if (l_shouldDispatch)
    {
        uint l_payloadMeshletIndex = k_modelAmplificationInitialVisibleMeshletCount;

        // 可視Meshletを書き込む配列位置を一つ確保する
        // InterlockedAddの第三引数には加算前の値が入るため、
        // 可視Meshletは配列のZero番目から連続して格納される
        InterlockedAdd(g_modelVisibleMeshletCount, k_modelAmplificationVisibleMeshletCountIncrement, l_payloadMeshletIndex);

        g_modelAmplificationPayload.meshletIndexList[l_payloadMeshletIndex] = l_meshletIndex;
    }

    // DispatchMesh自体にGroupMemoryBarrierWithGroupSync相当の同期が含まれる
    // 可視判定を通過したMeshlet数だけ、子となるMeshShaderGroupを起動する
    // 可視Meshletが存在しなければ、GroupCountXは0となりMeshShaderは起動されない
    DispatchMesh(g_modelVisibleMeshletCount,
                 k_modelAmplificationDispatchMeshGroupCountY,
                 k_modelAmplificationDispatchMeshGroupCountZ,
                 g_modelAmplificationPayload);
}
```

### Shader/Model/Standard/ModelStandard.hlsli(変更・写経)

```hlsl
#ifndef MODEL_STANDARD_HLSLI
#define MODEL_STANDARD_HLSLI
#include "../Model.hlsli"
#include "../../Camera/CameraPass.hlsli"

SamplerState g_textureSampler : register(s0);

// ModelのLocal法線をWorld空間へ変換する
// 法線は位置ではなく方向なのでw = 0
// 非均一スケールでも法線方向が壊れにくいようにWorldInverseTransposeMatrixを使う
float3 TransformModelLocalNormalToWorld(const ModelObjectData a_object, const float3 a_localNormal)
{
    const float4 l_localNormal = float4(a_localNormal, k_modelDirectionElementW);
    const float4 l_worldNormal = mul   (l_localNormal, a_object.worldInverseTransposeMatrix);

    return normalize(l_worldNormal.xyz);
}

// ModelのLocalTangentをWorld空間へ変換する
// Tangentは方向なのでw = 0
// tangent.wはNormalMap用の向き補正なので維持する
float4 TransformModelLocalTangentToWorld(const ModelObjectData a_object, const float4 a_localTangent)
{
    const float4 l_localTangent = float4(a_localTangent.xyz, k_modelDirectionElementW);
    const float4 l_worldTangent = mul   (l_localTangent,     a_object.worldMatrix);

    return float4(normalize(l_worldTangent.xyz), a_localTangent.w * a_object.worldOrientationSign);
}

// ベースカラーのテクスチャを読み、色を掛ける
// マテリアルのテーブルを直接読まず、番号と色を引数で受け取る(規約 19-8)
// こうすると、Lit / UnLit / トゥーンなど、マテリアルの種類が違っても同じ関数を使える
float4 FetchModelBaseColor(const uint a_baseColorTextureSRVDescriptorIndex, const float4 a_baseColor, const float2 a_uv)
{
    Texture2D<float4> l_baseColorTexture = ResourceDescriptorHeap[a_baseColorTextureSRVDescriptorIndex];

    const float4 l_baseColorSample = l_baseColorTexture.Sample(g_textureSampler, a_uv);

    return l_baseColorSample * a_baseColor;
}

#endif // MODEL_STANDARD_HLSLI
```

### Shader/Model/Static/Standard/Lit/StaticModelStandardLit_MS.hlsl(変更・写経)

```hlsl
#include "../../StaticModel.hlsli"
#include "../../../Standard/ModelStandard.hlsli"
#include "../../../Standard/Lit/ModelStandardLit.hlsli"

[outputtopology("triangle")]
[numthreads(k_modelMeshShaderThreadCountX, k_modelMeshShaderThreadCountY, k_modelMeshShaderThreadCountZ)]
void main(in  payload  ModelAmplificationPayload a_payload,
          out vertices MSOutputLit               a_vertexList[k_modelMAXMeshletVertexCount],
          out indices  uint3                     a_primitiveList[k_modelMAXMeshletPrimitiveCount],
              const    uint3                     a_groupID : SV_GroupID,
              const    uint                      a_groupThreadIndex : SV_GroupIndex)
{
    // この描画のオブジェクト(行列)とメッシュ(バッファの番号)を、テーブルから読む
    const ModelObjectData l_object = FetchModelObjectData();
    const ModelMeshData   l_mesh   = FetchModelMeshData  ();

    StructuredBuffer<StaticModelVertex> l_staticModelVertexBuffer = ResourceDescriptorHeap[l_mesh.vertexBufferSRVDescriptorIndex];
    StructuredBuffer<ModelMeshlet>      l_modelMeshletBuffer      = ResourceDescriptorHeap[l_mesh.meshletBufferSRVDescriptorIndex];
    StructuredBuffer<uint>              l_uniqueVertexIndexBuffer = ResourceDescriptorHeap[l_mesh.uniqueVertexIndexBufferSRVDescriptorIndex];

    // ASからPayload経由で渡されたMeshletIndexを使う
    const uint         l_meshletIndex = a_payload.meshletIndexList[a_groupID.x];
    const ModelMeshlet l_modelMeshlet = l_modelMeshletBuffer      [l_meshletIndex];

    // 出力頂点数、三角形数を設定
    SetMeshOutputCounts(l_modelMeshlet.vertexCount, l_modelMeshlet.triangleCount);

    for (uint l_vertexIndex = a_groupThreadIndex; l_vertexIndex < l_modelMeshlet.vertexCount; l_vertexIndex += k_modelMeshShaderThreadCountX)
    {
        // このメッシュレットの使用頂点開始位置 + 何番目か で、UniqueVertexIndexの位置を求める
        const uint l_uniqueVertexIndex = l_modelMeshlet.vertexOffset + l_vertexIndex;

        // UniqueVertexIndexBufferからVertexBufferにアクセスするためのIndexを取得
        // 例 : l_uniqueVertexIndexBuffer[0] = 40;でIndexが40のVertexBufferにアクセス
        const uint l_modelVertexIndex = l_uniqueVertexIndexBuffer[l_uniqueVertexIndex];

        // 取得した頂点番号から頂点情報を取得
        const StaticModelVertex l_staticModelVertex = l_staticModelVertexBuffer[l_modelVertexIndex];

        // ワールド座標、法線、接線、クリップ座標を計算する
        const float3 l_worldPosition          = TransformModelLocalPositionToWorld(l_object, l_staticModelVertex.position);
        const float3 l_worldNormal            = TransformModelLocalNormalToWorld  (l_object, l_staticModelVertex.normal);
        const float4 l_worldTangent           = TransformModelLocalTangentToWorld (l_object, l_staticModelVertex.tangent);
        const float4 l_viewProjectionPosition = mul                               (float4(l_worldPosition, k_modelPositionElementW), g_viewProjectionMatrix);

        a_vertexList[l_vertexIndex].position      = l_viewProjectionPosition;
        a_vertexList[l_vertexIndex].worldPosition = l_worldPosition;
        a_vertexList[l_vertexIndex].worldNormal   = l_worldNormal;
        a_vertexList[l_vertexIndex].worldTangent  = l_worldTangent;
        a_vertexList[l_vertexIndex].uv            = l_staticModelVertex.uv;
    }

    for (uint l_triangleIndex = a_groupThreadIndex; l_triangleIndex < l_modelMeshlet.triangleCount; l_triangleIndex += k_modelMeshShaderThreadCountX)
    {
        // 3個Pack方式では、uint32_t1個が三角形1個分のPrimitiveIndexを持つ
        // そのため、triangleOffsetはPack済みのPrimitiveIndexBuffer上の開始Indexとして扱う
        const uint l_packedPrimitiveIndex = l_modelMeshlet.triangleOffset + l_triangleIndex;

        a_primitiveList[l_triangleIndex] = FetchModelPackedPrimitiveIndex(l_object, l_mesh, l_packedPrimitiveIndex);
    }
}
```

**StaticModelStandardUnLit_MS.hlsl / SkeletalAnimationModelStandardLit_MS.hlsl / SkeletalAnimationModelStandardUnLit_MS.hlsl** も同じ書き換え。

- 先頭で `l_object` と `l_mesh` を読む。
- バッファの番号は `g_xxxSRVDescriptorIndex` ではなく `l_mesh.xxxSRVDescriptorIndex`。
- `TransformModelLocal...` と `FetchModelPackedPrimitiveIndex` に `l_object`(と `l_mesh`)を渡す。
- Skeletal の頂点の型は `SkeletalAnimationSkinnedVertex` のまま(メッシュのテーブルがスキニング後の頂点のバッファを指している)。

### Shader/Model/Shadow/Cascade/ModelCascadeShadowMeshletCulling.hlsli(変更・写経)

```hlsl
bool IsVisibleModelMeshletByCascadeFrustum(const ModelObjectData a_object, const ModelMeshletBounds a_meshletBounds)
{
    const float3 l_worldCenter     = TransformModelLocalPositionToWorld(a_object, a_meshletBounds.center);
    const float  l_worldRadius     = a_meshletBounds.radius * a_object.worldMAXScale;
    const float4 l_lightViewCenter = mul(float4(l_worldCenter, k_modelPositionElementW), g_cascadeViewMatrix);

    // (ここから下は今までと同じ)
    ...
}

bool IsBackfaceModelMeshletByDirectionalLightCone(const ModelObjectData a_object, const ModelMeshletBounds a_meshletBounds)
{
    if (a_meshletBounds.coneCutoff >= k_modelDisabledMeshletConeCutoff) { return false; }

    const float4 l_localConeAxis = float4(a_meshletBounds.coneAxis, k_modelDirectionElementW);
    const float3 l_worldConeAxis = normalize(mul(l_localConeAxis, a_object.worldInverseTransposeMatrix).xyz);

    return dot(g_directionalLightDirection, l_worldConeAxis) >= a_meshletBounds.coneCutoff;
}

bool ShouldDispatchModelCascadeShadowMeshlet(const ModelObjectData a_object, const ModelMeshData a_mesh, const uint a_meshletIndex)
{
    StructuredBuffer<ModelMeshletBounds> l_meshletBoundsBuffer = ResourceDescriptorHeap[a_mesh.meshletBoundsBufferSRVDescriptorIndex];

    const ModelMeshletBounds l_meshletBounds = l_meshletBoundsBuffer[a_meshletIndex];

    if (!IsVisibleModelMeshletByCascadeFrustum(a_object, l_meshletBounds) ||
        IsBackfaceModelMeshletByDirectionalLightCone(a_object, l_meshletBounds))
    {
        return false;
    }

    return true;
}
```

**ModelCascadeShadow_AS.hlsl** は `Model_AS.hlsl` と同じく、先頭で `l_object` / `l_mesh` を読み、`l_mesh.meshletCount` と `ShouldDispatchModelCascadeShadowMeshlet(l_object, l_mesh, l_meshletIndex)` を使う。
**StaticModelCascadeShadow_MS.hlsl / SkeletalAnimationModelCascadeShadow_MS.hlsl** は Lit の MS と同じ書き換え(位置だけを計算する)。

---

## CONFIG/Graphics/GraphicsCONFIG.json(変更)

### `Renderer` に `ModelRenderSystem` を足す

```json
        "ModelRenderSystem": {
            "TableMap": [
                { "Capacity": 4096,  "TypeName": "ModelObjectGPUData" },
                { "Capacity": 16384, "TypeName": "ModelMeshGPUData" },
                { "Capacity": 1024,  "TypeName": "ModelStandardLitMaterialGPUData" },
                { "Capacity": 1024,  "TypeName": "ModelStandardUnLitMaterialGPUData" }
            ]
        },
```

> **ここに書いた種類だけテーブルが作られる。** 新しいマテリアルの種類を足したら、ここにも1行足す(パスを RenderGraphPassList に足すのと同じ)。
> `Capacity` を省くと 1024(`ModelRenderSystemJsonConverter::k_defaultCapacity`)。
> 容量の目安 : オブジェクト 4096 体。メッシュは Skeletal が「メッシュ × 3 フレーム」使うので多めに 16384。
> メモリ : 4096 × 136 = 約 557KB、16384 × 24 = 約 393KB(それぞれ UPLOAD × 3 も持つ)。

### `RenderGraphPassList` の先頭にパスを足す(追加済み)

```json
            "RenderGraphPassList": [
                {
                    "RenderGraphPassTypeName": "ModelRenderTableUploadPass"
                },
                {
                    "RenderGraphPassTypeName": "SkeletalAnimationComputePass"
                },
```

> 一覧の中の位置は、どこでもよい(実行レイヤー `Upload` が一番前なので、`Compile` で先頭へ並べ替えられる)。読みやすいように先頭に置いた。

### `RootSignatureType::ModelStandard`

```json
"RootParameterIndexMap": [
    { "Index": 0, "RootParameterType": "RootParameterType::CBCameraPass" },
    { "Index": 1, "RootParameterType": "RootParameterType::RCModelDrawItem" },
    { "Index": 2, "RootParameterType": "RootParameterType::CBLightPass" },
    { "Index": 3, "RootParameterType": "RootParameterType::CBCascadeShadowMapPass" },
    { "Index": 4, "RootParameterType": "RootParameterType::CBCullingCameraPass" },
    { "Index": 5, "RootParameterType": "RootParameterType::RCModelTable" },
    { "Index": 6, "RootParameterType": "RootParameterType::RCModelMaterialTable" }
],
"RootParameterList": [
    { "ParameterType": "D3D12_ROOT_PARAMETER_TYPE_CBV",             "RegisterSpace": 0, "ShaderRegister": 0, "ShaderVisibility": "D3D12_SHADER_VISIBILITY_ALL" },
    { "ParameterType": "D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS", "RegisterSpace": 0, "ShaderRegister": 1, "Num32BitValues": 3, "ShaderVisibility": "D3D12_SHADER_VISIBILITY_ALL" },
    { "ParameterType": "D3D12_ROOT_PARAMETER_TYPE_CBV",             "RegisterSpace": 0, "ShaderRegister": 2, "ShaderVisibility": "D3D12_SHADER_VISIBILITY_PIXEL" },
    { "ParameterType": "D3D12_ROOT_PARAMETER_TYPE_CBV",             "RegisterSpace": 0, "ShaderRegister": 3, "ShaderVisibility": "D3D12_SHADER_VISIBILITY_PIXEL" },
    { "ParameterType": "D3D12_ROOT_PARAMETER_TYPE_CBV",             "RegisterSpace": 0, "ShaderRegister": 4, "ShaderVisibility": "D3D12_SHADER_VISIBILITY_AMPLIFICATION" },
    { "ParameterType": "D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS", "RegisterSpace": 0, "ShaderRegister": 5, "Num32BitValues": 2, "ShaderVisibility": "D3D12_SHADER_VISIBILITY_ALL" },
    { "ParameterType": "D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS", "RegisterSpace": 0, "ShaderRegister": 6, "Num32BitValues": 1, "ShaderVisibility": "D3D12_SHADER_VISIBILITY_PIXEL" }
]
```

> ルート定数の JSON のキー名 `Num32BitValues` は、`RootSignatureJsonConverter` の `k_num32BitValuesJsonKey` と同じ文字列(確認済み)。
> 大きさ : CBV 4 つ × 2 + ルート定数 3 + 2 + 1 = **14 DWORD**(上限 64)。

### `RootSignatureType::ModelCascadeShadow`

```json
"RootParameterIndexMap": [
    { "Index": 0, "RootParameterType": "RootParameterType::CBModelCascadeShadowPass" },
    { "Index": 1, "RootParameterType": "RootParameterType::RCModelDrawItem" },
    { "Index": 2, "RootParameterType": "RootParameterType::RCModelTable" }
],
"RootParameterList": [
    { "ParameterType": "D3D12_ROOT_PARAMETER_TYPE_CBV",             "RegisterSpace": 0, "ShaderRegister": 0, "ShaderVisibility": "D3D12_SHADER_VISIBILITY_ALL" },
    { "ParameterType": "D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS", "RegisterSpace": 0, "ShaderRegister": 1, "Num32BitValues": 3, "ShaderVisibility": "D3D12_SHADER_VISIBILITY_ALL" },
    { "ParameterType": "D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS", "RegisterSpace": 0, "ShaderRegister": 5, "Num32BitValues": 2, "ShaderVisibility": "D3D12_SHADER_VISIBILITY_ALL" }
]
```

> 影のシェーダーも `Model.hlsli` の `RCModelTable : register(b5)` を使うので、レジスタは同じ b5 にする(ルートシグネチャの中の順番は 2 番目でよい)。

### PSO の AS のパス

`StaticModelLit` / `StaticModelUnLit` / `SkeletalAnimationModelLit` / `SkeletalAnimationModelUnLit` の4つの `AmplificationShader.FilePath` を
`"Shader/Model/Model_AS.cso"` にする。

---

## 動作の確認

S3 ~ S5 の間はビルドしない。S5 の終わりで、モデルが今までどおり描かれることを確かめる。

## 次のステップへのつながり

- S4 : マテリアルの値を `ModelStandardLitMaterialGPUData` / `ModelStandardUnLitMaterialGPUData` のテーブルへ書き、PS がそれを読む形にする。
  マテリアルのクラスは、自分の GPU データの型のテーブルの情報(`GetREFModelRenderTableINFO()`)を返すだけで、テーブルが決まる。
- S5 : パスが「描画項目(`ModelDrawItem`)の一覧」を回して、ルート定数 + `DispatchMesh` だけを積む形にする。
  描画項目の一覧は「メッシュの種類(Static / Skeletal) × マテリアルのテーブル」ごとに、CONFIG に書いたマテリアルの数だけ自動で作る。
