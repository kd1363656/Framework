# S3 ModelRenderSystem(オブジェクトのテーブル・メッシュのテーブル)とルート定数

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

## ファイル一覧

### 新規

| ファイル | 内容 |
|---|---|
| `Source/Framework/Definition/Enum/Graphics/ModelRenderSystemEnum.h` | `Enum::ModelRenderTableType`(Object / Mesh / StandardLitMaterial / StandardUnLitMaterial) |
| `Source/Framework/Definition/Constant/Graphics/ModelRenderSystemConstant.h` | 向きの符号などの定数(旧 `ModelPerObjectConstantBufferUploaderConstant.h` から移す) |
| `Source/Framework/Definition/Struct/Graphics/Buffer/Root/RCModelStruct.h` | `Struct::RCModelDrawItem` / `RCModelTable` / `RCModelMaterialTable` |
| `Source/Framework/Definition/Struct/Graphics/ModelRenderSystemStruct.h` | `Struct::ModelObjectGPUData` / `ModelMeshGPUData` / マテリアルの GPU データ / `ModelDrawItem` / `ModelRenderTableSetting` |
| `Source/Framework/Graphics/Render/Model/ModelRenderSystem.h/.cpp` | テーブルをまとめて持つクラス |
| `Source/Framework/Graphics/Render/Model/Converter/Json/ModelRenderSystemJsonConverter.h/.cpp` | 容量の設定の JSON |
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
| `Graphics/Render/Renderer.h/.cpp` | `ModelRenderSystem` を持つ。PostDeserialize で作り、BeginFrame でコピー |
| `Graphics/Render/Converter/Json/RendererJsonConverter.h/.cpp` | `"ModelRenderSystem"` キー |
| `Graphics/Resource/Model/Skeletal/Player/SkeletalAnimationPlayer.h` | `GetREFFrameDataList` |
| `GameObject/Component/Model/Renderer/...StaticRenderer.h/.cpp` / `...SkeletalRenderer.h/.cpp` | テーブルの要素を持つ(番号の割り当て・行列が変わったときだけ書く・解放) |
| `Shader/Model/Model.hlsli` / `ModelMeshletCulling.hlsli` / `Standard/ModelStandard.hlsli` | CB をやめてテーブルを読む |
| `Shader/Model/Static|Skeletal/Standard/Lit|UnLit/*_MS.hlsl`(4つ) | 同上 |
| `Shader/Model/Shadow/Cascade/ModelCascadeShadowMeshletCulling.hlsli` / `ModelCascadeShadow_AS.hlsl` / Static・Skeletal の影の MS | 同上 |
| `CONFIG/Graphics/GraphicsCONFIG.json` | ルートシグネチャ2つ・PSO の AS のパス・`ModelRenderSystem` の容量 |
| `Framework.vcxproj` / `.filters` | DXCTask の AS を差し替え |

### 登録

- フィルター: `Source\Framework\Definition\Struct\Graphics\Buffer\Root` / `Source\Framework\Graphics\Render\Model` / `...\Model\Converter` / `...\Model\Converter\Json`
- Framework.h:
  - Constant : `ModelRenderSystemConstant.h`(`ModelPerObjectConstantBufferUploaderConstant.h` の行を置き換える)
  - Enum : `ModelRenderSystemEnum.h`
  - Struct : `RCModelStruct.h` → `ModelRenderSystemStruct.h`
  - `ModelRenderSystemJsonConverter.h` → `ModelRenderSystem.h` は `GPUElementTable.h` より後、`Renderer.h` より前
- DXCTask: `Shader\Model\Model_AS.hlsl`(ShaderType = Amplification)を足し、Static / Skeletal の AS を消す。

---

## コード(C++)

### Definition/Enum/Graphics/ModelRenderSystemEnum.h(新規)

```cpp
#pragma once

namespace FWK::Enum
{
    enum class ModelRenderTableType
    {
        Invalid,
        Object,
        Mesh,
        StandardLitMaterial,
        StandardUnLitMaterial,
        Count,
    };

    FWK_JSON_SERIALIZE_ENUM
    (
        ModelRenderTableType,
        FWK_JSON_ENUM_VALUE(ModelRenderTableType::Invalid),
        FWK_JSON_ENUM_VALUE(ModelRenderTableType::Object),
        FWK_JSON_ENUM_VALUE(ModelRenderTableType::Mesh),
        FWK_JSON_ENUM_VALUE(ModelRenderTableType::StandardLitMaterial),
        FWK_JSON_ENUM_VALUE(ModelRenderTableType::StandardUnLitMaterial),
        FWK_JSON_ENUM_VALUE(ModelRenderTableType::Count),
    )
}
```

> マテリアルの種類が増えたら(トゥーンなど)、ここに1行足し、`ModelRenderSystem::FetchVALElementByteStride` に大きさを足すだけでテーブルが増える。

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

### Definition/Struct/Graphics/ModelRenderSystemStruct.h(新規)

```cpp
#pragma once

namespace FWK::Struct
{
    struct ModelObjectGPUData final
    {
        static constexpr float k_initialWorldMAXScale = 1.0F;

        TypeAlias::Math::Matrix m_worldMatrix                 = TypeAlias::Math::Matrix::Identity;
        TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;

        float m_worldMAXScale        = k_initialWorldMAXScale;
        float m_worldOrientationSign = Constant::k_normalModelWorldOrientationSign;
    };

    struct ModelMeshGPUData final
    {
        static constexpr std::uint32_t k_initialMeshletCount = 0U;

        TypeAlias::DescriptorIndex m_vertexBufferSRVDescriptorIndex            = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBufferSRVDescriptorIndex           = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_uniqueVertexIndexBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_primitiveIndexBufferSRVDescriptorIndex    = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBoundsBufferSRVDescriptorIndex     = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        std::uint32_t m_meshletCount = k_initialMeshletCount;
    };

    struct ModelStandardLitMaterialGPUData final
    {
        static constexpr float k_defaultMetallic  = 0.0F;
        static constexpr float k_defaultRoughness = 1.0F;

        TypeAlias::Math::Color m_baseColor = Constant::k_whiteColor;

        float m_metallic  = k_defaultMetallic;
        float m_roughness = k_defaultRoughness;

        TypeAlias::DescriptorIndex m_baseColorTextureSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_normalTextureSRVDescriptorIndex    = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_metallicTextureSRVDescriptorIndex  = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_roughnessTextureSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
    };

    struct ModelStandardUnLitMaterialGPUData final
    {
        TypeAlias::Math::Color m_baseColor = Constant::k_whiteColor;

        TypeAlias::DescriptorIndex m_baseColorTextureSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
    };

    struct ModelDrawItem final
    {
        Struct::RCModelDrawItem m_rootConstant = {};

        D3D12_DISPATCH_MESH_ARGUMENTS m_dispatchMeshArguments = {};
    };

    struct ModelRenderTableSetting final
    {
        static constexpr UINT k_defaultCapacity = 1024U;

        Enum::ModelRenderTableType m_type = Enum::ModelRenderTableType::Invalid;

        UINT m_capacity = k_defaultCapacity;
    };
}
```

> 大きさ: `ModelObjectGPUData` = 64 + 64 + 4 + 4 = **136 バイト**、`ModelMeshGPUData` = 4 × 6 = **24 バイト**、
> `ModelStandardLitMaterialGPUData` = 16 + 4 + 4 + 4 × 4 = **40 バイト**、`ModelStandardUnLitMaterialGPUData` = 16 + 4 = **20 バイト**。
> StructuredBuffer は cbuffer と違って 16 バイト境界に揃えないので、パディングは入れない(規約 19-10)。
> `ModelDrawItem` は「描画1回ぶん」= ルート定数(12 バイト)+ `DispatchMesh` の引数(`D3D12_DISPATCH_MESH_ARGUMENTS` = X / Y / Z の 12 バイト)= **24 バイト**。
> S5 ではこれを CPU が読んで `SetGraphicsRoot32BitConstants` と `DispatchMesh` を呼ぶ。
> **S6 の ExecuteIndirect の引数のバッファの1件と同じ並びにしてある**ので、S6 では一覧をそのまま GPU のバッファへ memcpy するだけでよい(変換が要らない)。
> GPU のメモリ配置に合わせる構造体なので、メンバの並びは規約 20-6 の例外。

### Graphics/Render/Model/ModelRenderSystem.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelRenderSystem final
    {
    private:

        using ModelRenderTableMap = std::unordered_map<Enum::ModelRenderTableType, std::shared_ptr<GPUElementTable>>;

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

        void SetTableSettingList(std::vector<Struct::ModelRenderTableSetting>&& a_set) { m_tableSettingList = std::move(a_set); }

        Struct::RCModelTable FetchVALRCModelTable() const;

        TypeAlias::DescriptorIndex FetchVALTableSRVDescriptorIndex(const Enum::ModelRenderTableType a_type) const;

        std::weak_ptr<GPUElementTable> FindVALTable(const Enum::ModelRenderTableType a_type) const;

        const auto& GetREFTableSettingList() const { return m_tableSettingList; }

    private:

        UINT FetchVALElementByteStride(const Enum::ModelRenderTableType a_type) const;

        UINT FetchVALCapacity(const Enum::ModelRenderTableType a_type) const;

        static constexpr UINT k_invalidElementByteStride = 0U;

        std::vector<Struct::ModelRenderTableSetting> m_tableSettingList = {};

        ModelRenderTableMap m_tableMap = {};

        Converter::ModelRenderSystemJsonConverter m_jsonConverter = {};
    };
}
```

> - テーブルは `shared_ptr` で持ち、使う側(ModelComponent の描き方・マテリアル)には `FindVALTable` で `weak_ptr` を渡す。
>   使う側は `lock()` してから番号の割り当て・書き込み・返却をする。
> - アプリの終了時は Renderer(このクラス)がマテリアル(ResourceContext)より先に破棄される。使う側が `weak_ptr` なら、
>   テーブルが先に消えていても `lock()` に失敗するだけで、壊れたメモリには触れない(撤回した設計で起きた終了時のアサートの対策)。

### Graphics/Render/Model/ModelRenderSystem.cpp(新規・写経)

```cpp
#include "ModelRenderSystem.h"

// モデルの描画に使うテーブル(GPUElementTable)を、種類ごとにまとめて持つクラス
// 種類 : オブジェクト(行列) / メッシュ(バッファのSRVの番号) / マテリアル(種類ごと)
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
    // Invalidの次(Object)からCountの手前(最後のマテリアル)まで、すべての種類のテーブルを作る
    // 例 : Object(1) / Mesh(2) / StandardLitMaterial(3) / StandardUnLitMaterial(4)
    for (auto l_typeValue = static_cast<std::size_t>(Enum::ModelRenderTableType::Object); l_typeValue < static_cast<std::size_t>(Enum::ModelRenderTableType::Count); ++l_typeValue)
    {
        const auto l_type              = static_cast<Enum::ModelRenderTableType>(l_typeValue);
        const auto l_elementByteStride = FetchVALElementByteStride(l_type);
        const auto l_capacity          = FetchVALCapacity         (l_type);
              auto l_table             = std::make_shared<GPUElementTable>();

        FWK_ASSERT_RETURN_VALUE_IF(l_elementByteStride == k_invalidElementByteStride, "1要素の大きさが決まっていない種類があるため、ModelRenderSystemの作成に失敗しました。", false);

        FWK_ASSERT_RETURN_VALUE_IF(!l_table->Create(a_device,
                                                    a_gpuMemoryAllocator,
                                                    a_frameCount,
                                                    l_capacity,
                                                    l_elementByteStride,
                                                    a_cbvSRVUAVDescriptorPool),
                                                    "ModelRenderSystemのテーブルの作成に失敗しました。",
                                                    false);

        m_tableMap.try_emplace(l_type, std::move(l_table));
    }

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

FWK::Struct::RCModelTable FWK::Graphics::ModelRenderSystem::FetchVALRCModelTable() const
{
    // パスの最初に1回だけ送る、オブジェクトとメッシュのテーブルのSRVの番号
    // シェーダーはこの番号でテーブル(StructuredBuffer)を取り出し、描画ごとの番号で要素を読む
    Struct::RCModelTable l_rcModelTable = {};

    l_rcModelTable.m_objectTableSRVDescriptorIndex = FetchVALTableSRVDescriptorIndex(Enum::ModelRenderTableType::Object);
    l_rcModelTable.m_meshTableSRVDescriptorIndex   = FetchVALTableSRVDescriptorIndex(Enum::ModelRenderTableType::Mesh);

    return l_rcModelTable;
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::ModelRenderSystem::FetchVALTableSRVDescriptorIndex(const Enum::ModelRenderTableType a_type) const
{
    const auto& l_tableITR = m_tableMap.find(a_type);

    FWK_ASSERT_RETURN_VALUE_IF(l_tableITR == m_tableMap.end(), "対応するテーブルが無いため、テーブルのSRVの番号の取得に失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);

    const auto& l_table = l_tableITR->second;

    FWK_ASSERT_RETURN_VALUE_IF(!l_table, "テーブルが無効のため、テーブルのSRVの番号の取得に失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);

    return l_table->GetVALSRVDescriptorIndex();
}

std::weak_ptr<FWK::Graphics::GPUElementTable> FWK::Graphics::ModelRenderSystem::FindVALTable(const Enum::ModelRenderTableType a_type) const
{
    const auto& l_tableITR = m_tableMap.find(a_type);

    if (l_tableITR == m_tableMap.end()) { return {}; }

    return l_tableITR->second;
}

UINT FWK::Graphics::ModelRenderSystem::FetchVALElementByteStride(const Enum::ModelRenderTableType a_type) const
{
    // 種類ごとの1要素の大きさ(GPUへ送る構造体の大きさ)
    // HLSLのStructuredBufferの要素の構造体と、同じ大きさ・同じ並びにしてある
    switch (a_type)
    {
        case Enum::ModelRenderTableType::Object:
        {
            return static_cast<UINT>(sizeof(Struct::ModelObjectGPUData));
        }
        break;

        case Enum::ModelRenderTableType::Mesh:
        {
            return static_cast<UINT>(sizeof(Struct::ModelMeshGPUData));
        }
        break;

        case Enum::ModelRenderTableType::StandardLitMaterial:
        {
            return static_cast<UINT>(sizeof(Struct::ModelStandardLitMaterialGPUData));
        }
        break;

        case Enum::ModelRenderTableType::StandardUnLitMaterial:
        {
            return static_cast<UINT>(sizeof(Struct::ModelStandardUnLitMaterialGPUData));
        }
        break;

        default:
        break;
    }

    return k_invalidElementByteStride;
}

UINT FWK::Graphics::ModelRenderSystem::FetchVALCapacity(const Enum::ModelRenderTableType a_type) const
{
    // GraphicsCONFIG.jsonで指定された容量を探す
    // 指定が無い種類は、既定の容量(1024)にする
    const auto& l_tableSettingITR = std::ranges::find_if(m_tableSettingList,
                                                         [a_type](const auto& a_tableSetting)
                                                         {
                                                             return a_tableSetting.m_type == a_type;
                                                         });

    if (l_tableSettingITR == m_tableSettingList.end()) { return Struct::ModelRenderTableSetting::k_defaultCapacity; }

    return l_tableSettingITR->m_capacity;
}
```

> `switch` の各 `case` は `return` で抜けるが、規約 9-10 の形(ブロックの後ろに `break;`)に揃えている。

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

        static constexpr std::string_view k_tableSettingListJsonKey = "TableSettingList";
        static constexpr std::string_view k_typeJsonKey             = "Type";
        static constexpr std::string_view k_capacityJsonKey         = "Capacity";
    };
}
```

### Graphics/Render/Model/Converter/Json/ModelRenderSystemJsonConverter.cpp(新規・写経)

```cpp
#include "ModelRenderSystemJsonConverter.h"

void FWK::Converter::ModelRenderSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::ModelRenderSystem& a_modelRenderSystem) const
{
    if (a_rootJson.is_null()) { return; }

    if (!Utility::IsJsonArray(a_rootJson, k_tableSettingListJsonKey)) { return; }

    // テーブルの種類と容量の組を読み込む
    // 例 : { "Type" : "ModelRenderTableType::Object", "Capacity" : 4096 }
    std::vector<Struct::ModelRenderTableSetting> l_tableSettingList = {};

    for (const auto& l_json : a_rootJson[k_tableSettingListJsonKey])
    {
        Struct::ModelRenderTableSetting l_tableSetting = {};

        l_tableSetting.m_type     = l_json.value(k_typeJsonKey,     Enum::ModelRenderTableType::Invalid);
        l_tableSetting.m_capacity = l_json.value(k_capacityJsonKey, Struct::ModelRenderTableSetting::k_defaultCapacity);

        l_tableSettingList.emplace_back(l_tableSetting);
    }

    a_modelRenderSystem.SetTableSettingList(std::move(l_tableSettingList));
}

nlohmann::json FWK::Converter::ModelRenderSystemJsonConverter::Serialize(const Graphics::ModelRenderSystem& a_modelRenderSystem) const
{
          nlohmann::json l_rootJson           = {};
          nlohmann::json l_tableSettingJson   = nlohmann::json::array();
    const auto&          l_tableSettingList   = a_modelRenderSystem.GetREFTableSettingList();

    for (const auto& l_tableSetting : l_tableSettingList)
    {
        nlohmann::json l_json = {};

        l_json[k_typeJsonKey]     = l_tableSetting.m_type;
        l_json[k_capacityJsonKey] = l_tableSetting.m_capacity;

        l_tableSettingJson.emplace_back(std::move(l_json));
    }

    l_rootJson[k_tableSettingListJsonKey] = std::move(l_tableSettingJson);

    return l_rootJson;
}
```

> `Utility::IsJsonArray(json, key)` は `Utility/Json/JsonUtility.h` にある(キーを渡すと、その子が配列かを調べる)。

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

**BeginFrame(`m_frameGPUTimestampScopeIndex = ...` の直後):**

```cpp
    // このフレームまでに書き換えたテーブルの要素だけを、GPUへコピーする命令を積む
    // すべてのパスより前に積むので、このフレームの描画は新しい値を読む
    m_modelRenderSystem.RecordUpload(m_directCommandList, m_currentFrameResourceIndex);
```

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

    const auto& l_graphicsManager   = Graphics::GraphicsManager::GetInstance  ();
    const auto& l_renderer          = l_graphicsManager.GetREFRenderer        ();
    const auto& l_modelRenderSystem = l_renderer.GetREFModelRenderSystem       ();
    const auto& l_modelData         = l_staticModelRecord->GetREFModelData    ();

    // オブジェクトとメッシュのテーブルを受け取って覚えておく
    // weak_ptrなので、アプリの終了時にテーブルが先に消えても、このクラスが壊れたメモリに触ることはない
    m_objectTable = l_modelRenderSystem.FindVALTable(Enum::ModelRenderTableType::Object);
    m_meshTable   = l_modelRenderSystem.FindVALTable(Enum::ModelRenderTableType::Mesh);

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

        Struct::ModelMeshGPUData l_meshGPUData = {};

        l_meshGPUData.m_vertexBufferSRVDescriptorIndex            = l_meshRuntimeData.m_vertexBuffer.GetVALSRVDescriptorIndex           ();
        l_meshGPUData.m_meshletBufferSRVDescriptorIndex           = l_meshRuntimeData.m_meshletBuffer.GetVALSRVDescriptorIndex          ();
        l_meshGPUData.m_uniqueVertexIndexBufferSRVDescriptorIndex = l_meshRuntimeData.m_uniqueVertexIndexBuffer.GetVALSRVDescriptorIndex();
        l_meshGPUData.m_primitiveIndexBufferSRVDescriptorIndex    = l_meshRuntimeData.m_primitiveIndexBuffer.GetVALSRVDescriptorIndex   ();
        l_meshGPUData.m_meshletBoundsBufferSRVDescriptorIndex     = l_meshRuntimeData.m_meshletBoundsBuffer.GetVALSRVDescriptorIndex    ();
        l_meshGPUData.m_meshletCount                              = static_cast<std::uint32_t>(l_modelMesh.m_meshletData.m_meshletList.size());

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

    Struct::ModelObjectGPUData l_objectGPUData = {};

    // ワールド行列 : モデルの頂点(ローカル座標)をシーンの座標(ワールド座標)へ移す行列
    // 逆転置行列   : 法線を移すための行列。拡大縮小が均一でなくても、法線が面に垂直なまま保たれる
    l_objectGPUData.m_worldMatrix                 = a_worldMatrix;
    l_objectGPUData.m_worldInverseTransposeMatrix = a_worldMatrix.Invert().Transpose();

    // Meshletの境界球(カリング用)はローカル空間の半径なので、ワールドでの大きさにするため最大の拡大率を掛ける
    // 例 : X方向だけ2倍に伸ばしたら、半径も2倍として扱う(はみ出して消えるのを防ぐ)
    l_objectGPUData.m_worldMAXScale = Utility::CalculateWorldMAXScale(a_worldMatrix);

    // 行列式が負 = 鏡に映したように裏返っている(拡大率に負の値がある)
    // そのときは三角形の頂点の並び(表裏)が逆になるため、シェーダーで並びを入れ替えて元に戻す
    l_objectGPUData.m_worldOrientationSign = a_worldMatrix.Determinant() < Constant::k_modelWorldOrientationDeterminantBoundary ? Constant::k_mirrorModelWorldOrientationSign : Constant::k_normalModelWorldOrientationSign;

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
    const auto& l_graphicsManager   = Graphics::GraphicsManager::GetInstance            ();
    const auto& l_renderer          = l_graphicsManager.GetREFRenderer                  ();
    const auto& l_modelRenderSystem = l_renderer.GetREFModelRenderSystem                 ();
    const auto& l_frameDataList     = m_skeletalAnimationPlayer->GetREFFrameDataList    ();

    m_objectTable = l_modelRenderSystem.FindVALTable(Enum::ModelRenderTableType::Object);
    m_meshTable   = l_modelRenderSystem.FindVALTable(Enum::ModelRenderTableType::Mesh);

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

            Struct::ModelMeshGPUData l_meshGPUData = {};

            // 頂点は、スキニング後の頂点のバッファ(このフレーム用)を読む
            // Meshletの境界も、スキニング後の形に合わせて毎フレーム計算し直したもの(このフレーム用)を読む
            // Meshletの並びと三角形は、元のモデルと同じなので、モデルのバッファを読む
            l_meshGPUData.m_vertexBufferSRVDescriptorIndex            = l_frameData.m_skinnedVertexBufferList[l_meshListIndex].GetVALSRVDescriptorIndex();
            l_meshGPUData.m_meshletBufferSRVDescriptorIndex           = l_meshRuntimeData.m_meshletBuffer.GetVALSRVDescriptorIndex                     ();
            l_meshGPUData.m_uniqueVertexIndexBufferSRVDescriptorIndex = l_meshRuntimeData.m_uniqueVertexIndexBuffer.GetVALSRVDescriptorIndex           ();
            l_meshGPUData.m_primitiveIndexBufferSRVDescriptorIndex    = l_meshRuntimeData.m_primitiveIndexBuffer.GetVALSRVDescriptorIndex              ();
            l_meshGPUData.m_meshletBoundsBufferSRVDescriptorIndex     = l_frameData.m_meshletBoundsBufferList[l_meshListIndex].GetVALSRVDescriptorIndex();
            l_meshGPUData.m_meshletCount                              = static_cast<std::uint32_t>(l_modelMesh.m_meshletData.m_meshletList.size());

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
// C++側のStruct::ModelObjectGPUDataと同じ並び(136バイト)にする
// StructuredBufferの要素なので、cbufferのような16バイト境界のパディングは入れない
struct ModelObjectData
{
    row_major float4x4 worldMatrix;
    row_major float4x4 worldInverseTransposeMatrix;
    float              worldMAXScale;
    float              worldOrientationSign;
};

// メッシュのテーブルの1要素
// C++側のStruct::ModelMeshGPUDataと同じ並び(24バイト)にする
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
            "TableSettingList": [
                { "Capacity": 4096,  "Type": "ModelRenderTableType::Object" },
                { "Capacity": 16384, "Type": "ModelRenderTableType::Mesh" },
                { "Capacity": 1024,  "Type": "ModelRenderTableType::StandardLitMaterial" },
                { "Capacity": 1024,  "Type": "ModelRenderTableType::StandardUnLitMaterial" }
            ]
        },
```

> 容量の目安 : オブジェクト 4096 体。メッシュは Skeletal が「メッシュ × 3 フレーム」使うので多めに 16384。
> メモリ : 4096 × 136 = 約 557KB、16384 × 24 = 約 393KB(それぞれ UPLOAD × 3 も持つ)。

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

- S4 : マテリアルの値を `StandardLitMaterial` / `StandardUnLitMaterial` のテーブルへ書き、PS がそれを読む形にする。
- S5 : パスが「描画項目(`ModelDrawItem`)の一覧」を回して、ルート定数 + `DispatchMesh` だけを積む形にする。
