# S4 マテリアル(.mat・ハンドル・GPU テーブル・スロット)

## 骨組みの状況(2026-10-11 作成済み。次は写経)

こちらが書いたもの(ビルドは S6 の後)。**空の関数(仮の return)の中身が写経の対象**。

| 区分 | ファイル | 状態 |
|---|---|---|
| S4-1 | `Definition/Constant/Graphics/ModelMaterialConstant.h` / `Concept/IsDerivedBase/Graphics/IsDerivedModelMaterialBaseConcept.h` / `Type/Alias/Factory/Shared/ModelMaterialSharedFactoryTypeAlias.h` | 完成 |
| S4-1 | `AssetFilePathEnum.h`(`ModelMaterial`)/ Watcher の削除・名前変更に `ModelMaterial` の case | 完成(case の中身も書いた) |
| S4-1 | `BinaryConverterBase.h/.cpp` の `TryReadStringBinaryData` | 宣言と空の定義 |
| S4-1 | `Material/Converter/Binary/ModelMaterialBinaryConverter.h/.cpp` | 全関数が空(テンプレート3つも空) |
| S4-1 | `Material/ModelMaterialBase.h/.cpp` | ctor / dtor は完成、他は空 |
| S4-1 | `Material/Standard/Lit/ModelStandardLitMaterial.h/.cpp` | ctor / dtor / `FetchREFTableINFO` は完成、他は空 |
| S4-1 | `Material/Standard/UnLit/ModelStandardUnLitMaterial.h/.cpp` | `FetchREFTableINFO` は完成、`WriteGPUData` は空 |
| S4-2 | `Definition/Concept/Graphics/IsDeferredReleaseRecordConcept.h` | 完成 |
| S4-2 | `AssetRecordBase.h`(`ReserveRelease` を削除)/ Texture・StaticModel・SkeletalAnimationModel の Record(`override` を削除) | 完成 |
| S4-2 | `Storage/AssetStorage.h` | 既存の `SubtractReferenceCount` に `requires` を付け、遅延解放なし版を追加(中身は `return false;` で空) |
| S4-2 | `Material/Record/ModelMaterialRecord.h` | 完成 |
| S4-2 | `Material/ModelMaterial.h/.cpp` | ctor(3つ)/ dtor は完成、`operator=`(2つ)と他は空 |
| S4-2 | `Material/ModelMaterialSystem.h/.cpp` / `Converter/Json/ModelMaterialSystemJsonConverter.h/.cpp` | 全関数が空 |
| S4-2 | `ResourceContext.h`(メンバとゲッター)/ `ResourceContextJsonConverter.h`(キー) | 完成。**`ResourceContext.cpp` の `PostDeserialize` と `ResourceContextJsonConverter.cpp` は写経** |
| S4-2 | `Shader/Model/Standard/ModelStandardMaterial.hlsli` と `Lit/` `UnLit/` の `...Material.hlsli` | cbuffer と構造体は完成、取得関数は空(`(型)0` を返す仮の中身) |
| S4-2 | `CONFIG/Graphics/GraphicsCONFIG.json` の `ResourceContext.ModelMaterialSystem` | 完成 |
| S4-3 | `Struct::StaticModelMesh` / `SkeletalAnimationModelMesh` の `m_subMeshName`、`ModelMeshBinaryHeader::m_subMeshNameSize`、`ModelBinaryConverterConstant.h`(`.staticModel` / `.skeletalModel` / `k_emptySubMeshNameSize`) | 完成 |
| S4-3 | `ModelBinaryConverterBase.h/.cpp` の `CanLoad` | **引数を2つにした(中身は今のまま。写経で書き換える)** |
| S4-3 | `FBXModelLoaderBase.h/.cpp` の `FetchVALSubMeshName` と `k_emptyFBXNameLength` | 宣言と空の定義 |
| S4-3 | `Material/File/ModelMaterialFileCreator.h/.cpp` | 全関数が空(テンプレートも空) |
| S4-3 | `StaticModelSystem.h` / `SkeletalAnimationModelSystem.h` の `m_materialFileCreator` | メンバのみ |

**まだ触っていないもの(写経で書く)** : `ModelBinaryConverterBase` の4つのテンプレート関数、`Static|Skeletal の *BinaryConverter.cpp`、`Static|Skeletal の *FBXLoader.cpp`、`Static|Skeletal の *ModelSystem.cpp`、`GraphicsManager.cpp`、`ModelStandardLit_PS.hlsl` / `ModelStandardUnLit_PS.hlsl`(S3 の途中の形のまま。`FetchModelBaseColor` の引数が古い)。

**ModelComponent を設計から外した(2026-10-11、ユーザー指示「ModelComponent はまだ実装しないので設計に加えないで、S4 ~ S6 を実装する」)。**

- S4 は S4-1 ~ S4-3 だけにした。旧 S4-4(ModelComponent のマテリアルのスロット)は、S0 を設計し直すときの材料として `S0_ModelComponent.md` の末尾へ移した。
- 骨組みで作っていた `Definition/Struct/GameObject/GameObjectModelComponentStruct.h`(`GameObjectModelComponentMaterialSlot` / `ModelDrawMaterial`)は消した(vcxproj / filters / Framework.h からも外した)。
  - `ModelDrawMaterial` も要らなくなった。S5 の `ModelDrawRegistration` が、メッシュごとのマテリアルのハンドル(`Graphics::ModelMaterial`)を受け取り、
    テーブルの種類と番号は自分で取り出す(下の「S5 で使う S4 の関数」)。
- `ModelMaterialFileCreator::CreateDefaultModelMaterialFilePath` は、クラスの中でしか使わなくなったので private の static にした(骨組みは直し済み)。
- **写経のコードが変わったところ** : `ModelMaterialFileCreator.cpp` の関数の順番とコメント、`FBXModelLoaderBase::FetchVALSubMeshName` のコメント1行(どちらも下のコードに反映済み)。処理の中身は変わらない。

**S5 で使う S4 の関数**(ModelComponent が無くても、S5 の `ModelDrawRegistration` が使うので、そのまま写経する)

| 関数 | S5 での使い道 |
|---|---|
| `ModelMaterial::FetchVALMaterial` | ハンドルからマテリアル本体を取り出す(読み込めていなければ nullptr) |
| `ModelMaterialSystem::GetREFErrorMaterial` | nullptr のときの代わり(マゼンタ) |
| `ModelMaterialBase::FetchREFTableINFO` / `GetVALTableElementIndex` | 描画項目を入れる一覧(マテリアルの種類)と、ルート定数のマテリアルの番号 |

**Framework.h の位置** : `ModelMaterialFileCreator.h` は `ModelMaterialRuntimeTextureBuilder.h` の次(StaticModelSystem より前)。マテリアル一式は「モデル(マテリアル)」の区画として `SkeletalAnimationModel.h` の後・アップロードシステムの前(`ResourceContext.h` が `ModelMaterialSystem` を値で持つため)。`ModelMaterialBase.h` は `Struct::ModelRenderTableINFO` を前方宣言している(本体の定義は `ModelRenderSystem` の区画で、これより後ろ)。`IsDeferredReleaseRecordConcept.h` は `AssetStorage.h` の前。

**写経の順番(おすすめ)**
1. S4-1 : `TryReadStringBinaryData` → `ModelMaterialBinaryConverter` → `ModelMaterialBase` → Lit → UnLit
2. S4-2 : `AssetStorage` → `ModelMaterial` → `ModelMaterialSystem` → JSON(System / ResourceContext)→ `GraphicsManager` → HLSL(hlsli → PS)
3. S4-3 : `ModelBinaryConverterBase` → Static / Skeletal の BinaryConverter → `FBXModelLoaderBase` → 各 FBXLoader → `ModelMaterialFileCreator` → 各 ModelSystem

**写経のとき合わせること** : 既存の `TryReadWStringBinaryData` は「`m_mappedData` の確認が無く、wchar_t で割り切れるかを確認する」形になっている。`TryReadStringBinaryData` は、この既存の形に合わせて書く(char は 1 バイトなので割り切れの確認は要らない)。

## 目的

- メッシュに埋め込まれていた「取り込み時のマテリアル」(`Struct::ModelMaterial`)ではなく、**.mat ファイルのマテリアル**で描く。
- マテリアルの値は、S3 で作ったマテリアルのテーブル(種類ごと)に **変えたときだけ** 書く。PS はテーブルから読む。
- 同じ .mat を使うモデルが何体あっても、実体は1つ(参照数で共有。テクスチャと同じハンドル方式)。
- メッシュは「サブメッシュ名」を持つ。どのメッシュにどの .mat を使うかの割り当て(スロット)は、ModelComponent を設計し直すとき(S0)に決める。

### 2026-10-09 に合意した設計(引き継ぐもの)

- 継承は `ModelMaterialBase` → シェーダーごとの派生(`ModelStandardLitMaterial` / `ModelStandardUnLitMaterial` / 後にトゥーン)の2段。
- ファイル名は「シェーダー名_サブメッシュ名.mat」(`StandardLit_Body.mat`)、モデルと同じフォルダ。既にあれば上書きしない。FBX を初めて取り込むと全部 StandardLit で作る。
- .mat には型名(クラス名)を最初に書き、読むときはファクトリーでその型を作ってから値を読ませる。テクスチャはパスではなく Registry の UUID で保存する。
- スロットは名前で対応づけ、番号は保存しない。名前が見つからないスロットはエラーマテリアル(マゼンタ)。
- JsonConverter / BinaryConverter は必ずメンバに持つ(関数の中でローカルに作らない)。

### 2026-10-10 改訂(テーブルの種類をマクロで登録する形に合わせた)

- マテリアルの GPU データ(`ModelStandardLitMaterialGPUData` など)は、S3 の改訂で **クラス + `FWK_DEFINE_MODEL_MATERIAL_RENDER_TABLE_INFO`** にした(ファイルは作成済み)。
- マテリアルのクラスは、`Enum::ModelRenderTableType` を返す代わりに、**自分の GPU データの型のテーブルの情報**を返す。
  ```cpp
  const Struct::ModelRenderTableINFO& FetchREFTableINFO() const override;   // return ModelStandardLitMaterialGPUData::GetREFModelRenderTableINFO();
  ```
  基底クラス(`CreateGPUData`)は、その情報の StaticTypeID でテーブルを探す。マテリアルの種類が増えても、基底クラスと ModelRenderSystem は書き換えない。
- GPU データへの値は Set 関数で詰める(クラスにしたため)。
- 描画項目(S5)は、マテリアルのテーブルの種類を enum ではなく StaticTypeID(`FetchREFTableINFO()` の `k_typeINFO` の `k_staticTypeID`)で見分ける。
  `k_typeINFO` は生のポインタなので、使う前に必ず nullptr を確認する(2026-10-11 ユーザー指示)。
  (2026-10-11 : 間に挟んでいた `Struct::ModelDrawMaterial` は廃止。S5 の `ModelDrawRegistration` がマテリアルから直接取り出す)

### 撤回した設計の反省を反映した点

- 前回は ModelMaterialSystem が GPU テーブルを持っていたため、アプリ終了時の破棄順でアサートが出た。
  今回は **テーブルは Renderer の ModelRenderSystem が `shared_ptr` で持ち、マテリアルは `weak_ptr` で覚える**。
  テーブルが先に消えていれば、番号の返却を飛ばすだけで済む(S3 の ModelRenderSystem もこの形に合わせる。末尾の「S3 への修正」を参照)。
- 「全マテリアルに PBR のテクスチャ番号を詰める」ような無駄はしない。種類ごとに必要な値だけをテーブルに置く(UnLit は 20 バイト)。

## 小ステップ

| | 内容 |
|---|---|
| S4-1 | `ModelMaterialBase` / StandardLit / StandardUnLit と、.mat の読み書き(`ModelMaterialBinaryConverter`)、ファクトリー |
| S4-2 | `ModelMaterialRecord` / ハンドル `ModelMaterial` / `ModelMaterialSystem`(AssetStorage)/ エラーマテリアル / GPU テーブルへの書き込み / PS がテーブルを読む |
| S4-3 | メッシュにサブメッシュ名(FBX のマテリアル名)を持たせ、取り込み時に .mat を作る。モデルのキャッシュの拡張子を Static / Skeletal で分ける |
| (旧 S4-4) | ModelComponent のマテリアルのスロット → **S0 へ移した**(2026-10-11、ModelComponent を設計から外したため) |

## DirectX12 / 設計の解説

### マテリアルの値を「テーブル + 番号」にする理由

- マテリアルの値(色・ラフネス・テクスチャの SRV 番号)は、ほとんど変わらない。
- 1つのマテリアルを 100 体が使っていても、テーブルの要素は1つ(40 バイト)。描画ごとに送るのは番号(4 バイト)だけ。
- 値を変えたとき(エディターで色を変えた)だけ、その1要素を書けば、次のフレームから全員に反映される。

### PS がマテリアルを読む流れ

```hlsl
// ルート定数(描画ごと) : g_materialIndex
// ルート定数(パスごと) : g_materialTableSRVDescriptorIndex
StructuredBuffer<ModelStandardLitMaterialData> l_materialTable = ResourceDescriptorHeap[g_materialTableSRVDescriptorIndex];
const ModelStandardLitMaterialData l_material = l_materialTable[g_materialIndex];
```

- Lit のパスは Lit のテーブルの SRV 番号を、UnLit のパスは UnLit のテーブルの番号を、パスの最初に1回送る。
- 種類が違うマテリアルを、同じパス(同じ PSO)で描くことはない(S5 で、マテリアルの種類ごとに描画項目を振り分ける)。

### テクスチャのハンドルと参照数

- `Texture`(ハンドル)をコピーすると参照数が1増え、破棄すると1減る。0 になると GPU の使用が終わってから解放される。
- マテリアルは `Texture` のハンドルを持つので、マテリアルが生きている間はテクスチャも生きている。
- テクスチャの SRV の番号は、読み込んだ時点で決まる(GPU へのコピーは次のフレームの最初)。そのため、読み込んだ直後にテーブルへ書いてよい。

---

# S4-1 マテリアルのクラスと .mat の読み書き

## ファイル一覧

### 新規

| ファイル | 内容 |
|---|---|
| `Source/Framework/Definition/Constant/Graphics/ModelMaterialConstant.h` | 拡張子 `.mat`・既定のサブメッシュ名・ファイル名の接頭辞・エラーマテリアルの色 |
| `Source/Framework/Definition/Concept/IsDerivedBase/Graphics/IsDerivedModelMaterialBaseConcept.h` | `Concept::IsDerivedModelMaterialBaseConcept` |
| `Source/Framework/Definition/Type/Alias/Factory/Shared/ModelMaterialSharedFactoryTypeAlias.h` | `TypeAlias::ModelMaterialSharedFactory` |
| `Source/Framework/Graphics/Resource/Model/Material/Converter/Binary/ModelMaterialBinaryConverter.h/.cpp` | .mat の読み書き |
| `Source/Framework/Graphics/Resource/Model/Material/ModelMaterialBase.h/.cpp` | 基底(ベースカラーとそのテクスチャ) |
| `Source/Framework/Graphics/Resource/Model/Material/Standard/Lit/ModelStandardLitMaterial.h/.cpp` | 法線・メタリック・ラフネス |
| `Source/Framework/Graphics/Resource/Model/Material/Standard/UnLit/ModelStandardUnLitMaterial.h/.cpp` | 追加の値なし |

### 変更

| ファイル | 変更 |
|---|---|
| `Converter/Binary/BinaryConverterBase.h/.cpp` | `TryReadStringBinaryData`(型名の読み込み用) |
| `Definition/Enum/Asset/AssetFilePathEnum.h` | `AssetFilePathType::ModelMaterial` |

### 登録

- フィルター: `...\Graphics\Resource\Model\Material` / `...\Material\Converter` / `...\Material\Converter\Binary` / `...\Material\Standard` / `...\Material\Standard\Lit` / `...\Material\Standard\UnLit`
- Framework.h: Constant → Concept → TypeAlias の各区画に1行ずつ。本体は `ModelMaterialBinaryConverter.h` → `ModelMaterialBase.h` → `ModelStandardLitMaterial.h` → `ModelStandardUnLitMaterial.h`(Texture / AssetFilePath / ModelRenderSystem より後)

## コード

### Definition/Constant/Graphics/ModelMaterialConstant.h(新規)

```cpp
#pragma once

namespace FWK::Constant
{
    inline const std::filesystem::path k_lowerModelMaterialExtension = ".mat";

    inline constexpr std::wstring_view k_modelMaterialDefaultSubMeshName     = L"Default";
    inline constexpr std::wstring_view k_standardLitModelMaterialFilePrefix  = L"StandardLit_";

    inline constexpr TypeAlias::Math::Color k_errorModelMaterialBaseColor = { 1.0F,
                                                                              0.0F,
                                                                              1.0F,
                                                                              1.0F };
}
```

### Definition/Concept/IsDerivedBase/Graphics/IsDerivedModelMaterialBaseConcept.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelMaterialBase;
}

namespace FWK::Concept
{
    template <typename Type>
    concept IsDerivedModelMaterialBaseConcept = IsDerivedBaseConcept<Type, Graphics::ModelMaterialBase>;
}
```

### Definition/Type/Alias/Factory/Shared/ModelMaterialSharedFactoryTypeAlias.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelMaterialBase;
}

namespace FWK::TypeAlias
{
    using ModelMaterialSharedFactory = GenericFactory<std::shared_ptr<Graphics::ModelMaterialBase>>;
}
```

> 既存の `DrawRequestPerObjectSharedFactoryTypeAlias.h` と同じ形。前方宣言の書き方は既存ファイルに合わせる。

### Definition/Enum/Asset/AssetFilePathEnum.h(変更)

`Model` の後ろに `ModelMaterial` を足す(`FWK_JSON_SERIALIZE_ENUM` にも)。
S0 で足した Watcher(削除 / 名前変更)の `case` にも、Model と同じ中身で `ModelMaterial` を足す。

### Converter/Binary/BinaryConverterBase(変更)

`.h` の `TryReadWStringBinaryData` の下に、std::string 版を足す。

```cpp
        bool TryReadWStringBinaryData(const std::uint64_t& a_wStringBinaryFileSize, std::wstring& a_destinationString, std::uint64_t& a_memoryReadOffset) const;
        bool TryReadStringBinaryData (const std::uint64_t& a_stringBinaryFileSize,  std::string&  a_destinationString, std::uint64_t& a_memoryReadOffset) const;
```

`.cpp`(写経):

```cpp
bool FWK::Converter::BinaryConverterBase::TryReadStringBinaryData(const std::uint64_t& a_stringBinaryFileSize, std::string& a_destinationString, std::uint64_t& a_memoryReadOffset) const
{
    // 保存されている文字列のバイト数が0なら、空の文字列として読み込み成功にする
    if (a_stringBinaryFileSize == k_emptyReadDataSize)
    {
        a_destinationString.clear();

        return true;
    }

    // 保存されているバイト数が、ファイルの残りに収まっているかを確かめてから読む
    // charは1バイトなので、wstringのような「割り切れるか」の確認は要らない
    if (!CanReadBinaryData(a_memoryReadOffset, a_stringBinaryFileSize)) { return false; }

    // string情報を読み込む
    ReadStringBinaryData(a_stringBinaryFileSize, a_destinationString, a_memoryReadOffset);

    return true;
}
```

> 既存の `TryReadWStringBinaryData` と同じ形。`ReadStringBinaryData`(private)は既にある。

### Graphics/Resource/Model/Material/Converter/Binary/ModelMaterialBinaryConverter.h(新規)

```cpp
#pragma once

namespace FWK
{
    class AssetFilePath;
}

namespace FWK::Graphics
{
    class ModelMaterialBase;
}

namespace FWK::Converter
{
    class ModelMaterialBinaryConverter final : public BinaryConverterBase
    {
    private:

        struct ModelMaterialBinaryHeader final
        {
            std::uint64_t m_fileSize     = k_emptyAssetFileSize;
            std::uint64_t m_typeNameSize = k_emptyAssetFileSize;
            std::uint16_t m_version      = k_modelMaterialAssetVersion;
            std::uint16_t m_assetTypeID  = k_modelMaterialAssetTypeID;
        };

    public:

         ModelMaterialBinaryConverter()          = default;
        ~ModelMaterialBinaryConverter() override = default;

        bool Load(const std::filesystem::path& a_filePath, Graphics::ModelMaterialBase& a_material);

        bool Save(const std::filesystem::path& a_filePath, const Graphics::ModelMaterialBase& a_material);

        bool TryReadTypeName(const std::filesystem::path& a_filePath, std::string& a_typeName);

        bool TryReadAssetFilePath(AssetFilePath& a_assetFilePath, std::uint64_t& a_memoryReadOffset) const;

        template <typename Type>
        bool TryReadMaterialValue(Type& a_value, std::uint64_t& a_memoryReadOffset) const
        {
            // 色・数値などの固定長の値を、そのままのバイト列として読む
            // 書き込み(WriteMaterialValue)と同じ型・同じ順番で読まないと、値がずれてしまう
            return TryReadSingleBinaryData(a_value, a_memoryReadOffset);
        }

        void WriteAssetFilePath(const AssetFilePath& a_assetFilePath, std::uint64_t& a_memoryWriteOffset) const;

        template <typename Type>
        void WriteMaterialValue(const Type& a_value, std::uint64_t& a_memoryWriteOffset) const
        {
            // 色・数値などの固定長の値を、そのままのバイト列として書く
            WriteBinaryData(k_singleBinaryElementCount, &a_value, a_memoryWriteOffset);
        }

        std::uint64_t CalculateAssetFilePathBinaryDataSize() const;

        template <typename Type>
        std::uint64_t CalculateMaterialValueBinaryDataSize() const
        {
            return CalculateBinaryDataSize<Type>(k_singleBinaryElementCount);
        }

    private:

        bool TryReadModelMaterialBinaryHeader(ModelMaterialBinaryHeader& a_header, std::uint64_t& a_memoryReadOffset) const;

        // 'M' = 0x4D, 'T' = 0x54のため、0x4D54で"MT"を表す
        static constexpr std::uint16_t k_modelMaterialAssetTypeID = 0x4D54U;

        // ※ 注意 : .matに保存する値が変化したらバージョンを上げる
        static constexpr std::uint16_t k_modelMaterialAssetVersion = 1U;
    };
}
```

> `ModelMaterialBinaryHeader` は、このクラスの中だけで使うので private の入れ子(規約 1-3)。
> 値を読む関数を public にしているのは、マテリアルの各クラスが「自分の値を、自分の順番で」読み書きするため(派生クラスが増えても、このクラスは変えなくてよい)。

### Graphics/Resource/Model/Material/Converter/Binary/ModelMaterialBinaryConverter.cpp(新規・写経)

```cpp
#include "ModelMaterialBinaryConverter.h"

// .matファイルの読み書きを行うクラス
// ファイルの中身 : ヘッダー(ファイルの大きさ・型名の大きさ・バージョン・種類のID) → 型名(クラス名) → 値
// 値の中身はマテリアルの種類ごとに違うため、マテリアル自身のTryReadBinaryData / WriteBinaryDataに任せる
// 例 : StandardLitなら、基底の値(ベースカラーとそのテクスチャのUUID) → Litの値(法線などのテクスチャのUUID、メタリック、ラフネス)
bool FWK::Converter::ModelMaterialBinaryConverter::Load(const std::filesystem::path& a_filePath, Graphics::ModelMaterialBase& a_material)
{
    if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerModelMaterialExtension)) { return false; }

    if (!CreateReadMemoryMappedFile(a_filePath)) { return false; }

    auto                      l_memoryReadOffset = k_initialMemoryReadOffset;
    ModelMaterialBinaryHeader l_header           = {};
    std::string               l_typeName         = {};

    // ヘッダーと型名を読み、型名がこのマテリアルのクラス名と同じかを確かめる
    // 違えば、別の種類の.matを間違えて読もうとしている
    const bool l_isHeaderRead = TryReadModelMaterialBinaryHeader(l_header, l_memoryReadOffset)                    &&
                                TryReadStringBinaryData         (l_header.m_typeNameSize, l_typeName, l_memoryReadOffset);

    const auto& l_typeINFO = a_material.GetREFRuntimeTypeINFO();

    if (!l_isHeaderRead ||
        l_typeName != l_typeINFO.k_name)
    {
        DestroyMemoryMappedFile();

        return false;
    }

    // 値の読み込みはマテリアル自身に任せる
    // 派生クラスは、先に基底クラスのTryReadBinaryDataを呼んでから自分の値を読む
    const bool l_isValueRead = a_material.TryReadBinaryData(*this, l_memoryReadOffset);

    // 最後まで読み終わった位置が、ヘッダーに書いたファイルの大きさと同じでなければ、ファイルが壊れている
    const bool l_isEndMatched = l_memoryReadOffset == l_header.m_fileSize;

    DestroyMemoryMappedFile();

    return l_isValueRead &&
           l_isEndMatched;
}

bool FWK::Converter::ModelMaterialBinaryConverter::Save(const std::filesystem::path& a_filePath, const Graphics::ModelMaterialBase& a_material)
{
    const auto& l_typeINFO = a_material.GetREFRuntimeTypeINFO();
    const auto& l_typeName = std::string{ l_typeINFO.k_name };

    ModelMaterialBinaryHeader l_header = {};

    // ファイル全体の大きさ = ヘッダー + 型名 + 値
    // メモリマップドファイルは、最初に大きさを決めてから書き込むため、先に計算する
    l_header.m_typeNameSize = CalculateStringBinaryFileSize(l_typeName);
    l_header.m_fileSize     = CalculateBinaryDataSize<ModelMaterialBinaryHeader>(k_singleBinaryElementCount) +
                              l_header.m_typeNameSize                                                        +
                              a_material.CalculateBinaryDataSize(*this);

    if (!CreateWriteMemoryMappedFile(a_filePath, l_header.m_fileSize)) { return false; }

    auto l_memoryWriteOffset = k_initialMemoryWriteOffset;

    WriteBinaryData      (k_singleBinaryElementCount, &l_header, l_memoryWriteOffset);
    WriteStringBinaryData(l_typeName,                            l_memoryWriteOffset);

    // 値の書き込みはマテリアル自身に任せる
    a_material.WriteBinaryData(*this, l_memoryWriteOffset);

    DestroyMemoryMappedFile();

    return true;
}

bool FWK::Converter::ModelMaterialBinaryConverter::TryReadTypeName(const std::filesystem::path& a_filePath, std::string& a_typeName)
{
    // .matの先頭から、保存したときのマテリアルの種類の名前(クラス名)だけを読む
    // ModelMaterialSystemは、この名前でファクトリーから正しい派生クラスを作り、それから値を読ませる
    // 例 : StandardLitで保存した.matなら"ModelStandardLitMaterial"
    if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerModelMaterialExtension)) { return false; }

    if (!CreateReadMemoryMappedFile(a_filePath)) { return false; }

    auto                      l_memoryReadOffset = k_initialMemoryReadOffset;
    ModelMaterialBinaryHeader l_header           = {};

    const bool l_isRead = TryReadModelMaterialBinaryHeader(l_header, l_memoryReadOffset) &&
                          TryReadStringBinaryData         (l_header.m_typeNameSize, a_typeName, l_memoryReadOffset);

    DestroyMemoryMappedFile();

    return l_isRead;
}

bool FWK::Converter::ModelMaterialBinaryConverter::TryReadAssetFilePath(AssetFilePath& a_assetFilePath, std::uint64_t& a_memoryReadOffset) const
{
    // テクスチャはファイルパスではなく、Registryに登録したUUID(16バイト)で保存している
    // UUIDなら、テクスチャの名前を変えたり移動したりしても、同じテクスチャを辿れる
    boost::uuids::uuid l_assetFilePathUUID = {};

    if (!TryReadSingleBinaryData(l_assetFilePathUUID, a_memoryReadOffset)) { return false; }

    a_assetFilePath.SetAssetFilePathUUID(l_assetFilePathUUID);

    return true;
}

void FWK::Converter::ModelMaterialBinaryConverter::WriteAssetFilePath(const AssetFilePath& a_assetFilePath, std::uint64_t& a_memoryWriteOffset) const
{
    const auto& l_assetFilePathUUID = a_assetFilePath.GetREFAssetFilePathUUID();

    WriteBinaryData(k_singleBinaryElementCount, &l_assetFilePathUUID, a_memoryWriteOffset);
}

std::uint64_t FWK::Converter::ModelMaterialBinaryConverter::CalculateAssetFilePathBinaryDataSize() const
{
    return CalculateBinaryDataSize<boost::uuids::uuid>(k_singleBinaryElementCount);
}

bool FWK::Converter::ModelMaterialBinaryConverter::TryReadModelMaterialBinaryHeader(ModelMaterialBinaryHeader& a_header, std::uint64_t& a_memoryReadOffset) const
{
    if (!TryReadSingleBinaryData(a_header, a_memoryReadOffset)) { return false; }

    // .mat以外のファイル、または古い形式なら読まない
    if (a_header.m_assetTypeID != k_modelMaterialAssetTypeID) { return false; }
    if (a_header.m_version != k_modelMaterialAssetVersion)    { return false; }

    // ヘッダーに書いた大きさと、実際のファイルの大きさが違えば、ファイルが壊れている
    if (a_header.m_fileSize != GetVALMappedDataSize()) { return false; }

    return true;
}
```

> `Load` / `Save` / `TryReadTypeName` はメモリマップドファイルを開く(状態を変える)ので const を付けない(規約 7-12)。

### Graphics/Resource/Model/Material/ModelMaterialBase.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelMaterialBase
    {
    public:

                 ModelMaterialBase();
        virtual ~ModelMaterialBase();

        ModelMaterialBase(const ModelMaterialBase&)  = delete;
        ModelMaterialBase(      ModelMaterialBase&&) = delete;

        ModelMaterialBase& operator=(const ModelMaterialBase&)  = delete;
        ModelMaterialBase& operator=(      ModelMaterialBase&&) = delete;

        bool Load(const std::filesystem::path& a_filePath);

        bool Save(const std::filesystem::path& a_filePath);

        virtual bool TryReadBinaryData(const Converter::ModelMaterialBinaryConverter& a_binaryConverter, std::uint64_t& a_memoryReadOffset);
        virtual void WriteBinaryData  (const Converter::ModelMaterialBinaryConverter& a_binaryConverter, std::uint64_t& a_memoryWriteOffset) const;

        virtual std::uint64_t CalculateBinaryDataSize(const Converter::ModelMaterialBinaryConverter& a_binaryConverter) const;

        virtual void LoadRuntimeTextures();

        bool CreateGPUData();

        void ApplyGPUData() const;

        void SetBaseColorTexture(const AssetFilePath& a_set) { m_baseColorTexture = a_set; }

        void SetBaseColor(const TypeAlias::Math::Color& a_set) { m_baseColor = a_set; }

        virtual const Struct::ModelRenderTableINFO& FetchREFTableINFO() const = 0;

        const auto& GetREFBaseColorTexture() const { return m_baseColorTexture; }

        const auto& GetREFBaseColorRuntimeTexture() const { return m_baseColorRuntimeTexture; }

        const auto& GetREFBaseColor() const { return m_baseColor; }

        auto GetVALTableElementIndex() const { return m_tableElementIndex; }

    protected:

        virtual void WriteGPUData(GPUElementTable& a_table, const std::uint32_t a_tableElementIndex) const = 0;

        void LoadRuntimeTexture(const AssetFilePath&              a_assetFilePath,
                                const Enum::TextureLoadColorSpace a_textureLoadColorSpace,
                                const Enum::DefaultTextureType    a_defaultTextureType,
                                      Texture&                    a_runtimeTexture) const;

        TypeAlias::DescriptorIndex FetchVALTextureSRVDescriptorIndex(const Texture& a_texture) const;

    private:

        void ReleaseGPUData();

        std::weak_ptr<GPUElementTable> m_table;

        AssetFilePath m_baseColorTexture;
        Texture       m_baseColorRuntimeTexture;

        Converter::ModelMaterialBinaryConverter m_binaryConverter;

        TypeAlias::Math::Color m_baseColor;

        std::uint32_t m_tableElementIndex;

        FWK_DEFINE_TYPE_INFO_ROOT(ModelMaterialBase)
    };
}
```

> - テーブルの番号を持ち、デストラクタで返すため、コピー・ムーブは禁止。
> - `AssetFilePath` の受け取る種類(Texture)をコンストラクタで設定するので、コンストラクタは `= default` にせず、全メンバを初期化子リストで初期化する(規約 7-6)。
> - `Texture` はハンドル(参照数付き)。`SetBaseColorTexture` で UUID を変えたら、`LoadRuntimeTextures` → `ApplyGPUData` で反映する(エディターはフェーズ2)。

### Graphics/Resource/Model/Material/ModelMaterialBase.cpp(新規・写経)

```cpp
#include "ModelMaterialBase.h"

// すべてのマテリアルの基底クラス
// 持つ値 : ベースカラー(色)と、そのテクスチャ(AssetFilePath = RegistryのUUID / Texture = 読み込んだテクスチャのハンドル)
// 派生クラス(StandardLit / StandardUnLit / 後にトゥーン)は、シェーダーごとに必要な値だけを足す
// 描画用の値は、種類ごとのテーブル(ModelRenderSystemのGPUElementTable)の1要素に置く
// テーブルはRendererが持つため、ここではweak_ptrで覚える(アプリの終了時、テーブルが先に消えても問題ないようにするため)
FWK::Graphics::ModelMaterialBase::ModelMaterialBase() :
    m_table(),

    m_baseColorTexture       (),
    m_baseColorRuntimeTexture(),

    m_binaryConverter(),

    m_baseColor(Constant::k_whiteColor),

    m_tableElementIndex(GPUElementTable::k_invalidElementIndex)
{
    // ベースカラーのテクスチャは、テクスチャ(PNG)だけを受け取る
    m_baseColorTexture.SetAllowedType(Enum::AssetFilePathType::Texture);
}
FWK::Graphics::ModelMaterialBase::~ModelMaterialBase()
{
    ReleaseGPUData();
}

bool FWK::Graphics::ModelMaterialBase::Load(const std::filesystem::path& a_filePath)
{
    // .matから値を読み、その値でテクスチャを読み込む
    // 値の読み込みは「基底の値 → 派生の値」の順(BinaryConverterが、このクラスのTryReadBinaryDataを呼ぶ)
    if (!m_binaryConverter.Load(a_filePath, *this)) { return false; }

    LoadRuntimeTextures();

    return true;
}

bool FWK::Graphics::ModelMaterialBase::Save(const std::filesystem::path& a_filePath)
{
    return m_binaryConverter.Save(a_filePath, *this);
}

bool FWK::Graphics::ModelMaterialBase::TryReadBinaryData(const Converter::ModelMaterialBinaryConverter& a_binaryConverter, std::uint64_t& a_memoryReadOffset)
{
    // 書き込み(WriteBinaryData)と同じ順番で読む
    if (!a_binaryConverter.TryReadMaterialValue(m_baseColor, a_memoryReadOffset))          { return false; }
    if (!a_binaryConverter.TryReadAssetFilePath(m_baseColorTexture, a_memoryReadOffset))   { return false; }

    return true;
}
void FWK::Graphics::ModelMaterialBase::WriteBinaryData(const Converter::ModelMaterialBinaryConverter& a_binaryConverter, std::uint64_t& a_memoryWriteOffset) const
{
    a_binaryConverter.WriteMaterialValue(m_baseColor,        a_memoryWriteOffset);
    a_binaryConverter.WriteAssetFilePath(m_baseColorTexture, a_memoryWriteOffset);
}

std::uint64_t FWK::Graphics::ModelMaterialBase::CalculateBinaryDataSize(const Converter::ModelMaterialBinaryConverter& a_binaryConverter) const
{
    // 例 : Color(16バイト) + UUID(16バイト) = 32バイト
    return a_binaryConverter.CalculateMaterialValueBinaryDataSize<TypeAlias::Math::Color>() +
           a_binaryConverter.CalculateAssetFilePathBinaryDataSize                          ();
}

void FWK::Graphics::ModelMaterialBase::LoadRuntimeTextures()
{
    // ベースカラーは色なので、sRGB(人の目に合わせて明るさを曲げた色)として読む
    // 見つからなければ、白の既定テクスチャが入る(色をそのまま掛けても変わらない)
    LoadRuntimeTexture(m_baseColorTexture, Enum::TextureLoadColorSpace::SRGB, Enum::DefaultTextureType::BaseColor, m_baseColorRuntimeTexture);
}

bool FWK::Graphics::ModelMaterialBase::CreateGPUData()
{
    // 既に番号を持っていれば、作り直さずに値だけ書き直す
    if (m_tableElementIndex != GPUElementTable::k_invalidElementIndex)
    {
        ApplyGPUData();

        return true;
    }

    const auto& l_graphicsManager   = GraphicsManager::GetInstance      ();
    const auto& l_renderer          = l_graphicsManager.GetREFRenderer  ();
    const auto& l_modelRenderSystem = l_renderer.GetREFModelRenderSystem();

    // 自分の種類(StandardLitなど)のテーブルの情報を、派生クラスから受け取る
    // 情報のStaticTypeID(GPUデータの型ごとに決まる番号)でテーブルを探し、番号を1つもらう
    // 例 : ModelStandardLitMaterialなら、ModelStandardLitMaterialGPUData(40バイト)のテーブル
    const auto& l_tableINFO = FetchREFTableINFO();
    const auto* l_typeINFO  = l_tableINFO.k_typeINFO;

    FWK_ASSERT_RETURN_VALUE_IF(!l_typeINFO, "テーブルの情報のTypeINFOが無効のため、マテリアルのGPUデータの作成に失敗しました。", false);

    m_table = l_modelRenderSystem.FindVALTable(l_typeINFO->k_staticTypeID);

    const auto& l_table = m_table.lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_table, "マテリアルの種類に対応するテーブルが無いため(GraphicsCONFIG.jsonのTableMapに書いてあるか確認)、マテリアルのGPUデータの作成に失敗しました。", false);

    m_tableElementIndex = l_table->AllocateElementIndex();

    FWK_ASSERT_RETURN_VALUE_IF(m_tableElementIndex == GPUElementTable::k_invalidElementIndex, "テーブルの番号の割り当てに失敗したため、マテリアルのGPUデータの作成に失敗しました。", false);

    ApplyGPUData();

    return true;
}

void FWK::Graphics::ModelMaterialBase::ApplyGPUData() const
{
    if (m_tableElementIndex == GPUElementTable::k_invalidElementIndex) { return; }

    const auto& l_table = m_table.lock();

    if (!l_table) { return; }

    // 値の詰め方は種類ごとに違うので、派生クラスに任せる
    // 書いた要素は、次のフレームの最初にGPUへコピーされる
    WriteGPUData(*l_table, m_tableElementIndex);
}

void FWK::Graphics::ModelMaterialBase::LoadRuntimeTexture(const AssetFilePath&              a_assetFilePath,
                                                          const Enum::TextureLoadColorSpace a_textureLoadColorSpace,
                                                          const Enum::DefaultTextureType    a_defaultTextureType,
                                                                Texture&                    a_runtimeTexture) const
{
    // RegistryのUUIDから、今のファイルパスを引いてテクスチャを読み込む
    // UUIDが空・見つからない場合は空のパスになり、既定のテクスチャ(色なら白、法線なら平らな面)が入る
    const auto& l_filePath = a_assetFilePath.FetchVALFilePath();

    a_runtimeTexture.Load(l_filePath, a_textureLoadColorSpace, a_defaultTextureType);
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::ModelMaterialBase::FetchVALTextureSRVDescriptorIndex(const Texture& a_texture) const
{
    // テクスチャのSRVの番号は、読み込んだ時点で決まっている(GPUへのコピーは次のフレームの最初)
    const auto& l_textureRecord = a_texture.GetREFTextureRecord().lock();

    if (!l_textureRecord) { return DescriptorHeap::k_invalidDescriptorIndex; }

    return l_textureRecord->GetVALSRVDescriptorIndex();
}

void FWK::Graphics::ModelMaterialBase::ReleaseGPUData()
{
    if (m_tableElementIndex == GPUElementTable::k_invalidElementIndex) { return; }

    // テーブルが既に破棄されている(アプリの終了時)なら、返す先がないので何もしない
    if (const auto& l_table = m_table.lock();
        l_table)
    {
        l_table->ReleaseElementIndex(m_tableElementIndex);
    }

    m_tableElementIndex = GPUElementTable::k_invalidElementIndex;
}
```


### Graphics/Resource/Model/Material/Standard/Lit/ModelStandardLitMaterial.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelStandardLitMaterial final : public ModelMaterialBase
    {
    public:

         ModelStandardLitMaterial();
        ~ModelStandardLitMaterial() override;

        bool TryReadBinaryData(const Converter::ModelMaterialBinaryConverter& a_binaryConverter, std::uint64_t& a_memoryReadOffset)        override;
        void WriteBinaryData  (const Converter::ModelMaterialBinaryConverter& a_binaryConverter, std::uint64_t& a_memoryWriteOffset) const override;

        std::uint64_t CalculateBinaryDataSize(const Converter::ModelMaterialBinaryConverter& a_binaryConverter) const override;

        void LoadRuntimeTextures() override;

        void SetNormalTexture   (const AssetFilePath& a_set) { m_normalTexture    = a_set; }
        void SetMetallicTexture (const AssetFilePath& a_set) { m_metallicTexture  = a_set; }
        void SetRoughnessTexture(const AssetFilePath& a_set) { m_roughnessTexture = a_set; }

        void SetMetallic (const float a_set) { m_metallic  = a_set; }
        void SetRoughness(const float a_set) { m_roughness = a_set; }

        const Struct::ModelRenderTableINFO& FetchREFTableINFO() const override;

    protected:

        void WriteGPUData(GPUElementTable& a_table, const std::uint32_t a_tableElementIndex) const override;

    private:

        AssetFilePath m_normalTexture;
        AssetFilePath m_metallicTexture;
        AssetFilePath m_roughnessTexture;

        Texture m_normalRuntimeTexture;
        Texture m_metallicRuntimeTexture;
        Texture m_roughnessRuntimeTexture;

        float m_metallic;
        float m_roughness;

        FWK_DEFINE_TYPE_INFO(ModelStandardLitMaterial, ModelMaterialBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ModelMaterialSharedFactory, FWK::Graphics::ModelStandardLitMaterial)
```

> 取得関数(GetREF)は、使う所(インスペクター、フェーズ2)ができたときに足す(規約 6-4)。

### Graphics/Resource/Model/Material/Standard/Lit/ModelStandardLitMaterial.cpp(新規・写経)

```cpp
#include "ModelStandardLitMaterial.h"

// 光の当たり方を計算する(Lit)標準のマテリアル
// 基底の値(ベースカラー)に加えて、法線・メタリック・ラフネスのテクスチャと値を持つ
// GPUへは、ModelStandardLitMaterialGPUData(40バイト)としてStandardLitのテーブルへ書く
FWK::Graphics::ModelStandardLitMaterial::ModelStandardLitMaterial() :
    m_normalTexture   (),
    m_metallicTexture (),
    m_roughnessTexture(),

    m_normalRuntimeTexture   (),
    m_metallicRuntimeTexture (),
    m_roughnessRuntimeTexture(),

    m_metallic (ModelStandardLitMaterialGPUData::k_defaultMetallic),
    m_roughness(ModelStandardLitMaterialGPUData::k_defaultRoughness)
{
    m_normalTexture.SetAllowedType   (Enum::AssetFilePathType::Texture);
    m_metallicTexture.SetAllowedType (Enum::AssetFilePathType::Texture);
    m_roughnessTexture.SetAllowedType(Enum::AssetFilePathType::Texture);
}
FWK::Graphics::ModelStandardLitMaterial::~ModelStandardLitMaterial() = default;

bool FWK::Graphics::ModelStandardLitMaterial::TryReadBinaryData(const Converter::ModelMaterialBinaryConverter& a_binaryConverter, std::uint64_t& a_memoryReadOffset)
{
    // 先に基底の値(ベースカラー)を読む
    if (!ModelMaterialBase::TryReadBinaryData(a_binaryConverter, a_memoryReadOffset)) { return false; }

    if (!a_binaryConverter.TryReadAssetFilePath(m_normalTexture, a_memoryReadOffset))    { return false; }
    if (!a_binaryConverter.TryReadAssetFilePath(m_metallicTexture, a_memoryReadOffset))  { return false; }
    if (!a_binaryConverter.TryReadAssetFilePath(m_roughnessTexture, a_memoryReadOffset)) { return false; }
    if (!a_binaryConverter.TryReadMaterialValue(m_metallic, a_memoryReadOffset))         { return false; }
    if (!a_binaryConverter.TryReadMaterialValue(m_roughness, a_memoryReadOffset))        { return false; }

    return true;
}
void FWK::Graphics::ModelStandardLitMaterial::WriteBinaryData(const Converter::ModelMaterialBinaryConverter& a_binaryConverter, std::uint64_t& a_memoryWriteOffset) const
{
    ModelMaterialBase::WriteBinaryData  (a_binaryConverter,  a_memoryWriteOffset);
    a_binaryConverter.WriteAssetFilePath(m_normalTexture,    a_memoryWriteOffset);
    a_binaryConverter.WriteAssetFilePath(m_metallicTexture,  a_memoryWriteOffset);
    a_binaryConverter.WriteAssetFilePath(m_roughnessTexture, a_memoryWriteOffset);
    a_binaryConverter.WriteMaterialValue(m_metallic,         a_memoryWriteOffset);
    a_binaryConverter.WriteMaterialValue(m_roughness,        a_memoryWriteOffset);
}

std::uint64_t FWK::Graphics::ModelStandardLitMaterial::CalculateBinaryDataSize(const Converter::ModelMaterialBinaryConverter& a_binaryConverter) const
{
    // 例 : 基底32バイト + UUID 3つ(48バイト) + float 2つ(8バイト) = 88バイト
    return ModelMaterialBase::CalculateBinaryDataSize               (a_binaryConverter)               +
           a_binaryConverter.CalculateAssetFilePathBinaryDataSize   ()                                * k_litTextureCount +
           a_binaryConverter.CalculateMaterialValueBinaryDataSize<float>()                            * k_litValueCount;
}

void FWK::Graphics::ModelStandardLitMaterial::LoadRuntimeTextures()
{
    ModelMaterialBase::LoadRuntimeTextures();

    // 法線・メタリック・ラフネスは「色」ではなく「数値」なので、Linear(明るさを曲げない)で読む
    // sRGBで読むと、0.5が約0.21になってしまい、凹凸や金属っぽさが狂う
    LoadRuntimeTexture(m_normalTexture,    Enum::TextureLoadColorSpace::Linear, Enum::DefaultTextureType::Normal,    m_normalRuntimeTexture);
    LoadRuntimeTexture(m_metallicTexture,  Enum::TextureLoadColorSpace::Linear, Enum::DefaultTextureType::Metallic,  m_metallicRuntimeTexture);
    LoadRuntimeTexture(m_roughnessTexture, Enum::TextureLoadColorSpace::Linear, Enum::DefaultTextureType::Roughness, m_roughnessRuntimeTexture);
}

const FWK::Struct::ModelRenderTableINFO& FWK::Graphics::ModelStandardLitMaterial::FetchREFTableINFO() const
{
    // このマテリアルがGPUへ送る型(ModelStandardLitMaterialGPUData)の、テーブルの情報を返す
    // 情報は、その型に書いたマクロ(FWK_DEFINE_MODEL_MATERIAL_RENDER_TABLE_INFO)が作ったもの
    return ModelStandardLitMaterialGPUData::GetREFModelRenderTableINFO();
}

void FWK::Graphics::ModelStandardLitMaterial::WriteGPUData(GPUElementTable& a_table, const std::uint32_t a_tableElementIndex) const
{
    // テクスチャは、SRVの番号だけを書く(シェーダーはResourceDescriptorHeap[番号]で取り出す)
    const auto& l_baseColorRuntimeTexture            = GetREFBaseColorRuntimeTexture    ();
    const auto  l_baseColorTextureSRVDescriptorIndex = FetchVALTextureSRVDescriptorIndex(l_baseColorRuntimeTexture);
    const auto  l_normalTextureSRVDescriptorIndex    = FetchVALTextureSRVDescriptorIndex(m_normalRuntimeTexture);
    const auto  l_metallicTextureSRVDescriptorIndex  = FetchVALTextureSRVDescriptorIndex(m_metallicRuntimeTexture);
    const auto  l_roughnessTextureSRVDescriptorIndex = FetchVALTextureSRVDescriptorIndex(m_roughnessRuntimeTexture);

    // シェーダー(ModelStandardLitMaterial.hlsli)のModelStandardLitMaterialDataと同じ並びの型(ModelStandardLitMaterialGPUData)に、Set関数で詰める
    ModelStandardLitMaterialGPUData l_gpuData = {};

    l_gpuData.SetBaseColor(GetREFBaseColor());
    l_gpuData.SetMetallic (m_metallic);
    l_gpuData.SetRoughness(m_roughness);

    l_gpuData.SetBaseColorTextureSRVDescriptorIndex(l_baseColorTextureSRVDescriptorIndex);
    l_gpuData.SetNormalTextureSRVDescriptorIndex   (l_normalTextureSRVDescriptorIndex);
    l_gpuData.SetMetallicTextureSRVDescriptorIndex (l_metallicTextureSRVDescriptorIndex);
    l_gpuData.SetRoughnessTextureSRVDescriptorIndex(l_roughnessTextureSRVDescriptorIndex);

    a_table.WriteElement(l_gpuData, a_tableElementIndex);
}
```

> `k_litTextureCount = 3ULL` / `k_litValueCount = 2ULL` はヘッダーの private に `static constexpr std::uint64_t` で足す。

### Graphics/Resource/Model/Material/Standard/UnLit/ModelStandardUnLitMaterial.h / .cpp(新規・写経)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelStandardUnLitMaterial final : public ModelMaterialBase
    {
    public:

         ModelStandardUnLitMaterial()          = default;
        ~ModelStandardUnLitMaterial() override = default;

        const Struct::ModelRenderTableINFO& FetchREFTableINFO() const override;

    protected:

        void WriteGPUData(GPUElementTable& a_table, const std::uint32_t a_tableElementIndex) const override;

    private:

        FWK_DEFINE_TYPE_INFO(ModelStandardUnLitMaterial, ModelMaterialBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ModelMaterialSharedFactory, FWK::Graphics::ModelStandardUnLitMaterial)
```

```cpp
#include "ModelStandardUnLitMaterial.h"

// 光の当たり方を計算しない(UnLit)標準のマテリアル
// ベースカラーとそのテクスチャだけを使う(基底の値だけで足りるため、追加の値はない)
// エラーマテリアル(マゼンタ)もこの種類で作る
const FWK::Struct::ModelRenderTableINFO& FWK::Graphics::ModelStandardUnLitMaterial::FetchREFTableINFO() const
{
    // このマテリアルがGPUへ送る型(ModelStandardUnLitMaterialGPUData)の、テーブルの情報を返す
    return ModelStandardUnLitMaterialGPUData::GetREFModelRenderTableINFO();
}

void FWK::Graphics::ModelStandardUnLitMaterial::WriteGPUData(GPUElementTable& a_table, const std::uint32_t a_tableElementIndex) const
{
    // UnLitは色とテクスチャの番号だけ(20バイト)
    // Litの値(メタリックなど)は使わないので、送らない
    const auto& l_baseColorRuntimeTexture            = GetREFBaseColorRuntimeTexture    ();
    const auto  l_baseColorTextureSRVDescriptorIndex = FetchVALTextureSRVDescriptorIndex(l_baseColorRuntimeTexture);

    ModelStandardUnLitMaterialGPUData l_gpuData = {};

    l_gpuData.SetBaseColor                         (GetREFBaseColor());
    l_gpuData.SetBaseColorTextureSRVDescriptorIndex(l_baseColorTextureSRVDescriptorIndex);

    a_table.WriteElement(l_gpuData, a_tableElementIndex);
}
```

---

# S4-2 Record・ハンドル・System と、PS がテーブルを読む

## ファイル一覧

### 新規

| ファイル | 内容 |
|---|---|
| `Definition/Concept/Graphics/IsDeferredReleaseRecordConcept.h` | `Concept::IsDeferredReleaseRecordConcept`(GPU のリソースを持ち、遅延解放が要る Record か) |
| `Graphics/Resource/Model/Material/Record/ModelMaterialRecord.h` | AssetStorage に入れる Record(マテリアルの実体を shared_ptr で持つ。.h のみ) |
| `Graphics/Resource/Model/Material/ModelMaterial.h/.cpp` | ハンドル(コピーで参照数 +1、破棄で −1) |
| `Graphics/Resource/Model/Material/ModelMaterialSystem.h/.cpp` | 読み込みと共有、エラーマテリアル |
| `Graphics/Resource/Model/Material/Converter/Json/ModelMaterialSystemJsonConverter.h/.cpp` | AssetStorage の容量 |
| `Shader/Model/Standard/ModelStandardMaterial.hlsli` | `RCModelMaterialTable`(マテリアルを使う PS だけが include) |
| `Shader/Model/Standard/Lit/ModelStandardLitMaterial.hlsli` | Lit のテーブルの要素と読み出し |
| `Shader/Model/Standard/UnLit/ModelStandardUnLitMaterial.hlsli` | UnLit のテーブルの要素と読み出し |

### 変更

| ファイル | 変更 |
|---|---|
| `Graphics/Resource/Record/AssetRecordBase.h` | 純粋仮想関数 `ReserveRelease` を消す(下の「継承の見直し」) |
| `Graphics/Resource/Storage/AssetStorage.h` | `SubtractReferenceCount` を2つにする(遅延解放あり / なし) |
| `TextureRecord.h` / `StaticModelRecord.h` / `SkeletalAnimationModelRecord.h` | `ReserveRelease` の `override` を外す(中身はそのまま) |
| `Graphics/Resource/ResourceContext.h/.cpp` | `ModelMaterialSystem` を持つ(テクスチャの後ろに宣言) |
| `Graphics/Resource/Converter/Json/ResourceContextJsonConverter.h/.cpp` | `"ModelMaterialSystem"` キー |
| `Graphics/GraphicsManager.cpp` | Renderer を作った後に、エラーマテリアルを作る |
| `Shader/Model/Standard/Lit/ModelStandardLit_PS.hlsl` / `UnLit/ModelStandardUnLit_PS.hlsl` | テーブルから値を読む |
| `CONFIG/Graphics/GraphicsCONFIG.json` | `ResourceContext.ModelMaterialSystem.MaterialStorage` |

## コード

### 継承の見直し(AssetRecordBase の ReserveRelease)

**今の問題 :** `AssetRecordBase` が `virtual bool ReserveRelease(...) = 0;`(GPU のリソースを遅延解放する関数)を、すべての派生に強制している。
マテリアルの Record は GPU のリソース(GPUResource・SRV の番号)を持たないので、この関数を書くと「何も遅延解放しない ReserveRelease」になる。
**使わない仮想関数を、意味のない中身で派生に書かせるのは、継承の使い方としておかしい**(「マテリアルの Record は、GPU のリソースを持つ Record の一種」は成り立たない)。

**直し方 :**

1. `AssetRecordBase` から `ReserveRelease` を消す(基底は「ファイルパス・StorageID・参照数」だけを持つ)。
2. GPU のリソースを持つ Record(Texture / StaticModel / SkeletalAnimationModel)は、`ReserveRelease` を**自分の関数として**持つ(`override` を外すだけで、中身はそのまま)。
3. `AssetStorage::SubtractReferenceCount` を2つにし、Record が `ReserveRelease` を持っているかを Concept でコンパイル時に見分けて、どちらを使えるかを決める。
   - 持っている → `SubtractReferenceCount(Record, コマンドキュー, ResourceReleaseContext)`(今までどおり、GPU の使用が終わってから解放)
   - 持っていない → `SubtractReferenceCount(Record)`(参照数が 0 になったら、すぐに外す)
4. 書き忘れると、GPU のリソースを持つ Record がすぐ解放する側で動いてしまう危険がある。規約 18-8 にこの決まりを書く。

### Definition/Concept/Graphics/IsDeferredReleaseRecordConcept.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ResourceReleaseContext;
}

namespace FWK::Concept
{
    template <typename Type>
    concept IsDeferredReleaseRecordConcept = requires(Type& a_record, const UINT64& a_retiredFenceValue, Graphics::ResourceReleaseContext& a_resourceReleaseContext)
    {
        { a_record.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext) } -> std::same_as<bool>;
    };
}
```

> `requires(...) { 式 } -> 型;` は「この式が書けて、結果がこの型になる」ことを確かめる書き方。
> `ReserveRelease` を持たない型(ModelMaterialRecord)では式が書けないので、この Concept は false になる。

### Graphics/Resource/Record/AssetRecordBase.h(変更)

```cpp
        void AddReferenceCount();

        bool SubtractReferenceCount();

        bool IsUnused() const;
```

(`virtual bool ReserveRelease(...) = 0;` の行を消す。基底クラスなので仮想デストラクタはそのまま残す)

### Graphics/Resource/Storage/AssetStorage.h(変更・写経)

今の `SubtractReferenceCount` に `requires` を付け、遅延解放の無い版を足す。

```cpp
        bool SubtractReferenceCount(const std::weak_ptr<RecordType>& a_record, const TypeAlias::DirectCommandQueue& a_directCommandQueue, ResourceReleaseContext& a_resourceReleaseContext)
            requires Concept::IsDeferredReleaseRecordConcept<RecordType>
        {
            // (中身は今までと同じ)
            ...
        }

        bool SubtractReferenceCount(const std::weak_ptr<RecordType>& a_record)
            requires (!Concept::IsDeferredReleaseRecordConcept<RecordType>)
        {
            // GPUのリソースを持たないRecord(マテリアルなど)用
            // GPUが使い終わるのを待つ必要がないため、参照数が0になったらすぐにAssetStorageから外す
            const auto& l_record = a_record.lock();

            FWK_ASSERT_RETURN_VALUE_IF(!l_record,                           "指定されたRecordが見つからないため、参照数の減算に失敗しました。", false);
            FWK_ASSERT_RETURN_VALUE_IF(!l_record->SubtractReferenceCount(), "Recordの参照数の減算に失敗しました。",                           false);

            // まだ使っている人がいるなら何もしない
            if (!l_record->IsUnused()) { return true; }

            const auto& l_filePath = l_record->GetREFFilePath();

            if (const auto& l_storageID = l_record->GetVALStorageID();
                l_storageID != Constant::k_invalidStorageID)
            {
                m_storageIDAllocator.Release(l_storageID);
            }

            // mapから外すと、Recordを持っているのはここのローカル変数(l_record)だけになり、
            // この関数を抜けるとRecordとマテリアルの実体が破棄される(マテリアルのデストラクタでテーブルの番号が返される)
            m_recordMap.erase(l_filePath);

            return true;
        }
```

> `requires` で2つの関数を分けると、Record の種類ごとに「呼べる方」だけが残る。
> 例 : `AssetStorage<ModelMaterialRecord>` で3引数の方を呼ぶとコンパイルエラーになり、間違いにすぐ気づける。

### Graphics/Resource/Model/Material/Record/ModelMaterialRecord.h(新規・.h のみ)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelMaterialRecord final : public AssetRecordBase
    {
    public:

         ModelMaterialRecord()          = default;
        ~ModelMaterialRecord() override = default;

        ModelMaterialRecord(const ModelMaterialRecord&)           = delete;
        ModelMaterialRecord(      ModelMaterialRecord&&) noexcept = default;

        ModelMaterialRecord& operator=(const ModelMaterialRecord&)           = delete;
        ModelMaterialRecord& operator=(      ModelMaterialRecord&&) noexcept = default;

        void SetMaterial(const std::shared_ptr<ModelMaterialBase>& a_set) { m_material = a_set; }

        const auto& GetREFMaterial() const { return m_material; }

    private:

        std::shared_ptr<ModelMaterialBase> m_material = nullptr;
    };
}
```

> `ReserveRelease` は持たない(GPU のリソースを持たないため)。そのため `AssetStorage` は遅延解放の無い `SubtractReferenceCount(Record)` を使う。

### Graphics/Resource/Model/Material/ModelMaterial.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelMaterial final
    {
    public:

         ModelMaterial();
         ModelMaterial(const ModelMaterial&  a_other);
         ModelMaterial(      ModelMaterial&& a_other) noexcept;
        ~ModelMaterial();

        ModelMaterial& operator=(const ModelMaterial&  a_other);
        ModelMaterial& operator=(      ModelMaterial&& a_other) noexcept;

        bool Load(const std::filesystem::path& a_filePath);

        bool IsValid() const;

        std::shared_ptr<ModelMaterialBase> FetchVALMaterial() const;

    private:

        void AddReferenceCount() const;

        void SubtractReferenceCount();

        std::weak_ptr<ModelMaterialRecord> m_materialRecord;

        TypeAlias::StorageID m_storageID;
    };
}
```

### Graphics/Resource/Model/Material/ModelMaterial.cpp(新規・写経)

```cpp
#include "ModelMaterial.h"

// マテリアルのハンドル
// Recordをweak_ptrで指し、ハンドルをコピーすると参照数が1増え、破棄すると1減る(Texture / StaticModelと同じ考え方)
// 参照数が0になると、ModelMaterialSystemがRecordをAssetStorageから外し、マテリアルの実体が破棄される
FWK::Graphics::ModelMaterial::ModelMaterial() :
    m_materialRecord(),

    m_storageID(Constant::k_invalidStorageID)
{}
FWK::Graphics::ModelMaterial::ModelMaterial(const ModelMaterial& a_other) :
    m_materialRecord(a_other.m_materialRecord),

    m_storageID(a_other.m_storageID)
{
    // コピーしたので、同じマテリアルを使う人が1人増える
    AddReferenceCount();
}
FWK::Graphics::ModelMaterial::ModelMaterial(ModelMaterial&& a_other) noexcept :
    m_materialRecord(std::move(a_other.m_materialRecord)),

    m_storageID(a_other.m_storageID)
{
    // ムーブは「持ち主が移るだけ」なので参照数は変えず、移動元を空にする
    a_other.m_storageID = Constant::k_invalidStorageID;

    a_other.m_materialRecord.reset();
}
FWK::Graphics::ModelMaterial::~ModelMaterial()
{
    SubtractReferenceCount();
}

FWK::Graphics::ModelMaterial& FWK::Graphics::ModelMaterial::operator=(const ModelMaterial& a_other)
{
    if (this == &a_other) { return *this; }

    // 今持っているマテリアルの参照数を減らしてから、コピー元と同じマテリアルを指す
    SubtractReferenceCount();

    m_storageID      = a_other.m_storageID;
    m_materialRecord = a_other.m_materialRecord;

    AddReferenceCount();

    return *this;
}
FWK::Graphics::ModelMaterial& FWK::Graphics::ModelMaterial::operator=(ModelMaterial&& a_other) noexcept
{
    if (this == &a_other) { return *this; }

    SubtractReferenceCount();

    m_storageID      = a_other.m_storageID;
    m_materialRecord = std::move(a_other.m_materialRecord);

    a_other.m_storageID = Constant::k_invalidStorageID;

    a_other.m_materialRecord.reset();

    return *this;
}

bool FWK::Graphics::ModelMaterial::Load(const std::filesystem::path& a_filePath)
{
    // 既に別のマテリアルを持っていれば、先に参照を外す
    SubtractReferenceCount();

    auto& l_graphicsManager     = GraphicsManager::GetInstance                      ();
    auto& l_resourceContext     = l_graphicsManager.GetMutableREFResourceContext    ();
    auto& l_modelMaterialSystem = l_resourceContext.GetMutableREFModelMaterialSystem();

    // 読み込み済みなら同じRecord(参照数+1)、まだなら新しく読み込んだRecordが返る
    const auto& l_materialRecord = l_modelMaterialSystem.LoadModelMaterial(a_filePath).lock();

    // 読み込めなかった(ファイルが無い・種類が分からないなど)ときは、空のハンドルのまま
    // 使う側はFetchVALMaterialがnullptrを返すので、エラーマテリアルに切り替える
    if (!l_materialRecord) { return false; }

    m_storageID      = l_materialRecord->GetVALStorageID();
    m_materialRecord = l_materialRecord;

    return true;
}

bool FWK::Graphics::ModelMaterial::IsValid() const
{
    if (m_storageID == Constant::k_invalidStorageID ||
        m_materialRecord.expired())
    {
        return false;
    }

    return true;
}

std::shared_ptr<FWK::Graphics::ModelMaterialBase> FWK::Graphics::ModelMaterial::FetchVALMaterial() const
{
    // Recordが持つマテリアルの実体を返す
    // 読み込みに失敗している・解放済みならnullptr(使う側はエラーマテリアルに切り替える)
    const auto& l_materialRecord = m_materialRecord.lock();

    if (!l_materialRecord) { return nullptr; }

    return l_materialRecord->GetREFMaterial();
}

void FWK::Graphics::ModelMaterial::AddReferenceCount() const
{
    if (m_storageID == Constant::k_invalidStorageID) { return; }

    auto& l_graphicsManager     = GraphicsManager::GetInstance                      ();
    auto& l_resourceContext     = l_graphicsManager.GetMutableREFResourceContext    ();
    auto& l_modelMaterialSystem = l_resourceContext.GetMutableREFModelMaterialSystem();

    FWK_ASSERT_RETURN_IF(!l_modelMaterialSystem.AddModelMaterialReferenceCount(m_materialRecord), "マテリアルの参照数の加算に失敗しました。");
}

void FWK::Graphics::ModelMaterial::SubtractReferenceCount()
{
    if (m_storageID == Constant::k_invalidStorageID)
    {
        m_materialRecord.reset();

        return;
    }

    auto& l_graphicsManager     = GraphicsManager::GetInstance                      ();
    auto& l_resourceContext     = l_graphicsManager.GetMutableREFResourceContext    ();
    auto& l_modelMaterialSystem = l_resourceContext.GetMutableREFModelMaterialSystem();

    // マテリアルはGPUのリソースを持たないので、コマンドキューやResourceReleaseContextは要らない
    FWK_ASSERT_RETURN_IF(!l_modelMaterialSystem.SubtractModelMaterialReferenceCount(m_materialRecord), "マテリアルの参照数の減算に失敗しました。");

    m_storageID = Constant::k_invalidStorageID;

    m_materialRecord.reset();
}
```

### Graphics/Resource/Model/Material/ModelMaterialSystem.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelMaterialSystem final
    {
    public:

         ModelMaterialSystem() = default;
        ~ModelMaterialSystem() = default;

        void Deserialize(const nlohmann::json& a_rootJson);
        bool Create     ();

        nlohmann::json Serialize() const;

        bool CreateErrorMaterial();

        std::weak_ptr<ModelMaterialRecord> LoadModelMaterial(const std::filesystem::path& a_filePath);

        bool AddModelMaterialReferenceCount     (const std::weak_ptr<ModelMaterialRecord>& a_materialRecord);
        bool SubtractModelMaterialReferenceCount(const std::weak_ptr<ModelMaterialRecord>& a_materialRecord);

        const auto& GetREFErrorMaterial() const { return m_errorMaterial; }

        const auto& GetREFMaterialStorage() const { return m_materialStorage; }

        auto& GetMutableREFMaterialStorage() { return m_materialStorage; }

    private:

        std::shared_ptr<ModelMaterialBase> CreateModelMaterial(const std::filesystem::path& a_filePath);

        std::shared_ptr<ModelMaterialBase> m_errorMaterial = nullptr;

        AssetStorage<ModelMaterialRecord> m_materialStorage = {};

        Converter::ModelMaterialSystemJsonConverter m_jsonConverter   = {};
        Converter::ModelMaterialBinaryConverter     m_binaryConverter = {};
    };
}
```

### Graphics/Resource/Model/Material/ModelMaterialSystem.cpp(新規・写経)

```cpp
#include "ModelMaterialSystem.h"

// .matファイルから作ったマテリアルを、テクスチャと同じハンドル方式で管理するクラス
// 同じ.matは1度だけ読み込み、AssetStorageに登録したRecord(ModelMaterialRecord)を参照数で共有する
// 例 : 3体のキャラクターが同じ"StandardLit_Body.mat"を使うなら、実体は1つで参照数は3になる
// 描画用の値は、マテリアル自身が種類ごとのテーブル(Rendererが持つ)へ書く
void FWK::Graphics::ModelMaterialSystem::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}
bool FWK::Graphics::ModelMaterialSystem::Create()
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_materialStorage.Create(), "AssetStorageの作成に失敗したため、ModelMaterialSystemの作成に失敗しました。", false);

    return true;
}

nlohmann::json FWK::Graphics::ModelMaterialSystem::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

bool FWK::Graphics::ModelMaterialSystem::CreateErrorMaterial()
{
    // .matが割り当てられていない、または読み込めなかったメッシュを描くためのマテリアル
    // 目立つ色(マゼンタ)のUnLitにして、設定漏れや読み込みの失敗にすぐ気付けるようにする
    // ※ 注意 : テーブルはRendererが持つため、Rendererを作った後(GraphicsManager::PostLoadCONFIGの最後)に呼ぶ
    auto l_errorMaterial = std::make_shared<ModelStandardUnLitMaterial>();

    l_errorMaterial->SetBaseColor       (Constant::k_errorModelMaterialBaseColor);
    l_errorMaterial->LoadRuntimeTextures();

    FWK_ASSERT_RETURN_VALUE_IF(!l_errorMaterial->CreateGPUData(), "エラーマテリアルのGPUデータの作成に失敗しました。", false);

    m_errorMaterial = std::move(l_errorMaterial);

    return true;
}

std::weak_ptr<FWK::Graphics::ModelMaterialRecord> FWK::Graphics::ModelMaterialSystem::LoadModelMaterial(const std::filesystem::path& a_filePath)
{
    const auto& l_filePath = a_filePath.wstring();

    // 既に読み込み済みなら、参照数を1つ増やして同じRecordを返す
    if (const auto& l_materialRecord = m_materialStorage.FindVALRecord(l_filePath).lock();
        l_materialRecord)
    {
        FWK_ASSERT_RETURN_VALUE_IF(!AddModelMaterialReferenceCount(l_materialRecord), "読み込み済みのマテリアルの参照数の加算に失敗したため、マテリアルの読み込みに失敗しました。", {});

        return l_materialRecord;
    }

    // .matに保存されたマテリアルの種類の名前から正しい派生クラスを作り、値を読み込ませる
    const auto& l_material = CreateModelMaterial(a_filePath);

    if (!l_material) { return {}; }

    const auto& l_allocatedStorageID = m_materialStorage.AllocateStorageID();

    FWK_ASSERT_RETURN_VALUE_IF(l_allocatedStorageID == Constant::k_invalidStorageID, "StorageIDの割り当てに失敗したため、マテリアルの読み込みに失敗しました。", {});

    auto l_materialRecord = std::make_shared<ModelMaterialRecord>();

    // 参照数は、このマテリアルを読み込んだ1人分の1から始める
    l_materialRecord->SetFilePath      (l_filePath);
    l_materialRecord->SetStorageID     (l_allocatedStorageID);
    l_materialRecord->SetReferenceCount(AssetRecordBase::k_initialAssetReferenceCount);
    l_materialRecord->SetMaterial      (l_material);

    // テクスチャやモデルと違い、GPUへのコピーを待つ必要がないため、すぐにAssetStorageへ登録する
    if (!m_materialStorage.RegisterRecord(l_materialRecord, l_filePath))
    {
        // 登録できなかったStorageIDは返却する(返却しないと使用中のまま残り、いずれ割り当てられなくなる)
        m_materialStorage.ReleaseStorageID(l_allocatedStorageID);

        FWK_ASSERT_RETURN_VALUE("マテリアルのRecordの登録に失敗したため、マテリアルの読み込みに失敗しました。", {});
    }

    return l_materialRecord;
}

bool FWK::Graphics::ModelMaterialSystem::AddModelMaterialReferenceCount(const std::weak_ptr<ModelMaterialRecord>& a_materialRecord)
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_materialStorage.AddReferenceCount(a_materialRecord), "AssetStorageでの参照数の加算に失敗したため、マテリアルの参照数の加算に失敗しました。", false);

    return true;
}
bool FWK::Graphics::ModelMaterialSystem::SubtractModelMaterialReferenceCount(const std::weak_ptr<ModelMaterialRecord>& a_materialRecord)
{
    // マテリアルのRecordはGPUのリソースを持たないため、遅延解放の無い方のSubtractReferenceCountが使われる
    // 参照数が0になったら、すぐにAssetStorageから外れ、マテリアルの実体が破棄される
    FWK_ASSERT_RETURN_VALUE_IF(!m_materialStorage.SubtractReferenceCount(a_materialRecord), "AssetStorageでの参照数の減算に失敗したため、マテリアルの参照数の減算に失敗しました。", false);

    return true;
}

std::shared_ptr<FWK::Graphics::ModelMaterialBase> FWK::Graphics::ModelMaterialSystem::CreateModelMaterial(const std::filesystem::path& a_filePath)
{
    std::string l_typeName = {};

    // .matの先頭から、保存したときのマテリアルの種類の名前(クラス名)だけを読む
    if (!m_binaryConverter.TryReadTypeName(a_filePath, l_typeName))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "マテリアルファイルの種類の名前を読み込めませんでした。\nFilePath : {}", a_filePath.string());

        return nullptr;
    }

    // 種類の名前で、ファクトリーから派生クラスを作る
    // FWK_REGISTER_FACTORY_METHODで登録したクラス(StandardLit / StandardUnLit)だけを作れる
    const auto& l_modelMaterialFactory = TypeAlias::ModelMaterialSharedFactory::GetInstance();
    const auto& l_material             = l_modelMaterialFactory.Create                     (l_typeName);

    if (!l_material)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "ファクトリーに登録されていない種類のマテリアルのため、マテリアルを作れませんでした。\nTypeName : {}", l_typeName);

        return nullptr;
    }

    // 作ったマテリアル自身に、.matの値を「基底の値 → 派生の値」の順に読み込ませる(テクスチャの読み込みも含む)
    if (!l_material->Load(a_filePath))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "マテリアルファイルの値を読み込めませんでした。\nFilePath : {}", a_filePath.string());

        return nullptr;
    }

    // 読み込んだ値を、マテリアルの種類に合ったテーブルへ置く(GPUへは次のフレームの最初にコピーされる)
    if (!l_material->CreateGPUData())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "マテリアルのGPUデータを作れませんでした。\nFilePath : {}", a_filePath.string());

        return nullptr;
    }

    return l_material;
}
```

### ModelMaterialSystemJsonConverter(新規)

`StaticModelSystemJsonConverter` と同じ形(キーは `"MaterialStorage"`、中身は `AssetStorage` の Deserialize / Serialize に任せる)。

### ResourceContext(変更)

```cpp
        const auto& GetREFModelMaterialSystem() const { return m_modelMaterialSystem; }

        auto& GetMutableREFModelMaterialSystem() { return m_modelMaterialSystem; }
```

```cpp
        TextureSystem                m_textureSystem                = {};
        StaticModelSystem            m_staticModelSystem            = {};
        SkeletalAnimationModelSystem m_skeletalAnimationModelSystem = {};
        ModelMaterialSystem          m_modelMaterialSystem          = {};
```

`PostDeserialize` で `m_modelMaterialSystem.Create()`。JsonConverter に `"ModelMaterialSystem"`。

> マテリアルは Texture のハンドルを持つので、`m_textureSystem` より後に宣言する(先に破棄される)。
> テーブルの番号は weak_ptr で返すので、Renderer(テーブル)が先に破棄されていても問題ない。

### Graphics/GraphicsManager.cpp(変更・写経)

`PostLoadCONFIG` の `m_renderer.PostDeserialize(...)` の後、`ProcessPendingTextureUploads` の前に追加:

```cpp
    // エラーマテリアル(マゼンタ)を作る
    // マテリアルはRendererが持つテーブルへ値を書くため、Rendererを作った後に行う
    auto& l_modelMaterialSystem = m_resourceContext.GetMutableREFModelMaterialSystem();

    FWK_ASSERT_RETURN_VALUE_IF(!l_modelMaterialSystem.CreateErrorMaterial(), "エラーマテリアルの作成に失敗しました。", false);
```

### Shader/Model/Standard/ModelStandardMaterial.hlsli(新規・写経)

```hlsl
#ifndef MODEL_STANDARD_MATERIAL_HLSLI
#define MODEL_STANDARD_MATERIAL_HLSLI

// マテリアルのテーブルのSRVの番号
// Lit / UnLitのパスが、パスの最初に自分の種類のテーブルの番号を1回だけ送る
// 影のパスは送らないため、影のシェーダーはこのファイルをincludeしない(規約 19-8)
// C++側のStruct::RCModelMaterialTableと同じ並びにする
cbuffer RCModelMaterialTable : register(b6)
{
    uint g_materialTableSRVDescriptorIndex;
};

#endif // MODEL_STANDARD_MATERIAL_HLSLI
```

### Shader/Model/Standard/Lit/ModelStandardLitMaterial.hlsli(新規・写経)

```hlsl
#ifndef MODEL_STANDARD_LIT_MATERIAL_HLSLI
#define MODEL_STANDARD_LIT_MATERIAL_HLSLI
#include "../ModelStandardMaterial.hlsli"

// StandardLitのテーブルの1要素
// C++側のGraphics::ModelStandardLitMaterialGPUDataと同じ並び(40バイト)にする
struct ModelStandardLitMaterialData
{
    float4 baseColor;
    float  metallic;
    float  roughness;
    uint   baseColorTextureSRVDescriptorIndex;
    uint   normalTextureSRVDescriptorIndex;
    uint   metallicTextureSRVDescriptorIndex;
    uint   roughnessTextureSRVDescriptorIndex;
};

// この描画のマテリアルの値を、StandardLitのテーブルから読む
// g_materialIndexはModel.hlsliのRCModelDrawItem(描画ごとのルート定数)
ModelStandardLitMaterialData FetchModelStandardLitMaterialData()
{
    StructuredBuffer<ModelStandardLitMaterialData> l_materialTable = ResourceDescriptorHeap[g_materialTableSRVDescriptorIndex];

    return l_materialTable[g_materialIndex];
}

#endif // MODEL_STANDARD_LIT_MATERIAL_HLSLI
```

### Shader/Model/Standard/UnLit/ModelStandardUnLitMaterial.hlsli(新規・写経)

```hlsl
#ifndef MODEL_STANDARD_UNLIT_MATERIAL_HLSLI
#define MODEL_STANDARD_UNLIT_MATERIAL_HLSLI
#include "../ModelStandardMaterial.hlsli"

// StandardUnLitのテーブルの1要素
// C++側のGraphics::ModelStandardUnLitMaterialGPUDataと同じ並び(20バイト)にする
struct ModelStandardUnLitMaterialData
{
    float4 baseColor;
    uint   baseColorTextureSRVDescriptorIndex;
};

ModelStandardUnLitMaterialData FetchModelStandardUnLitMaterialData()
{
    StructuredBuffer<ModelStandardUnLitMaterialData> l_materialTable = ResourceDescriptorHeap[g_materialTableSRVDescriptorIndex];

    return l_materialTable[g_materialIndex];
}

#endif // MODEL_STANDARD_UNLIT_MATERIAL_HLSLI
```

### Shader/Model/Standard/UnLit/ModelStandardUnLit_PS.hlsl(変更・写経)

```hlsl
#include "../ModelStandard.hlsli"
#include "ModelStandardUnLit.hlsli"
#include "ModelStandardUnLitMaterial.hlsli"

float4 main(const MSOutputUnLit a_input) : SV_Target0
{
    // この描画のマテリアルの値を、テーブルから読む
    const ModelStandardUnLitMaterialData l_material = FetchModelStandardUnLitMaterialData();

    return FetchModelBaseColor(l_material.baseColorTextureSRVDescriptorIndex, l_material.baseColor, a_input.uv);
}
```

### Shader/Model/Standard/Lit/ModelStandardLit_PS.hlsl(変更・写経)

変える所は4つ(PBR の計算そのものは今のまま)。

1. include に `#include "ModelStandardLitMaterial.hlsli"` を足す。
2. `main` の先頭で `const ModelStandardLitMaterialData l_material = FetchModelStandardLitMaterialData();` を読む。
3. テクスチャを `g_xxxTextureSRVDescriptorIndex` ではなく `l_material.xxxTextureSRVDescriptorIndex` で取り出し、
   `g_metallicFactor` / `g_roughnessFactor` を `l_material.metallic` / `l_material.roughness` にする。
4. `FetchWorldNormal(a_input)` を `FetchWorldNormal(a_input, l_material.normalTextureSRVDescriptorIndex)` にし、関数の中の
   `g_normalTextureSRVDescriptorIndex` を引数に置き換える。

```hlsl
float4 main(const MSOutputLit a_input) : SV_Target0
{
    // この描画のマテリアルの値を、StandardLitのテーブルから読む
    const ModelStandardLitMaterialData l_material = FetchModelStandardLitMaterialData();

    Texture2D<float4> l_metallicTexture  = ResourceDescriptorHeap[l_material.metallicTextureSRVDescriptorIndex];
    Texture2D<float4> l_roughnessTexture = ResourceDescriptorHeap[l_material.roughnessTextureSRVDescriptorIndex];

    // BaseColor、非金属ではDiffuse色として使う
    // 金属ではSpecular反射色として使う
    const float4 l_baseColor = FetchModelBaseColor(l_material.baseColorTextureSRVDescriptorIndex, l_material.baseColor, a_input.uv);

    // Metallic
    // 0.0Fに近いほど非金属、1.0Fに近いほど金属
    const float l_metallic = saturate(l_metallicTexture.Sample(g_textureSampler, a_input.uv).r * l_material.metallic);

    // Roughness
    // 低いほど鏡面反射が鋭く、高いほどぼやける
    // 0に近すぎるとSpecularが不安定になりやすいので最低値を持たせる
    const float l_roughness = max(saturate(l_roughnessTexture.Sample(g_textureSampler, a_input.uv).r * l_material.roughness), k_minRoughnessForSpecularStability);

    // NormalMap込みのWorld空間法線
    const float3 l_normal = FetchWorldNormal(a_input, l_material.normalTextureSRVDescriptorIndex);

    // (ここから下は今までと同じ)
    ...
}
```

### CONFIG/Graphics/GraphicsCONFIG.json(変更)

`ResourceContext` に足す。

```json
        "ModelMaterialSystem": {
            "MaterialStorage": {
                "StorageIDAllocator": {
                    "StorageIDAllocatorCapacity": 10000
                }
            }
        },
```

> 中の形は、今の GraphicsCONFIG.json の `StaticModelSystem.ModelStorage` と同じ(キーは `StorageIDAllocatorCapacity`、確認済み)。

---

# S4-3 サブメッシュ名と、取り込み時の .mat の自動生成

## 目的

- メッシュに「サブメッシュ名」(FBX のマテリアル名。無ければ `Default`)を持たせる。取り込み時に作る .mat のファイル名に使う(後で S0 のスロットも、この名前で .mat と対応づける)。
- FBX を初めて読み込んだとき(キャッシュを作るとき)、メッシュごとに `StandardLit_<サブメッシュ名>.mat` をモデルと同じフォルダに作る。
  - 既にあれば上書きしない(ユーザーが編集した値を守るため)。
  - 値は FBX のマテリアル(ベースカラー・メタリック・ラフネス・テクスチャ)から取る。テクスチャは Registry に登録して UUID を書く。
- モデルのキャッシュの拡張子を、Static は `.staticModel`、Skeletal は `.skeletalModel` に分ける(今は両方 `.asset` で、同じ FBX を Static と Skeletal の両方で読むと、読むたびに作り直しになる)。

## ファイル一覧

### 新規

| ファイル | 内容 |
|---|---|
| `Graphics/Resource/Model/Material/File/ModelMaterialFileCreator.h/.cpp` | 取り込み時に .mat を作る |


### 変更

| ファイル | 変更 |
|---|---|
| `Definition/Struct/Graphics/StaticModelRecordStruct.h` / `SkeletalAnimationModelRecordStruct.h` | メッシュに `std::wstring m_subMeshName` |
| `Definition/Struct/Graphics/ModelBinaryConverterBaseStruct.h` | `ModelMeshBinaryHeader::m_subMeshNameSize` |
| `Definition/Constant/Graphics/ModelBinaryConverterConstant.h`(既存) | `.staticModel` / `.skeletalModel` と `k_emptySubMeshNameSize` を足す |
| `Graphics/Resource/Model/Converter/Binary/ModelBinaryConverterBase.h/.cpp` | サブメッシュ名の読み書き、`CanLoad` にキャッシュのパスを渡す |
| `Static|Skeletal/Converter/Binary/*BinaryConverter.h/.cpp` | バージョン +1、自分の拡張子でキャッシュのパスを作る |
| `Graphics/Resource/Model/FBXLoader/FBXModelLoaderBase.h/.cpp` | `FetchVALSubMeshName(ufbx_material*)` |
| `Static|Skeletal/FBXLoader/*FBXLoader.cpp` | メッシュにサブメッシュ名を入れる |
| `Static|Skeletal/*ModelSystem.h/.cpp` | `BuildXxxAssetData` の最後で `ModelMaterialFileCreator` を呼ぶ |

## コード

### Definition/Struct/Graphics/StaticModelRecordStruct.h(変更)

```cpp
        std::vector<StaticModelVertex> m_vertexList = {};
        std::vector<std::uint32_t>     m_indexList  = {};

        std::wstring m_subMeshName = {};

        Struct::ModelMaterial m_material = {};
```

(Skeletal も同じ位置に足す)

### Definition/Struct/Graphics/ModelBinaryConverterBaseStruct.h(変更)

```cpp
        std::uint64_t m_vertexCount = Constant::k_emptyModelVertexCount;
        std::uint64_t m_indexCount  = Constant::k_emptyModelIndexCount;

        std::uint64_t m_subMeshNameSize = Constant::k_emptySubMeshNameSize;
```

> 意味が違う定数(`k_emptyTextureFilePathSize`)は流用せず、既存の `ModelBinaryConverterConstant.h` に `k_emptySubMeshNameSize` を足す。

### ModelBinaryConverterBase.h(変更・写経)

- `CreateModelMeshBinaryHeader` に `l_modelMeshBinaryHeader.m_subMeshNameSize = CalculateWStringBinaryFileSize(a_modelMesh.m_subMeshName);`
- `TryReadModelMeshBinaryDataCommon` の Index 配列の後に `if (!TryReadWStringBinaryData(l_modelMeshBinaryHeader.m_subMeshNameSize, a_modelMesh.m_subMeshName, a_memoryReadOffset)) { return false; }`
- `WriteModelMeshBinaryDataCommon` の Index 配列の後に `WriteWStringBinaryData(a_modelMesh.m_subMeshName, a_memoryWriteOffset);`
- `CalculateModelMeshBinaryFileSizeCommon` に `l_modelMeshBinaryFileSize += CalculateWStringBinaryFileSize(a_modelMesh.m_subMeshName);`
- `CanLoad(const std::filesystem::path& a_filePath)` を `CanLoad(const std::filesystem::path& a_filePath, const std::filesystem::path& a_modelAssetFilePath)` にする(拡張子の確認は、キャッシュのパスの拡張子で行う)。

```cpp
bool FWK::Converter::ModelBinaryConverterBase::CanLoad(const std::filesystem::path& a_filePath, const std::filesystem::path& a_modelAssetFilePath) const
{
    // 元となるFBXが存在しない場合は、キャッシュの正当性を判断できないので読み込まない
    if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerFBXExtension)) { return false; }

    // キャッシュが存在しないなら、FBXから読み込んで生成する
    // 拡張子はStatic(.staticModel) / Skeletal(.skeletalModel)で違うため、渡されたパスの拡張子で確かめる
    if (!Utility::CanLoadFilePath(a_modelAssetFilePath, a_modelAssetFilePath.extension())) { return false; }

    // FBXがキャッシュより新しいなら、古いキャッシュは使わない
    if (IsUpdatedSourceFile(a_filePath, a_modelAssetFilePath)) { return false; }

    return true;
}
```

### Static / Skeletal の BinaryConverter(変更)

- `k_modelAssetVersion` を 1 つ上げる(Static 2 → 3、Skeletal 2 → 3)。
- `CreateAssetFilePath(a_filePath)` を呼んでいた所(Load / Save の2か所)を、
  `Utility::CreateFilePathByReplaceExtension(a_filePath, Constant::k_lowerStaticModelExtension)`(Skeletal は `k_lowerSkeletalModelExtension`)にする。
- `CanLoad(a_filePath)` を `CanLoad(a_filePath, l_modelAssetFilePath)` にする。

```cpp
// Definition/Constant/Graphics/ModelBinaryConverterConstant.h(既存のファイルに足す)
namespace FWK::Constant
{
    inline const std::filesystem::path k_lowerStaticModelExtension   = ".staticModel";
    inline const std::filesystem::path k_lowerSkeletalModelExtension = ".skeletalModel";

    inline constexpr std::uint64_t k_emptyModelVertexCount = 0ULL;
    inline constexpr std::uint64_t k_emptyModelIndexCount  = 0ULL;
    inline constexpr std::uint64_t k_emptySubMeshNameSize  = 0ULL;

    ...(既存の定数はそのまま)
}
```

> 古い `.asset` はもう読まない(初回だけ FBX から作り直す)。不要になった `.asset` は手で消してよい。

### FBXModelLoaderBase(変更・写経)

`.h`(protected の `ExtractModelMaterial` の下):

```cpp
        std::wstring FetchVALSubMeshName(const ufbx_material* a_fbxMaterial) const;
```

`.cpp`:

```cpp
std::wstring FWK::Graphics::FBXModelLoaderBase::FetchVALSubMeshName(const ufbx_material* a_fbxMaterial) const
{
    // サブメッシュ名は、FBXのマテリアル名にする
    // 取り込み時に作る.matのファイル名(StandardLit_<サブメッシュ名>.mat)に使う
    // 番号ではなく名前なので、FBXを作り直してメッシュの順番が変わっても、同じ.matを指せる
    // マテリアルが無い・名前が空なら"Default"
    if (!a_fbxMaterial ||
        a_fbxMaterial->name.length == k_emptyFBXNameLength)
    {
        return std::wstring{ Constant::k_modelMaterialDefaultSubMeshName };
    }

    // ufbxの文字列はUTF-8(dataとlength)なので、std::stringにしてからstd::wstringへ変換する
    const auto& l_materialName = std::string{ a_fbxMaterial->name.data, a_fbxMaterial->name.length };

    return Utility::StringToWString(l_materialName);
}
```

> `k_emptyFBXNameLength` は `static constexpr std::size_t k_emptyFBXNameLength = 0ULL;` を private に足す。
> `Utility::StringToWString` は `MultiByteToWideChar(CP_UTF8, ...)` で変換している(UTF-8 前提、確認済み)。

### StaticModelFBXLoader.cpp / SkeletalAnimationModelFBXLoader.cpp(変更)

`ExtractModelMeshList` の中で、メッシュを作る2か所に1行ずつ足す。

```cpp
        // マテリアルが無いメッシュ(materials.count == 0)の場合
        l_modelMesh.m_subMeshName = FetchVALSubMeshName(nullptr);
```

```cpp
        const auto* l_fbxMaterial = l_fbxMesh->materials.data[l_materialIndex];

        ExtractModelMaterial(l_fbxMaterial, l_modelMesh.m_material.m_assetData);

        l_modelMesh.m_subMeshName = FetchVALSubMeshName(l_fbxMaterial);
```

> 1つの FBX の中に同じ名前のマテリアルが複数のノードで使われている場合、同じサブメッシュ名のメッシュが複数できる。
> スロットは名前で対応づけるので、**同じ名前のメッシュは同じ .mat を使う**(Unity と同じ考え方で、たいていはそれが望ましい)。

### Graphics/Resource/Model/Material/File/ModelMaterialFileCreator.h(新規)

```cpp
#pragma once

namespace FWK::Graphics
{
    class ModelMaterialFileCreator final
    {
    public:

         ModelMaterialFileCreator() = default;
        ~ModelMaterialFileCreator() = default;

        template <Concept::IsDerivedAssetRecordBaseConcept ModelRecordType>
        void CreateDefaultModelMaterialFileList(const std::filesystem::path& a_modelFilePath, const ModelRecordType& a_modelRecord)
        {
            // モデルのメッシュごとに、既定のマテリアルファイル(StandardLit_<サブメッシュ名>.mat)を作る
            // FBXを初めて取り込んだとき(キャッシュを作るとき)だけ呼ばれる
            // 同じサブメッシュ名のメッシュが複数あっても、ファイルは1つだけ作る(2つ目は「既にある」ので作らない)
            const auto& l_modelData = a_modelRecord.GetREFModelData();

            for (const auto& l_modelMesh : l_modelData.m_meshList)
            {
                CreateDefaultModelMaterialFile(a_modelFilePath, l_modelMesh.m_subMeshName, l_modelMesh.m_material.m_assetData);
            }
        }

    private:

        void CreateDefaultModelMaterialFile(const std::filesystem::path& a_modelFilePath, const std::wstring& a_subMeshName, const Struct::ModelMaterialAssetData& a_materialAssetData);

        AssetFilePath CreateTextureAssetFilePath(const std::filesystem::path& a_modelFilePath, const std::wstring& a_textureFilePath) const;

        static std::filesystem::path CreateDefaultModelMaterialFilePath(const std::filesystem::path& a_modelFilePath, const std::wstring& a_subMeshName);

        static std::wstring ReplaceInvalidFileNameCharacter(const std::wstring& a_name);

        static constexpr std::wstring_view k_invalidFileNameCharacterList = L"\\/:*?\"<>|";

        static constexpr wchar_t k_replaceFileNameCharacter = L'_';
    };
}
```

> - `CreateDefaultModelMaterialFilePath` は、このクラスの中でしか使わないので private の static にした(2026-10-11)。
>   S0 で ModelComponent が「既定の .mat のパス」を探すようになったら、そのときに public へ移す。
> - .cpp の関数の順番も、ヘッダーの宣言の順番に合わせた(規約 8-2)。

### Graphics/Resource/Model/Material/File/ModelMaterialFileCreator.cpp(新規・写経)

```cpp
#include "ModelMaterialFileCreator.h"
#include "../../../../../../Application/Application.h"

// FBXを初めて取り込んだときに、メッシュごとの既定のマテリアルファイル(.mat)を作るクラス
// 作るのは全部StandardLit。値はFBXのマテリアル(色・メタリック・ラフネス・テクスチャ)から取る
// 既にファイルがあれば作らない(ユーザーが編集した値を上書きしないため)
void FWK::Graphics::ModelMaterialFileCreator::CreateDefaultModelMaterialFile(const std::filesystem::path& a_modelFilePath, const std::wstring& a_subMeshName, const Struct::ModelMaterialAssetData& a_materialAssetData)
{
    const auto& l_materialFilePath = CreateDefaultModelMaterialFilePath(a_modelFilePath, a_subMeshName);

    // 既にあれば作らない
    if (std::filesystem::exists(l_materialFilePath)) { return; }

    ModelStandardLitMaterial l_material = {};

    l_material.SetBaseColor       (a_materialAssetData.m_baseColorFactor);
    l_material.SetMetallic        (a_materialAssetData.m_metallicFactor);
    l_material.SetRoughness       (a_materialAssetData.m_roughnessFactor);
    l_material.SetBaseColorTexture(CreateTextureAssetFilePath(a_modelFilePath, a_materialAssetData.m_baseColorTextureFilePath));
    l_material.SetNormalTexture   (CreateTextureAssetFilePath(a_modelFilePath, a_materialAssetData.m_normalTextureFilePath));
    l_material.SetMetallicTexture (CreateTextureAssetFilePath(a_modelFilePath, a_materialAssetData.m_metallicTextureFilePath));
    l_material.SetRoughnessTexture(CreateTextureAssetFilePath(a_modelFilePath, a_materialAssetData.m_roughnessTextureFilePath));

    if (!l_material.Save(l_materialFilePath))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "既定のマテリアルファイルを作れませんでした。\nFilePath : {}", l_materialFilePath.string());

        return;
    }

    // 作った.matをRegistryへ登録する
    // .matはUUIDで指される(アセットブラウザでの移動・名前変更をしても、UUIDは変わらない)
    auto& l_application           = Application::GetInstance                        ();
    auto& l_assetFilePathRegistry = l_application.GetMutableREFAssetFilePathRegistry();
    auto& l_uuidManager           = Utility::UUIDManager::GetInstance               ();

    l_assetFilePathRegistry.Add(l_materialFilePath, l_uuidManager.GenerateVALUUID(), Enum::AssetFilePathType::ModelMaterial);
}

FWK::AssetFilePath FWK::Graphics::ModelMaterialFileCreator::CreateTextureAssetFilePath(const std::filesystem::path& a_modelFilePath, const std::wstring& a_textureFilePath) const
{
    AssetFilePath l_assetFilePath = {};

    l_assetFilePath.SetAllowedType(Enum::AssetFilePathType::Texture);

    // FBXにテクスチャが無ければ、空のまま(既定のテクスチャで描かれる)
    if (a_textureFilePath.empty()) { return l_assetFilePath; }

    // FBXのテクスチャのパスは、FBXのフォルダからの相対パスのことが多い
    // 例 : "Texture/Body.png" → "Asset/Model/Chara/Texture/Body.png"
    std::filesystem::path l_textureFilePath = a_textureFilePath;

    if (l_textureFilePath.is_relative())
    {
        l_textureFilePath = a_modelFilePath.parent_path() / l_textureFilePath;
    }

    // PNG以外(FBXが別の場所のファイルを指しているなど)は、Registryへ登録できないので空のままにする
    if (!Utility::CanLoadFilePath(l_textureFilePath, Constant::k_lowerPNGExtension)) { return l_assetFilePath; }

    auto& l_application           = Application::GetInstance                        ();
    auto& l_assetFilePathRegistry = l_application.GetMutableREFAssetFilePathRegistry();

    // 既に登録されていればそのUUID、無ければ新しく登録する
    if (const auto* l_assetUUID = l_assetFilePathRegistry.FindPTRAssetUUID(l_textureFilePath);
        l_assetUUID)
    {
        l_assetFilePath.SetAssetFilePathUUID(*l_assetUUID);

        return l_assetFilePath;
    }

          auto& l_uuidManager = Utility::UUIDManager::GetInstance();
    const auto& l_assetUUID   = l_uuidManager.GenerateVALUUID    ();

    l_assetFilePathRegistry.Add(l_textureFilePath, l_assetUUID, Enum::AssetFilePathType::Texture);

    l_assetFilePath.SetAssetFilePathUUID(l_assetUUID);

    return l_assetFilePath;
}

std::filesystem::path FWK::Graphics::ModelMaterialFileCreator::CreateDefaultModelMaterialFilePath(const std::filesystem::path& a_modelFilePath, const std::wstring& a_subMeshName)
{
    // 例 : Asset/Model/Chara/Chara.fbx のサブメッシュ"Body" → Asset/Model/Chara/StandardLit_Body.mat
    // ファイル名に使えない文字(\ / : * ? " < > |)は「_」に置き換える(FBXのマテリアル名には入っていることがある)
    // .matのパスはこの関数だけで作るので、作るときと探すときで置き換え方がずれない
    auto l_fileName = std::wstring{ Constant::k_standardLitModelMaterialFilePrefix } + ReplaceInvalidFileNameCharacter(a_subMeshName);

    l_fileName += Constant::k_lowerModelMaterialExtension.wstring();

    return a_modelFilePath.parent_path() / l_fileName;
}

std::wstring FWK::Graphics::ModelMaterialFileCreator::ReplaceInvalidFileNameCharacter(const std::wstring& a_name)
{
    auto l_safeName = a_name;

    // std::ranges::replace_ifは、条件に合う要素を指定した値へ置き換える
    std::ranges::replace_if(l_safeName,
                            [](const wchar_t a_character)
                            {
                                return k_invalidFileNameCharacterList.find(a_character) != std::wstring_view::npos;
                            },
                            k_replaceFileNameCharacter);

    return l_safeName;
}
```

> `l_assetFilePathRegistry.Add(...)` の第2引数に `GenerateVALUUID()` を直接渡しているのは、規約 11-11 では OK(戻り値に続けて書いていない)。

### Static / Skeletal の ModelSystem(変更)

`.h` のメンバに `ModelMaterialFileCreator m_materialFileCreator = {};` を足し、
`BuildStaticModelAssetData`(Skeletal は `BuildSkeletalAnimationModelAssetData`)の最後、`m_binaryConverter.Save` の後に:

```cpp
    // FBXのマテリアルから、メッシュごとの既定の.mat(StandardLit_<サブメッシュ名>.mat)を作る
    // 既にあるファイルは上書きしない
    m_materialFileCreator.CreateDefaultModelMaterialFileList(a_filePath, a_staticModelRecord);
```

---

# S4-4(S0 へ移した)

2026-10-11 : ModelComponent を設計から外したため、旧 S4-4(ModelComponent のマテリアルのスロット・JSON・インスペクター)は
`S0_ModelComponent.md` の末尾「S0 を設計し直すときの材料」へ移した。S4 で写経するのは S4-1 ~ S4-3 だけ。

---

## S3 との関係

S3 の ModelRenderSystem は、テーブルを `shared_ptr` で持ち `FindVALTable` で `weak_ptr` を渡す形にしてある(S3 の文書に反映済み)。
マテリアル(`ModelMaterialBase::m_table`)も同じく `weak_ptr` で覚えるので、アプリの終了時の破棄順の問題は起きない。

## 動作の確認

ビルドは S6 の後。ModelComponent(S0)が無い間はモデルを描く側が無いので、S6 のビルドの後に確かめられるのは次だけ。

- FBX を初めて置くと、同じフォルダに `StandardLit_<名前>.mat` ができ、アセットブラウザーに .mat が出る(Registry に登録されている)。
- 起動時にエラーマテリアルが作られ、GPU データの作成のアサートが出ない。

スロットの割り当て・見た目の切り替え・マゼンタの表示は、S0 で ModelComponent を作った後に確かめる。