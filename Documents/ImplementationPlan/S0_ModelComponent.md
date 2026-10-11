# S0 ModelComponent(最小)と、コンポーネントの Attach / Detach

## 目的

- 今のエンジンには、モデルの「描画申請」(`AddDrawRequest`)を出す側が1つも無い。モデルを置いても描かれず、速さも測れない。
- そこで、GameObject に付けると FBX のモデルを描くコンポーネント `GameObjectModelComponent` を作る。
  - Static(動かないモデル)と Skeletal(骨で動くモデル)の違いは、**モデルを設定したときに1回だけ選ぶ「描き方のクラス(Strategy)」**に任せる。毎フレーム `if (スケルタルなら)` と分岐しない。
- あわせて、コンポーネントへ「シーンに入った / シーンから外れた」を知らせる `Attach` / `Detach` を作る。
  - 今は、エディターでコンポーネントを削除しても、Undo のためにコマンドが実体を持ち続ける。描画の登録を1回だけにすると、消したはずのモデルが描かれ続けてしまう。
  - S5(描画の登録)で「登録は1回だけ」にするための土台でもある。

この段階では、描画そのものは今までの仕組み(`StaticModelStandardLitPerObjectDrawRequest` など、毎フレーム CB を書く方式)のまま使う。S1 でこの状態の速さを測り、S2 以降で速くしていく。

## DirectX12 / 描画の基礎の解説

### 座標の空間(ワールド・ビュー・クリップ)

モデルの頂点は、何段階かの「空間」を通って画面に出る。

| 空間 | 何を基準にした座標か | 例 |
|---|---|---|
| ローカル(モデル)空間 | モデル自身の原点。FBX に保存されている座標 | 頭のてっぺんが (0, 1.7, 0) |
| ワールド空間 | シーン全体の原点 | キャラを (10, 0, 5) に置けば、頭は (10, 1.7, 5) |
| ビュー(カメラ)空間 | カメラの位置を原点、カメラの向きを +Z にした座標 | カメラの 3m 前にあれば z = 3 |
| クリップ空間 | 遠近感を付けた後の座標(画面の端が -1 / +1 になる) | 画面中央なら x = 0, y = 0 |

- ローカル → ワールドの変換が **ワールド行列**。`GameObjectTransformComponent::GetREFMatrix()` が、位置・回転・拡大縮小から作った行列を持っている。
- ワールド → ビューが **ビュー行列**、ビュー → クリップが **プロジェクション行列**。どちらもカメラ(`CBCameraPass`)が持つ。
- このコンポーネントの仕事は、毎フレーム「今のワールド行列」を描画の仕組みへ渡すこと。

### 法線にはワールド行列の「逆転置行列」を使う

- 法線(面の向き)は、位置と違って、**拡大縮小が均一でないと、ワールド行列をそのまま掛けると向きが狂う**。
  - 例 : 球を Y 方向だけ 0.5 倍に潰すと、表面は寝るのに、ワールド行列を掛けた法線は逆に立ってしまう。
- 逆行列を転置した行列(逆転置行列)を掛けると、潰れても面に垂直なまま保てる。
- そのため、描画申請には `m_worldMatrix` と `m_worldInverseTransposeMatrix` の2つを渡す。

### 描画申請(DrawRequestPerObject)の今の流れ

1. 描画したい側が `std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData>`(モデルと行列)を作って持つ。
2. `StaticModelStandardLitPerObjectDrawRequest::AddDrawRequest` に渡すと、申請側は **weak_ptr** で覚える。
3. 毎フレーム、描画パスが weak_ptr を lock() し、生きている申請だけ、メッシュごとに CB を書いて `DispatchMesh` を呼ぶ。
4. 描画したい側が shared_ptr を手放すと、lock() に失敗して自動的に描かれなくなる(`RemoveExpiredElements` がリストから消す)。

このステップでは、外したいときに確実に外せるよう `RemoveDrawRequest` も足す。

## 設計

### クラスの関係

```
GameObjectModelComponent(コンポーネント本体)
 ├─ AssetFilePath m_modelFilePath         … FBX を UUID で指す(名前変更・移動に強い)
 ├─ bool m_isSkeletal                       … インスペクターで選ぶ(後で自動判定にしてもよい)
 └─ std::unique_ptr<GameObjectModelComponentRendererBase> m_renderer   … 描き方(Strategy)
       ├─ GameObjectModelComponentStaticRenderer    … StaticModel + 描画申請データ
       └─ GameObjectModelComponentSkeletalRenderer  … SkeletalAnimationModel + Player + 描画申請データ
```

### Attach / Detach を呼ぶ場所

| 出来事 | 呼ぶ場所 | 呼ばれる関数 |
|---|---|---|
| GameObject がシーンに入った(読み込み・作成・複製・Undo で戻した) | `Scene::AddGameObject` | `GameObject::ApplyIsInScene(true)` → 全コンポーネントの `Attach` |
| GameObject がシーンから外れた(エディターでの削除) | `Scene::UnregisterGameObject` | `GameObject::ApplyIsInScene(false)` → 全コンポーネントの `Detach` |
| GameObject が破棄された(ゲーム中の Destroy) | `Scene::RemoveDestroyedGameObjects` | 同上 |
| シーンにいる GameObject へコンポーネントを足した(追加・Undo で戻した) | `GameObjectComponentContainer::AddComponent` | そのコンポーネントの `Attach` |
| コンポーネントを外した(削除・Redo) | `GameObjectComponentContainer::RemoveComponent` | そのコンポーネントの `Detach` |

- **シーンにいない GameObject(クリップボードの複製・Prefab の作業用など)では Attach しない。** そのため GameObject に `m_isInScene` を持たせ、`AddComponent` はシーンにいるときだけ Attach を呼ぶ。
- Attach / Detach は2回呼ばれても問題ないように作る(ModelComponent は `m_isAttached` で確認する)。

## ファイル一覧

### 新規

| ファイル | 内容 |
|---|---|
| `Source/Framework/GameObject/Component/Model/GameObjectModelComponent.h/.cpp` | コンポーネント本体 |
| `Source/Framework/GameObject/Component/Model/Renderer/GameObjectModelComponentRendererBase.h` | 描き方の基底(.h のみ) |
| `Source/Framework/GameObject/Component/Model/Renderer/Static/GameObjectModelComponentStaticRenderer.h/.cpp` | Static の描き方 |
| `Source/Framework/GameObject/Component/Model/Renderer/Skeletal/GameObjectModelComponentSkeletalRenderer.h/.cpp` | Skeletal の描き方 |
| `Source/Framework/GameObject/Component/Model/Converter/Json/GameObjectModelComponentJsonConverter.h/.cpp` | JSON |
| `Source/Framework/GameObject/Component/Model/Inspector/GameObjectModelComponentInspector.h/.cpp` | インスペクター |
| `Source/Framework/Definition/Constant/GameObject/GameObjectModelComponentConstant.h` | 既定値の定数 |

### 変更

| ファイル | 変更 |
|---|---|
| `GameObject/Component/GameObjectComponentBase.h` | `Attach` / `Detach` の仮想関数 |
| `GameObject/Component/GameObjectComponentContainer.h/.cpp` | `AttachComponents` / `DetachComponents`、AddComponent / RemoveComponent から呼ぶ |
| `GameObject/GameObject.h/.cpp` | `m_isInScene` と `ApplyIsInScene`、INIT で戻す |
| `Scene/Scene.cpp` | AddGameObject / UnregisterGameObject / RemoveDestroyedGameObjects から ApplyIsInScene |
| `Definition/Enum/Asset/AssetFilePathEnum.h` | `AssetFilePathType::Model` |
| `Definition/Constant/GameObject/GameObjectComponentTaggedFactoryConstant.h` | `k_gameObjectComponentTagModel` |
| `Asset/Inspector/AssetFilePathInspector.h/.cpp` | FBX をドロップしたら Model として Registry へ登録 |
| `Editor/Window/AssetBrowser/Watcher/Change/Delete/AssetBrowserEditorWindowDirectoryDeleteChange.cpp` | Model の削除 |
| `Editor/Window/AssetBrowser/Watcher/Change/FilePath/AssetBrowserEditorWindowDirectoryFilePathChange.cpp` | Model の名前変更・移動 |
| `Graphics/Render/Graph/Request/Object/Model/Static/StaticModelPerObjectDrawRequestBase.h/.cpp` | `RemoveDrawRequest` |
| `Graphics/Render/Graph/Request/Object/Model/Skeletal/SkeletalAnimationModelPerObjectDrawRequestBase.h/.cpp` | `RemoveDrawRequest` |
| `Graphics/Render/Graph/Request/Object/Model/Skeletal/SkeletalAnimationPerObjectComputeRequest.h/.cpp` | `RemoveComputeRequest` |

### 登録(vcxproj / filters / Framework.h)

- フィルター: `Source\Framework\GameObject\Component\Model` / `...\Model\Renderer` / `...\Renderer\Static` / `...\Renderer\Skeletal` / `...\Model\Converter` / `...\Model\Converter\Json` / `...\Model\Inspector`
- Framework.h の順番(GameObjectCameraComponent の後):
  1. `Definition/Constant/GameObject/GameObjectModelComponentConstant.h`(Constant の区画)
  2. `GameObject/Component/Model/Converter/Json/GameObjectModelComponentJsonConverter.h`
  3. `GameObject/Component/Model/Inspector/GameObjectModelComponentInspector.h`
  4. `GameObject/Component/Model/Renderer/GameObjectModelComponentRendererBase.h`
  5. `GameObject/Component/Model/Renderer/Static/GameObjectModelComponentStaticRenderer.h`
  6. `GameObject/Component/Model/Renderer/Skeletal/GameObjectModelComponentSkeletalRenderer.h`
  7. `GameObject/Component/Model/GameObjectModelComponent.h`
  - ※ Renderer は Graphics の StaticModel / SkeletalAnimationPlayer / 描画申請を使うので、それらより後ろに置く。

---

## コード

### Definition/Constant/GameObject/GameObjectModelComponentConstant.h(新規)

```cpp
#pragma once

namespace FWK::Constant
{
    inline constexpr float k_gameObjectModelComponentDefaultPlaybackSpeed = 1.0F;

    inline constexpr std::uint32_t k_gameObjectModelComponentFirstMotionIndex = 0U;
}
```

### Definition/Constant/GameObject/GameObjectComponentTaggedFactoryConstant.h(変更)

```cpp
    inline constexpr std::string_view k_gameObjectComponentTagCamera = "カメラ";
    inline constexpr std::string_view k_gameObjectComponentTagModel  = "モデル";
```

### Definition/Enum/Asset/AssetFilePathEnum.h(変更)

```cpp
    enum class AssetFilePathType
    {
        Invalid,
        Prefab,
        Scene,
        Texture,
        Model,
    };

    FWK_JSON_SERIALIZE_ENUM
    (
        AssetFilePathType,
        FWK_JSON_ENUM_VALUE(AssetFilePathType::Invalid),
        FWK_JSON_ENUM_VALUE(AssetFilePathType::Prefab),
        FWK_JSON_ENUM_VALUE(AssetFilePathType::Scene),
        FWK_JSON_ENUM_VALUE(AssetFilePathType::Texture),
        FWK_JSON_ENUM_VALUE(AssetFilePathType::Model),
    )
```

### GameObject/Component/GameObjectComponentBase.h(変更)

`Clone` の後ろ、`IsAllowMultiple` の前に追加する。

```cpp
        virtual std::shared_ptr<GameObjectComponentBase> Clone() const = 0;

        virtual void Attach() { /*必要に応じてオーバーライドしてください*/ };
        virtual void Detach() { /*必要に応じてオーバーライドしてください*/ };

        virtual bool IsAllowMultiple() const { return false; }
```

### GameObject/Component/GameObjectComponentContainer.h(変更)

`SweepExpiredComponents` の前に追加する。

```cpp
        bool AddComponent                 (const std::shared_ptr<GameObjectComponentBase>& a_component);
        void AddPrefabRemovedComponentUUID(const boost::uuids::uuid&                       a_uuid);

        void AttachComponents() const;
        void DetachComponents() const;

        void SweepExpiredComponents();
```

### GameObject/Component/GameObjectComponentContainer.cpp(変更)

**AddComponent の最後(`return true;` の前)に追加:**

```cpp
    // 持ち主のGameObjectがシーンにいるときだけ、「シーンに入った」ことを知らせる
    // クリップボードの複製など、シーンにいないGameObjectのコンポーネントが
    // 描画の登録などをしてしまわないようにするため
    if (const auto& l_owner = m_owner.lock();
        l_owner &&
        l_owner->GetVALIsInScene())
    {
        a_component->Attach();
    }

    return true;
```

**RemoveComponent の最後(`m_componentUUIDRegistry.Erase(l_uuid);` の後)に追加:**

```cpp
    // 外したコンポーネントへ「シーンから外れた」ことを知らせる
    // Undoのためにコマンドが実体を持ち続けても、描画などの登録が残らないようにする
    l_component->Detach();
```

**AddPrefabRemovedComponentUUID の後ろに追加(写経):**

```cpp
void FWK::GameObjectComponentContainer::AttachComponents() const
{
    // 持ち主のGameObjectがシーンに入ったときに、全コンポーネントへ知らせる
    // 例 : モデルのコンポーネントは、ここで描画の登録をする
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { continue; }

        l_component->Attach();
    }
}
void FWK::GameObjectComponentContainer::DetachComponents() const
{
    // 持ち主のGameObjectがシーンから外れたときに、全コンポーネントへ知らせる
    // 例 : モデルのコンポーネントは、ここで描画の登録を外す
    const auto& l_componentDataList = m_componentSmartPointerVectorList.GetREFElementDataList();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { continue; }

        l_component->Detach();
    }
}
```

### GameObject/GameObject.h(変更)

```cpp
        void ClearAllPrefabRemovedUUIDSet();

        void ApplyIsInScene(const bool a_isInScene);

        void SetName(const std::string& a_set) { m_name = a_set; }
```

```cpp
        bool GetVALIsDestroyed   () const { return m_isDestroyed; }
        bool GetVALIsInScene     () const { return m_isInScene; }
        bool GetVALIsPrefabOrigin() const { return m_isPrefabOrigin; }
```

```cpp
        bool m_isDestroyed    = false;
        bool m_isInScene      = false;
        bool m_isPrefabOrigin = Constant::k_gameObjectInitialValueIsPrefabOrigin;
```

### GameObject/GameObject.cpp(変更)

**INIT の最後の代入に追加:**

```cpp
    m_isDestroyed    = false;
    m_isInScene      = false;
    m_isPrefabOrigin = Constant::k_gameObjectInitialValueIsPrefabOrigin;
```

**ClearAllPrefabRemovedUUIDSet の後ろに追加(写経):**

```cpp
void FWK::GameObject::ApplyIsInScene(const bool a_isInScene)
{
    // 同じ状態なら何もしない
    // 例 : 読み込みで作ったGameObjectは、AddComponentのたびにではなく、Sceneへ入った1回だけ知らせる
    if (m_isInScene == a_isInScene) { return; }

    m_isInScene = a_isInScene;

    // シーンへ入った / シーンから外れたことを、全コンポーネントへ知らせる
    if (m_isInScene)
    {
        m_componentContainer.AttachComponents();
    }
    else
    {
        m_componentContainer.DetachComponents();
    }
}
```

### Scene/Scene.cpp(変更)

**AddGameObject の最後(`m_pendingAddGameObjectList.emplace_back(a_gameObject);` の後):**

```cpp
    // シーンへ入ったことをGameObjectとそのコンポーネントへ知らせる
    // Undoで戻したGameObjectも、ここを通って描画の登録などをやり直す
    a_gameObject->ApplyIsInScene(true);
```

**UnregisterGameObject の最後(実行階層リストのループの後):**

```cpp
    // シーンから外れたことをGameObjectとそのコンポーネントへ知らせる
    // 実体は呼び出し側(Undoのコマンドなど)が持ち続けるため、描画などの登録をここで外す
    l_gameObject->ApplyIsInScene(false);
```

**RemoveDestroyedGameObjects の先頭(`std::erase_if(m_gameObjectList, ...)` の前):**

```cpp
    // 破棄されたGameObjectを、リストから取り除く前にシーンから外れた状態にする
    // 取り除いた後もどこかがshared_ptrを持っていると、描画などの登録が残ってしまうため
    auto l_destroyedGameObjectView = std::views::filter(m_gameObjectList,
                                                        [](const auto& a_gameObject)
                                                        {
                                                            return a_gameObject &&
                                                                   a_gameObject->GetVALIsDestroyed();
                                                        });

    for (const auto& l_destroyedGameObject : l_destroyedGameObjectView)
    {
        l_destroyedGameObject->ApplyIsInScene(false);
    }
```

### Asset/Inspector/AssetFilePathInspector.h(変更)

`RegisterTextureFilePath` を、拡張子と種類を受け取る形にする。

```cpp
    private:

        void RegisterDroppedFilePath(const std::filesystem::path&  a_filePath,
                                     const std::filesystem::path&  a_extension,
                                     const Enum::AssetFilePathType a_type,
                                           AssetFilePathRegistry&  a_assetFilePathRegistry) const;

        bool ApplyDroppedFilePath(const std::filesystem::path& a_droppedFilePath, AssetFilePath& a_assetFilePath) const;
```

### Asset/Inspector/AssetFilePathInspector.cpp(変更・写経)

```cpp
void FWK::AssetFilePathInspector::RegisterDroppedFilePath(const std::filesystem::path&  a_filePath,
                                                          const std::filesystem::path&  a_extension,
                                                          const Enum::AssetFilePathType a_type,
                                                                AssetFilePathRegistry&  a_assetFilePathRegistry) const
{
    // 指定された拡張子以外のファイルは登録しない
    // 例 : テクスチャは".png"、モデルは".fbx"
    if (!Utility::CanLoadFilePath(a_filePath, a_extension)) { return; }

    // 新しいUUIDを発行し、ファイルパスと結び付けてRegistryへ登録する
    // 登録後は、ファイルの名前変更や移動があってもWatcherがRegistry側のパスを書き換えるため、
    // 同じUUIDから同じファイルを辿れる
          auto& l_uuidManager = Utility::UUIDManager::GetInstance();
    const auto& l_assetUUID   = l_uuidManager.GenerateVALUUID    ();

    a_assetFilePathRegistry.Add(a_filePath, l_assetUUID, a_type);
}

bool FWK::AssetFilePathInspector::ApplyDroppedFilePath(const std::filesystem::path& a_droppedFilePath, AssetFilePath& a_assetFilePath) const
{
    auto& l_application           = Application::GetInstance                        ();
    auto& l_assetFilePathRegistry = l_application.GetMutableREFAssetFilePathRegistry();

    // テクスチャ(PNG)とモデル(FBX)は、ドロップされた時点でRegistryに無ければ登録する
    // Prefab / Sceneは作成したときに登録されているため、ここでは登録しない
    if (!l_assetFilePathRegistry.FindPTRAssetUUID(a_droppedFilePath))
    {
        switch (a_assetFilePath.GetVALAllowedType())
        {
            case Enum::AssetFilePathType::Texture:
            {
                RegisterDroppedFilePath(a_droppedFilePath, Constant::k_lowerPNGExtension, Enum::AssetFilePathType::Texture, l_assetFilePathRegistry);
            }
            break;

            case Enum::AssetFilePathType::Model:
            {
                RegisterDroppedFilePath(a_droppedFilePath, Constant::k_lowerFBXExtension, Enum::AssetFilePathType::Model, l_assetFilePathRegistry);
            }
            break;

            default:
            break;
        }
    }

    // ここから下は今までと同じ
    // (Registryに無いファイルは受け取らない / 種類が違えば受け取らない / 同じUUIDなら変更なし / SetAssetFilePathUUID)
    ...
}
```

### AssetBrowserEditorWindowDirectoryDeleteChange.cpp / AssetBrowserEditorWindowDirectoryFilePathChange.cpp(変更)

`case Enum::AssetFilePathType::Texture:` のブロックの後ろに、同じ中身の `case Enum::AssetFilePathType::Model:` を足す。
モデルもテクスチャと同じく、Registry から消す(削除)/ Registry のパスだけを書き換える(名前変更・移動)だけでよい。

```cpp
            case Enum::AssetFilePathType::Model:
            {
                // モデルも、他のデータから参照を外す処理がないため、Registryから取り除くだけでよい
                // 取り除くと、このUUIDを持つAssetFilePathは「不明なパス」として表示される
                a_assetFilePathRegistry.Erase(a_deleteFilePath);

                return;
            }
            break;
```

```cpp
        case Enum::AssetFilePathType::Model:
        {
            // UUIDはそのままで、Registry側のパスだけを新しいパスへ書き換える
            a_assetFilePathRegistry.ReplaceFilePath(a_oldFilePath, a_newFilePath);

            return;
        }
        break;
```

### 描画申請へ RemoveDrawRequest / RemoveComputeRequest を足す(変更)

**StaticModelPerObjectDrawRequestBase.h**

```cpp
        void AddDrawRequest   (const std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData>& a_drawRequestData);
        void RemoveDrawRequest(const std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData>& a_drawRequestData);
```

**StaticModelPerObjectDrawRequestBase.cpp(写経)**

```cpp
void FWK::Graphics::StaticModelPerObjectDrawRequestBase::RemoveDrawRequest(const std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData>& a_drawRequestData)
{
    FWK_ASSERT_RETURN_IF(!a_drawRequestData, "DrawRequestDataが無効のため、描画申請の削除に失敗しました。");

    // 同じアドレスの申請だけをリストから外す
    // weak_ptrのリストなので、持ち主のshared_ptrはそのまま残る(後でAddDrawRequestし直せる)
    m_forwardDrawRequestDataSmartPointerVectorList.RemoveSameElement(a_drawRequestData);
}
```

**SkeletalAnimationModelPerObjectDrawRequestBase.h / .cpp** も同じ形(型が `Struct::SkeletalAnimationModelPerObjectDrawRequestData` になるだけ)。

**SkeletalAnimationPerObjectComputeRequest.h**

```cpp
        void AddComputeRequest   (const std::shared_ptr<SkeletalAnimationPlayer>& a_skeletalAnimationPlayer);
        void RemoveComputeRequest(const std::shared_ptr<SkeletalAnimationPlayer>& a_skeletalAnimationPlayer);
```

**SkeletalAnimationPerObjectComputeRequest.cpp(写経)**

```cpp
void FWK::Graphics::SkeletalAnimationPerObjectComputeRequest::RemoveComputeRequest(const std::shared_ptr<SkeletalAnimationPlayer>& a_skeletalAnimationPlayer)
{
    FWK_ASSERT_RETURN_IF(!a_skeletalAnimationPlayer, "SkeletalAnimationPlayerが無効のため、計算申請の削除に失敗しました。");

    // 同じPlayerの申請だけをリストから外す
    // Player自体は持ち主(コンポーネント)が持ち続けるため、アニメーションの進み具合は失われない
    m_skeletalAnimationPlayerSmartPointerVectorList.RemoveSameElement(a_skeletalAnimationPlayer);
}
```

> `RemoveSameElement` の引数は weak_ptr のリストなら weak_ptr。shared_ptr から weak_ptr へは暗黙に変換されるので、そのまま渡せる。

### GameObject/Component/Model/Renderer/GameObjectModelComponentRendererBase.h(新規)

```cpp
#pragma once

namespace FWK
{
    class GameObjectModelComponentRendererBase
    {
    public:

                 GameObjectModelComponentRendererBase() = default;
        virtual ~GameObjectModelComponentRendererBase() = default;

        GameObjectModelComponentRendererBase(const GameObjectModelComponentRendererBase&)  = delete;
        GameObjectModelComponentRendererBase(      GameObjectModelComponentRendererBase&&) = delete;

        GameObjectModelComponentRendererBase& operator=(const GameObjectModelComponentRendererBase&)  = delete;
        GameObjectModelComponentRendererBase& operator=(      GameObjectModelComponentRendererBase&&) = delete;

        virtual bool Load(const std::filesystem::path& a_filePath) = 0;

        virtual void Update(const float) { /*必要に応じてオーバーライドしてください*/ };

        virtual void Register  () = 0;
        virtual void Unregister() = 0;

        virtual void ApplyWorldMatrix(const TypeAlias::Math::Matrix& a_worldMatrix) = 0;

    private:

        FWK_DEFINE_TYPE_INFO_ROOT(GameObjectModelComponentRendererBase)
    };
}
```

### GameObject/Component/Model/Renderer/Static/GameObjectModelComponentStaticRenderer.h(新規)

```cpp
#pragma once

namespace FWK
{
    class GameObjectModelComponentStaticRenderer final : public GameObjectModelComponentRendererBase
    {
    public:

         GameObjectModelComponentStaticRenderer()          = default;
        ~GameObjectModelComponentStaticRenderer() override = default;

        bool Load(const std::filesystem::path& a_filePath) override;

        void Register  () override;
        void Unregister() override;

        void ApplyWorldMatrix(const TypeAlias::Math::Matrix& a_worldMatrix) override;

    private:

        std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData> m_drawRequestData = nullptr;

        Graphics::StaticModel m_staticModel = {};

        bool m_isRegistered = false;

        FWK_DEFINE_TYPE_INFO(GameObjectModelComponentStaticRenderer, GameObjectModelComponentRendererBase)
    };
}
```

### GameObject/Component/Model/Renderer/Static/GameObjectModelComponentStaticRenderer.cpp(新規・写経)

```cpp
#include "GameObjectModelComponentStaticRenderer.h"

// 動かないモデル(StaticModel)を描くための「描き方」クラス
// モデルの読み込みと、描画申請(StaticModelPerObjectDrawRequestData)の登録・解除・行列の更新だけを担当する
// 描画申請はweak_ptrで覚えられるため、このクラスが破棄されれば、自動的に描かれなくなる
bool FWK::GameObjectModelComponentStaticRenderer::Load(const std::filesystem::path& a_filePath)
{
    // FBX(または変換済みの.staticModel)を読み込む
    // 同じファイルを使うモデルが既にあれば、実体は共有され参照数だけが増える
    if (!m_staticModel.Load(a_filePath)) { return false; }

    // 描画申請のデータを作る
    // 描画パスは、このデータからモデル(Record)と行列を読んで描く
    m_drawRequestData = std::make_shared<Struct::StaticModelPerObjectDrawRequestData>();

    m_drawRequestData->m_staticModelRecord = m_staticModel.GetREFStaticModelRecord();

    return true;
}

void FWK::GameObjectModelComponentStaticRenderer::Register()
{
    // 2回登録すると2回描かれてしまうため、登録済みなら何もしない
    if (m_isRegistered) { return; }

    // まだモデルを読み込んでいなければ、登録するものがない
    if (!m_drawRequestData) { return; }

    const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
    const auto& l_renderer        = l_graphicsManager.GetREFRenderer      ();
    const auto& l_renderGraph     = l_renderer.GetREFRenderGraph          ();

    // 見た目を描くパス(Lit)と、影を作るパスの、2つの描画申請へ登録する
    // GraphicsCONFIG.jsonのDrawRequestPerObjectListで作られたものを、型で探して取り出す
    const auto& l_litDrawRequest    = l_renderGraph.FindVALDrawRequestPerObject<Graphics::StaticModelStandardLitPerObjectDrawRequest>  ().lock();
    const auto& l_shadowDrawRequest = l_renderGraph.FindVALDrawRequestPerObject<Graphics::StaticModelCascadeShadowPerObjectDrawRequest>().lock();

    FWK_ASSERT_RETURN_IF(!l_litDrawRequest,    "StaticModelStandardLitPerObjectDrawRequestが無効のため、モデルの描画登録に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_shadowDrawRequest, "StaticModelCascadeShadowPerObjectDrawRequestが無効のため、モデルの描画登録に失敗しました。");

    l_litDrawRequest->AddDrawRequest   (m_drawRequestData);
    l_shadowDrawRequest->AddDrawRequest(m_drawRequestData);

    m_isRegistered = true;
}
void FWK::GameObjectModelComponentStaticRenderer::Unregister()
{
    if (!m_isRegistered) { return; }

    const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
    const auto& l_renderer        = l_graphicsManager.GetREFRenderer      ();
    const auto& l_renderGraph     = l_renderer.GetREFRenderGraph          ();

    const auto& l_litDrawRequest    = l_renderGraph.FindVALDrawRequestPerObject<Graphics::StaticModelStandardLitPerObjectDrawRequest>  ().lock();
    const auto& l_shadowDrawRequest = l_renderGraph.FindVALDrawRequestPerObject<Graphics::StaticModelCascadeShadowPerObjectDrawRequest>().lock();

    FWK_ASSERT_RETURN_IF(!l_litDrawRequest,    "StaticModelStandardLitPerObjectDrawRequestが無効のため、モデルの描画登録の解除に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_shadowDrawRequest, "StaticModelCascadeShadowPerObjectDrawRequestが無効のため、モデルの描画登録の解除に失敗しました。");

    l_litDrawRequest->RemoveDrawRequest   (m_drawRequestData);
    l_shadowDrawRequest->RemoveDrawRequest(m_drawRequestData);

    m_isRegistered = false;
}

void FWK::GameObjectModelComponentStaticRenderer::ApplyWorldMatrix(const TypeAlias::Math::Matrix& a_worldMatrix)
{
    if (!m_drawRequestData) { return; }

    // ワールド行列 : モデルの頂点(ローカル座標)をシーンの座標(ワールド座標)へ移す行列
    // 逆転置行列   : 法線を移すための行列。拡大縮小が均一でなくても、法線が面に垂直なまま保たれる
    // 例 : Y方向だけ0.5倍に潰した球でも、逆転置行列を掛けた法線は表面に垂直になる
    m_drawRequestData->m_worldMatrix                 = a_worldMatrix;
    m_drawRequestData->m_worldInverseTransposeMatrix = a_worldMatrix.Invert().Transpose();
}
```

> `Invert()` / `Transpose()` は DirectX::SimpleMath(TypeAlias::Math::Matrix)の関数。11-11 の「自作の関数の戻り値に続けて書かない」は、外部ライブラリなので対象外。

### GameObject/Component/Model/Renderer/Skeletal/GameObjectModelComponentSkeletalRenderer.h(新規)

```cpp
#pragma once

namespace FWK
{
    class GameObjectModelComponentSkeletalRenderer final : public GameObjectModelComponentRendererBase
    {
    public:

         GameObjectModelComponentSkeletalRenderer()          = default;
        ~GameObjectModelComponentSkeletalRenderer() override = default;

        bool Load(const std::filesystem::path& a_filePath) override;

        void Update(const float a_deltaTime) override;

        void Register  () override;
        void Unregister() override;

        void ApplyWorldMatrix(const TypeAlias::Math::Matrix& a_worldMatrix) override;

    private:

        std::shared_ptr<Graphics::SkeletalAnimationPlayer>                       m_skeletalAnimationPlayer = nullptr;
        std::shared_ptr<Struct::SkeletalAnimationModelPerObjectDrawRequestData> m_drawRequestData         = nullptr;

        Graphics::SkeletalAnimationModel m_skeletalAnimationModel = {};

        bool m_isRegistered = false;

        FWK_DEFINE_TYPE_INFO(GameObjectModelComponentSkeletalRenderer, GameObjectModelComponentRendererBase)
    };
}
```

### GameObject/Component/Model/Renderer/Skeletal/GameObjectModelComponentSkeletalRenderer.cpp(新規・写経)

```cpp
#include "GameObjectModelComponentSkeletalRenderer.h"

// 骨で動くモデル(SkeletalAnimationModel)を描くための「描き方」クラス
// Staticとの違いは次の2つ
// 1. 1体ごとにアニメーションの進み具合(SkeletalAnimationPlayer)を持ち、毎フレーム時間を進める
// 2. 描く前にGPUでスキニング(骨の動きで頂点を動かす計算)をするため、計算申請にも登録する
bool FWK::GameObjectModelComponentSkeletalRenderer::Load(const std::filesystem::path& a_filePath)
{
    if (!m_skeletalAnimationModel.Load(a_filePath)) { return false; }

    // アニメーションの進み具合を持つPlayerを作る
    // Playerは、フレームリソースの数だけ骨の行列のバッファと、スキニング後の頂点のバッファを持つ
    // (GPUが数フレーム遅れて描くため、描画中のバッファをCPUが上書きしないようにするため)
    auto l_skeletalAnimationPlayer = std::make_shared<Graphics::SkeletalAnimationPlayer>();

    FWK_ASSERT_RETURN_VALUE_IF(!l_skeletalAnimationPlayer->Create(m_skeletalAnimationModel), "SkeletalAnimationPlayerの作成に失敗したため、モデルの読み込みに失敗しました。", false);

    // モーションを持っていれば、最初のモーションをループ再生する
    // どのモーションを再生するかの選択は、フェーズ2(Animator)で作る
    const auto& l_skeletalAnimationModelRecord = m_skeletalAnimationModel.GetREFSkeletalAnimationModelRecord().lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_skeletalAnimationModelRecord, "SkeletalAnimationModelRecordが無効のため、モデルの読み込みに失敗しました。", false);

    const auto& l_modelData = l_skeletalAnimationModelRecord->GetREFModelData();

    if (!l_modelData.m_motionSequenceList.empty())
    {
        l_skeletalAnimationPlayer->PlayMotion(Constant::k_gameObjectModelComponentFirstMotionIndex, Constant::k_gameObjectModelComponentDefaultPlaybackSpeed, true);
    }

    m_skeletalAnimationPlayer = std::move(l_skeletalAnimationPlayer);

    m_drawRequestData = std::make_shared<Struct::SkeletalAnimationModelPerObjectDrawRequestData>();

    m_drawRequestData->m_skeletalAnimationPlayer = m_skeletalAnimationPlayer;

    return true;
}

void FWK::GameObjectModelComponentSkeletalRenderer::Update(const float a_deltaTime)
{
    if (!m_skeletalAnimationPlayer) { return; }

    // アニメーションの時間を進め、今のフレーム用の骨の行列をCPUで計算する
    // 計算した行列は、描画の直前にSkeletalAnimationComputePassがGPUへ送り、スキニングに使う
    m_skeletalAnimationPlayer->AdvanceTime(a_deltaTime);
}

void FWK::GameObjectModelComponentSkeletalRenderer::Register()
{
    if (m_isRegistered) { return; }

    if (!m_drawRequestData) { return; }

    const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
    const auto& l_renderer        = l_graphicsManager.GetREFRenderer      ();
    const auto& l_renderGraph     = l_renderer.GetREFRenderGraph          ();

    // スキニングの計算申請 / 見た目を描くパス(Lit) / 影を作るパスの、3つへ登録する
    const auto& l_computeRequest    = l_renderGraph.FindVALComputeRequestPerObject<Graphics::SkeletalAnimationPerObjectComputeRequest>              ().lock();
    const auto& l_litDrawRequest    = l_renderGraph.FindVALDrawRequestPerObject   <Graphics::SkeletalAnimationModelStandardLitPerObjectDrawRequest>  ().lock();
    const auto& l_shadowDrawRequest = l_renderGraph.FindVALDrawRequestPerObject   <Graphics::SkeletalAnimationModelCascadeShadowPerObjectDrawRequest>().lock();

    FWK_ASSERT_RETURN_IF(!l_computeRequest,    "SkeletalAnimationPerObjectComputeRequestが無効のため、モデルの描画登録に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_litDrawRequest,    "SkeletalAnimationModelStandardLitPerObjectDrawRequestが無効のため、モデルの描画登録に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_shadowDrawRequest, "SkeletalAnimationModelCascadeShadowPerObjectDrawRequestが無効のため、モデルの描画登録に失敗しました。");

    l_computeRequest->AddComputeRequest(m_skeletalAnimationPlayer);
    l_litDrawRequest->AddDrawRequest   (m_drawRequestData);
    l_shadowDrawRequest->AddDrawRequest(m_drawRequestData);

    m_isRegistered = true;
}
void FWK::GameObjectModelComponentSkeletalRenderer::Unregister()
{
    if (!m_isRegistered) { return; }

    const auto& l_graphicsManager = Graphics::GraphicsManager::GetInstance();
    const auto& l_renderer        = l_graphicsManager.GetREFRenderer      ();
    const auto& l_renderGraph     = l_renderer.GetREFRenderGraph          ();

    const auto& l_computeRequest    = l_renderGraph.FindVALComputeRequestPerObject<Graphics::SkeletalAnimationPerObjectComputeRequest>              ().lock();
    const auto& l_litDrawRequest    = l_renderGraph.FindVALDrawRequestPerObject   <Graphics::SkeletalAnimationModelStandardLitPerObjectDrawRequest>  ().lock();
    const auto& l_shadowDrawRequest = l_renderGraph.FindVALDrawRequestPerObject   <Graphics::SkeletalAnimationModelCascadeShadowPerObjectDrawRequest>().lock();

    FWK_ASSERT_RETURN_IF(!l_computeRequest,    "SkeletalAnimationPerObjectComputeRequestが無効のため、モデルの描画登録の解除に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_litDrawRequest,    "SkeletalAnimationModelStandardLitPerObjectDrawRequestが無効のため、モデルの描画登録の解除に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_shadowDrawRequest, "SkeletalAnimationModelCascadeShadowPerObjectDrawRequestが無効のため、モデルの描画登録の解除に失敗しました。");

    l_computeRequest->RemoveComputeRequest(m_skeletalAnimationPlayer);
    l_litDrawRequest->RemoveDrawRequest   (m_drawRequestData);
    l_shadowDrawRequest->RemoveDrawRequest(m_drawRequestData);

    m_isRegistered = false;
}

void FWK::GameObjectModelComponentSkeletalRenderer::ApplyWorldMatrix(const TypeAlias::Math::Matrix& a_worldMatrix)
{
    if (!m_drawRequestData) { return; }

    // StaticRendererと同じく、ワールド行列と、法線用の逆転置行列を渡す
    m_drawRequestData->m_worldMatrix                 = a_worldMatrix;
    m_drawRequestData->m_worldInverseTransposeMatrix = a_worldMatrix.Invert().Transpose();
}
```

### GameObject/Component/Model/GameObjectModelComponent.h(新規)

```cpp
#pragma once

namespace FWK
{
    class GameObjectModelComponent final : public GameObjectComponentBase
    {
    public:

         GameObjectModelComponent();
        ~GameObjectModelComponent() override;

        void Deserialize(const nlohmann::json& a_rootJson) override;

        void PostDeserialize() override;

        void Update        () override;
        void PostLateUpdate() override;

        void EditInspector() override;

        nlohmann::json Serialize() const override;

        std::shared_ptr<GameObjectComponentBase> Clone() const override;

        void Attach() override;
        void Detach() override;

        void ReloadModel();

        void ApplyIsSkeletal(const bool a_isSkeletal);

        void SetIsSkeletal(const bool a_set) { m_isSkeletal = a_set; }

        const auto& GetREFModelFilePath() const { return m_modelFilePath; }

        auto& GetMutableREFModelFilePath() { return m_modelFilePath; }

        bool GetVALIsSkeletal() const { return m_isSkeletal; }

    private:

        void ApplyWorldMatrixToRenderer() const;

        std::unique_ptr<GameObjectModelComponentRendererBase> m_renderer;

        AssetFilePath                                        m_modelFilePath;
        Utility::FetchSelfGameObjectTransformComponentHelper m_fetchSelfGameObjectTransformComponentHelper;

        GameObjectModelComponentInspector m_inspector;

        Converter::GameObjectModelComponentJsonConverter m_jsonConverter;

        bool m_isSkeletal;
        bool m_isAttached;

        FWK_DEFINE_TYPE_INFO(GameObjectModelComponent, GameObjectComponentBase)
    };
}

FWK_REGISTER_FACTORY_METHOD                      (FWK::TypeAlias::GameObjectComponentSharedFactory, FWK::GameObjectModelComponent)
FWK_REGISTER_TAGGED_GAME_OBJECT_COMPONENT_FACTORY(FWK::Constant::k_gameObjectComponentTagModel,     FWK::GameObjectModelComponent)
```

> コンストラクタで `m_modelFilePath` に受け取る種類(Model)を設定するため、コンストラクタは `= default` にしない。そのため規約 7-6 に従い、全メンバを初期化子リストで初期化し、宣言には `= {}` を書かない。

### GameObject/Component/Model/GameObjectModelComponent.cpp(新規・写経)

```cpp
#include "GameObjectModelComponent.h"
#include "../../../../Application/Application.h"

// GameObjectにFBXのモデルを持たせ、描画の仕組みへ登録するコンポーネント
// Static(動かないモデル)とSkeletal(骨で動くモデル)の違いは、
// モデルを設定したときに1回だけ作る「描き方」(GameObjectModelComponentRendererBaseの派生)に任せる
// こうすると、毎フレームの処理で「スケルタルかどうか」をif文で分ける必要がなくなる
FWK::GameObjectModelComponent::GameObjectModelComponent() :
    m_renderer(nullptr),

    m_modelFilePath                              (),
    m_fetchSelfGameObjectTransformComponentHelper(),

    m_inspector(),

    m_jsonConverter(),

    m_isSkeletal(false),
    m_isAttached(false)
{
    // AssetFilePathが受け取るファイルの種類をモデル(FBX)に限定する
    // インスペクターへテクスチャなど別の種類をドロップしても、受け取らなくなる
    m_modelFilePath.SetAllowedType(Enum::AssetFilePathType::Model);
}
FWK::GameObjectModelComponent::~GameObjectModelComponent() = default;

void FWK::GameObjectModelComponent::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::GameObjectModelComponent::PostDeserialize()
{
    // 自身のGameObjectが持つTransformComponentをキャッシュする
    // PostLateUpdateで毎フレームGameObjectから取り直さなくて済むようにするため
    m_fetchSelfGameObjectTransformComponentHelper.PostDeserialize(GetREFOwner());

    // 保存されていたファイルパスから、モデルを読み込み直す
    ReloadModel();
}

void FWK::GameObjectModelComponent::Update()
{
    if (!m_renderer) { return; }

    const auto& l_application   = Application::GetInstance         ();
    const auto& l_fpsController = l_application.GetREFFPSController();

    // 描き方ごとの毎フレームの処理(Skeletalならアニメーションの時間を進める)
    // Staticは何もしない(基底クラスの空の関数が呼ばれる)
    m_renderer->Update(l_fpsController.GetVALScaledDeltaTime());
}
void FWK::GameObjectModelComponent::PostLateUpdate()
{
    // TransformComponentの行列は、Update / LateUpdateで位置が変わった後、
    // PostLateUpdateの中で最新になる
    // ここで描き方へ渡すことで、このフレームの最終的な位置で描かれる
    ApplyWorldMatrixToRenderer();
}

void FWK::GameObjectModelComponent::EditInspector()
{
    m_inspector.EditInspector(*this);
}

nlohmann::json FWK::GameObjectModelComponent::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

std::shared_ptr<FWK::GameObjectComponentBase> FWK::GameObjectModelComponent::Clone() const
{
    auto l_clone = std::make_shared<GameObjectModelComponent>();

    // JSONに乗る部分(ファイルパスとスケルタルかどうか)は、Serialize / Deserializeの往復で写す
    // モデルの読み込みは、複製先のPostDeserializeで行われる
    l_clone->Deserialize(Serialize());

    return l_clone;
}

void FWK::GameObjectModelComponent::Attach()
{
    // シーンに入った(またはシーンにいるGameObjectへ追加された)
    m_isAttached = true;

    // まだモデルを読み込んでいなければ、読み込んだ時点(ReloadModel)で登録する
    if (!m_renderer) { return; }

    m_renderer->Register();
}
void FWK::GameObjectModelComponent::Detach()
{
    // シーンから外れた(またはコンポーネントを外された)
    // Undoのためにこのコンポーネントが生き続けても、描かれないように登録を外す
    m_isAttached = false;

    if (!m_renderer) { return; }

    m_renderer->Unregister();
}

void FWK::GameObjectModelComponent::ReloadModel()
{
    // 今の描き方があれば、登録を外してから捨てる
    // 捨てるとStaticModel / SkeletalAnimationModelのハンドルも破棄され、参照数が1つ減る
    if (m_renderer)
    {
        m_renderer->Unregister();
    }

    m_renderer.reset();

    // ファイルが指定されていなければ、何も描かない
    const auto& l_filePath = m_modelFilePath.FetchVALFilePath();

    if (l_filePath.empty()) { return; }

    // スケルタルかどうかで、描き方のクラスを選んで作る
    // ここが「毎フレーム分岐しない」ための分かれ道で、以後はm_rendererの仮想関数を呼ぶだけになる
    std::unique_ptr<GameObjectModelComponentRendererBase> l_renderer = nullptr;

    if (m_isSkeletal)
    {
        l_renderer = std::make_unique<GameObjectModelComponentSkeletalRenderer>();
    }
    else
    {
        l_renderer = std::make_unique<GameObjectModelComponentStaticRenderer>();
    }

    if (!l_renderer->Load(l_filePath))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "モデルを読み込めませんでした。\nFilePath : {}", l_filePath.string());

        return;
    }

    m_renderer = std::move(l_renderer);

    // 登録する前に今の位置を渡しておく
    // 渡さないと、最初の1フレームだけ原点(単位行列)で描かれてしまう
    ApplyWorldMatrixToRenderer();

    // すでにシーンにいるなら、ここで描画の登録をする
    if (!m_isAttached) { return; }

    m_renderer->Register();
}

void FWK::GameObjectModelComponent::ApplyIsSkeletal(const bool a_isSkeletal)
{
    if (m_isSkeletal == a_isSkeletal) { return; }

    m_isSkeletal = a_isSkeletal;

    // 描き方のクラスが変わるため、モデルを読み込み直す
    ReloadModel();
}

void FWK::GameObjectModelComponent::ApplyWorldMatrixToRenderer() const
{
    if (!m_renderer) { return; }

    const auto& l_transformComponent = m_fetchSelfGameObjectTransformComponentHelper.GetREFTransformComponent().lock();

    if (!l_transformComponent) { return; }

    m_renderer->ApplyWorldMatrix(l_transformComponent->GetREFMatrix());
}
```

> `ApplyWorldMatrixToRenderer` は const。`m_renderer` は unique_ptr なので、const の関数の中でも指す先(描き方)の非 const 関数は呼べる(ポインタ自体が const になるだけ)。

### GameObject/Component/Model/Converter/Json/GameObjectModelComponentJsonConverter.h(新規)

```cpp
#pragma once

namespace FWK
{
    class GameObjectModelComponent;
}

namespace FWK::Converter
{
    class GameObjectModelComponentJsonConverter final
    {
    public:

         GameObjectModelComponentJsonConverter() = default;
        ~GameObjectModelComponentJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, GameObjectModelComponent& a_gameObjectModelComponent) const;

        nlohmann::json Serialize(const GameObjectModelComponent& a_gameObjectModelComponent) const;

    private:

        static constexpr std::string_view k_modelFilePathJsonKey = "ModelFilePath";
        static constexpr std::string_view k_isSkeletalJsonKey    = "IsSkeletal";
    };
}
```

### GameObject/Component/Model/Converter/Json/GameObjectModelComponentJsonConverter.cpp(新規・写経)

```cpp
#include "GameObjectModelComponentJsonConverter.h"

void FWK::Converter::GameObjectModelComponentJsonConverter::Deserialize(const nlohmann::json& a_rootJson, GameObjectModelComponent& a_gameObjectModelComponent) const
{
    if (a_rootJson.is_null()) { return; }

    // UUIDや無効フラグなど、全コンポーネント共通の値は基底クラスに読ませる
    a_gameObjectModelComponent.GameObjectComponentBase::Deserialize(a_rootJson);

    // モデルのファイルパス(AssetFilePathのUUID)を読み込む
    // AssetFilePathのDeserializeは処理関数なので、GetMutableREFで取り出して任せる
    auto& l_modelFilePath = a_gameObjectModelComponent.GetMutableREFModelFilePath();

    l_modelFilePath.Deserialize(a_rootJson.value(k_modelFilePathJsonKey, nlohmann::json{}));

    // ここではモデルを読み込まない
    // 読み込みはPostDeserializeで、ファイルパスとスケルタルかどうかが両方そろってから行う
    a_gameObjectModelComponent.SetIsSkeletal(a_rootJson.value(k_isSkeletalJsonKey, false));
}

nlohmann::json FWK::Converter::GameObjectModelComponentJsonConverter::Serialize(const GameObjectModelComponent& a_gameObjectModelComponent) const
{
    nlohmann::json l_rootJson = a_gameObjectModelComponent.GameObjectComponentBase::Serialize();

    const auto& l_modelFilePath = a_gameObjectModelComponent.GetREFModelFilePath();

    l_rootJson[k_modelFilePathJsonKey] = l_modelFilePath.Serialize               ();
    l_rootJson[k_isSkeletalJsonKey]    = a_gameObjectModelComponent.GetVALIsSkeletal();

    return l_rootJson;
}
```

### GameObject/Component/Model/Inspector/GameObjectModelComponentInspector.h(新規)

```cpp
#pragma once

namespace FWK
{
    class GameObjectModelComponent;
}

namespace FWK
{
    class GameObjectModelComponentInspector final
    {
    public:

         GameObjectModelComponentInspector() = default;
        ~GameObjectModelComponentInspector() = default;

        void EditInspector(GameObjectModelComponent& a_modelComponent) const;

    private:

        static constexpr std::string_view k_modelFilePathLabel = "モデル(FBX)";
        static constexpr std::string_view k_isSkeletalLabel    = "スケルタル(骨で動くモデル)";
    };
}
```

### GameObject/Component/Model/Inspector/GameObjectModelComponentInspector.cpp(新規・写経)

```cpp
#include "GameObjectModelComponentInspector.h"

void FWK::GameObjectModelComponentInspector::EditInspector(GameObjectModelComponent& a_modelComponent) const
{
    // モデルのファイルを、アセットブラウザーからのドラッグ&ドロップで受け取る
    // 受け取ったファイルが変わったときだけ、モデルを読み込み直す
    ImGui::TextUnformatted(k_modelFilePathLabel.data());

    if (auto& l_modelFilePath = a_modelComponent.GetMutableREFModelFilePath();
        l_modelFilePath.EditInspector())
    {
        a_modelComponent.ReloadModel();
    }

    // スケルタルかどうかを切り替える
    // 切り替えたときは、描き方のクラスが変わるため読み込み直す(ApplyIsSkeletalの中で行う)
    bool l_isSkeletal = a_modelComponent.GetVALIsSkeletal();

    if (ImGui::Checkbox(k_isSkeletalLabel.data(), &l_isSkeletal))
    {
        a_modelComponent.ApplyIsSkeletal(l_isSkeletal);
    }
}
```

---

## S0 を設計し直すときの材料(2026-10-11、S4 / S5 から移した)

ユーザー指示(2026-10-11)「ModelComponent はまだ実装しないので設計に加えないで、S4 ~ S6 を実装する」により、
S4 / S5 から ModelComponent に関わる部分を外し、ここへ移した。**この文書の上の部分(S3 より前の DrawRequest の形)も含め、S0 は S6 のビルドの後に設計し直す。**
ここにあるのは、そのときの材料(確定した設計ではない)。

**S4 ~ S6 で、ModelComponent の代わりに Graphics 側にできたもの**

| もの | S0 での使い方 |
|---|---|
| `Graphics::ModelDrawRegistration`(S5) | 描き方(または ModelComponent)がメンバに1つ持つ。モデル・マテリアルが変わったときに `Register(フレームごとのメッシュの番号, Meshletの数, マテリアルのハンドル, メッシュの種類, オブジェクトの番号)` を呼ぶ。破棄で自動解除。Detach では `Unregister` |
| `ModelMaterialFileCreator::CreateDefaultModelMaterialFilePath`(S4) | 今は private の static。スロットで既定の .mat を探すときに public へ移す |
| `ModelMaterial::FetchVALMaterial` / `ModelMaterialSystem::GetREFErrorMaterial` | エラーマテリアルへの切り替えは `ModelDrawRegistration` の中で行うので、コンポーネントは呼ばなくてよい |

## 材料 1 : S5 の登録を使う側(旧 S5 の「ModelComponent の描き方」の変更から)

- 旧 S5 では、描き方の基底(`GameObjectModelComponentRendererBase`)に `RegisterDrawItemList` / `UnregisterDrawItemList` / `FetchVALFrameCount` と
  登録の一覧を持たせ、Static / Skeletal の `Register` がそれぞれ描画項目を作っていた。これらはすべて `ModelDrawRegistration` に入ったので、作らない。
- 描き方が用意するもの(S3 の分と合わせて S0 で書く):
  - オブジェクトのテーブルの番号(`m_objectIndex`)と、メッシュのテーブルの番号(Static は `m_meshIndexList`、Skeletal はフレームごとの `m_frameMeshIndexList`)
  - メッシュごとの Meshlet の数の一覧(`static_cast<std::uint32_t>(l_modelMesh.m_meshletData.m_meshletList.size())`。AS のグループ数への変換は `ModelDrawRegistration` が行う)
  - メッシュごとのマテリアルのハンドルの一覧(`std::vector<Graphics::ModelMaterial>`。スロットからサブメッシュ名で引いて並べる)
- Static は、同じ `m_meshIndexList` をフレームの数だけ並べた一覧を渡す(`std::vector<std::vector<std::uint32_t>>(l_frameCount, m_meshIndexList)`。個数と値で作るので `( )`、規約 9-11)。
- 旧 S4-4 の `ApplyMaterialListToRenderer` / `Renderer::ApplyMaterialList` / `Struct::ModelDrawMaterial` は要らなくなる(マテリアルのハンドルを並べて `Register` を呼び直すだけ)。
- 旧 S4-4 の `Struct::GameObjectModelComponentMaterialSlot` は、S4 の骨組みから消した(`GameObjectModelComponentStruct.h` ごと)。S0 で作り直す。

## 材料 2 : 旧 S4-4 ModelComponent のマテリアルのスロット(S4_Material.md から移した、2026-10-11 時点のまま)

> 下のコードは、`Struct::ModelDrawMaterial` があった頃の形のまま。使うときは材料 1 のとおりに直す。
### 目的

- ModelComponent が「サブメッシュ名 → .mat(AssetFilePath)」の対応を持ち、メッシュごとのマテリアルを決める。
- 保存(シーン / プレハブの JSON)は名前と UUID の組。番号は保存しない。今のモデルに無い名前の組も消さずに残す(モデルを差し替えて戻したときに復活する)。
- スロットに何も割り当てていなければ、モデルと同じフォルダの `StandardLit_<サブメッシュ名>.mat` を探して使う。それも無ければエラーマテリアル。
- 描き方(Renderer)は、メッシュごとに「マテリアルの種類(テーブル)と番号」を受け取って、描画項目を作る(S5)。

### ファイル一覧

#### 新規

| ファイル | 内容 |
|---|---|
| `Definition/Struct/GameObject/GameObjectModelComponentStruct.h` | `Struct::GameObjectModelComponentMaterialSlot` / `Struct::ModelDrawMaterial` |

#### 変更

| ファイル | 変更 |
|---|---|
| `GameObject/Component/Model/GameObjectModelComponent.h/.cpp` | スロットの一覧・名前と UUID の対応・スロットを作る処理 |
| `GameObject/Component/Model/Converter/Json/GameObjectModelComponentJsonConverter.h/.cpp` | `"MaterialSlotList"` |
| `GameObject/Component/Model/Inspector/GameObjectModelComponentInspector.h/.cpp` | スロットごとに .mat のボタン |
| `GameObject/Component/Model/Renderer/*` | `FetchVALSubMeshNameList` / `ApplyMaterialList` |

### コード

#### Definition/Struct/GameObject/GameObjectModelComponentStruct.h(新規)

```cpp
#pragma once

namespace FWK::Struct
{
    struct GameObjectModelComponentMaterialSlot final
    {
        std::wstring m_subMeshName = {};

        AssetFilePath m_materialFilePath = {};

        Graphics::ModelMaterial m_material = {};
    };

    struct ModelDrawMaterial final
    {
        TypeAlias::StaticTypeID m_tableStaticTypeID = StaticTypeIDGenerator::k_invalidStaticTypeID;

        std::uint32_t m_tableElementIndex = Graphics::GPUElementTable::k_invalidElementIndex;
    };
}
```

> `GameObjectModelComponentMaterialSlot` は AssetFilePath(クラス)とハンドルを持つので、Framework.h では
> `ModelMaterial.h` と `AssetFilePath.h` より後に include する。`ModelDrawMaterial` は Renderer と ModelComponent の両方が使う。

#### GameObjectModelComponent.h(変更)

```cpp
        void ReloadModel();

        void ApplyMaterialFilePath(const std::size_t& a_slotIndex);

        void ApplyIsSkeletal(const bool a_isSkeletal);

        void SetMaterialAssignmentMap(std::unordered_map<std::wstring, boost::uuids::uuid>&& a_set) { m_materialAssignmentMap = std::move(a_set); }

        void SetIsSkeletal(const bool a_set) { m_isSkeletal = a_set; }

        const auto& GetREFMaterialSlotList() const { return m_materialSlotList; }

        const auto& GetREFMaterialAssignmentMap() const { return m_materialAssignmentMap; }

        const auto& GetREFModelFilePath() const { return m_modelFilePath; }

        auto& GetMutableREFMaterialSlotList() { return m_materialSlotList; }

        auto& GetMutableREFModelFilePath() { return m_modelFilePath; }
```

private:

```cpp
        void BuildMaterialSlotList();

        void ApplyMaterialListToRenderer() const;

        void ApplyWorldMatrixToRenderer() const;

        boost::uuids::uuid FetchVALDefaultMaterialUUID(const std::wstring& a_subMeshName) const;

        std::vector<Struct::GameObjectModelComponentMaterialSlot> m_materialSlotList;

        std::unordered_map<std::wstring, boost::uuids::uuid> m_materialAssignmentMap;

        std::unique_ptr<GameObjectModelComponentRendererBase> m_renderer;
```

#### GameObjectModelComponent.cpp(変更・写経)

`ReloadModel` で、`m_renderer = std::move(l_renderer);` の後に `BuildMaterialSlotList();` と `ApplyMaterialListToRenderer();` を呼ぶ。

```cpp
void FWK::GameObjectModelComponent::ApplyMaterialFilePath(const std::size_t& a_slotIndex)
{
    // インスペクターでスロットの.matを変えたときに呼ばれる
    FWK_ASSERT_RETURN_IF(a_slotIndex >= m_materialSlotList.size(), "スロットの番号が範囲外のため、マテリアルの変更に失敗しました。");

    auto& l_materialSlot = m_materialSlotList[a_slotIndex];

    // 名前とUUIDの対応を書き換える(保存されるのはこの対応)
    m_materialAssignmentMap.insert_or_assign(l_materialSlot.m_subMeshName, l_materialSlot.m_materialFilePath.GetREFAssetFilePathUUID());

    // 新しい.matを読み込む(読み込めなければハンドルは無効のまま → エラーマテリアルで描かれる)
    const auto& l_materialFilePath = l_materialSlot.m_materialFilePath.FetchVALFilePath();

    l_materialSlot.m_material.Load(l_materialFilePath);

    // マテリアルの種類が変わるとパスも変わるため、描き方へ渡し直す(S5で描画項目を作り直す)
    ApplyMaterialListToRenderer();
}

void FWK::GameObjectModelComponent::BuildMaterialSlotList()
{
    // モデルのサブメッシュ名の一覧から、スロットの一覧を作り直す
    // モデルを読み込んだとき(ReloadModel)にだけ呼ばれ、毎フレームは呼ばれない
    m_materialSlotList.clear();

    if (!m_renderer) { return; }

    const auto& l_subMeshNameList = m_renderer->FetchVALSubMeshNameList();

    for (const auto& l_subMeshName : l_subMeshNameList)
    {
        // 同じサブメッシュ名は1つのスロットにまとめる(同じ.matを使う)
        const bool l_hasSlot = std::ranges::any_of(m_materialSlotList,
                                                   [&l_subMeshName](const auto& a_materialSlot)
                                                   {
                                                       return a_materialSlot.m_subMeshName == l_subMeshName;
                                                   });

        if (l_hasSlot) { continue; }

        Struct::GameObjectModelComponentMaterialSlot l_materialSlot = {};

        l_materialSlot.m_subMeshName = l_subMeshName;

        l_materialSlot.m_materialFilePath.SetAllowedType(Enum::AssetFilePathType::ModelMaterial);

        // 1. 保存されていた対応があればそのUUID
        // 2. 無ければ、モデルと同じフォルダの既定の.mat(StandardLit_<名前>.mat)のUUID
        // 3. それも無ければ空のまま(エラーマテリアルで描かれる)
        if (const auto& l_assignmentITR = m_materialAssignmentMap.find(l_subMeshName);
            l_assignmentITR != m_materialAssignmentMap.end())
        {
            l_materialSlot.m_materialFilePath.SetAssetFilePathUUID(l_assignmentITR->second);
        }
        else
        {
            l_materialSlot.m_materialFilePath.SetAssetFilePathUUID(FetchVALDefaultMaterialUUID(l_subMeshName));
        }

        const auto& l_materialFilePath = l_materialSlot.m_materialFilePath.FetchVALFilePath();

        if (!l_materialFilePath.empty())
        {
            l_materialSlot.m_material.Load(l_materialFilePath);
        }

        m_materialSlotList.emplace_back(std::move(l_materialSlot));
    }
}

void FWK::GameObjectModelComponent::ApplyMaterialListToRenderer() const
{
    if (!m_renderer) { return; }

    const auto& l_graphicsManager     = Graphics::GraphicsManager::GetInstance       ();
    const auto& l_resourceContext     = l_graphicsManager.GetREFResourceContext      ();
    const auto& l_modelMaterialSystem = l_resourceContext.GetREFModelMaterialSystem  ();
    const auto& l_errorMaterial       = l_modelMaterialSystem.GetREFErrorMaterial    ();
    const auto& l_subMeshNameList     = m_renderer->FetchVALSubMeshNameList          ();

    // メッシュごとに「マテリアルの種類(どのテーブルか = GPUデータの型のStaticTypeID)と、テーブルの何番目か」を並べる
    // 例 : メッシュ0("Body") → StandardLitの3番 / メッシュ1("Face") → StandardLitの4番
    std::vector<Struct::ModelDrawMaterial> l_drawMaterialList = {};

    l_drawMaterialList.reserve(l_subMeshNameList.size());

    for (const auto& l_subMeshName : l_subMeshNameList)
    {
        const auto& l_materialSlotITR = std::ranges::find_if(m_materialSlotList,
                                                             [&l_subMeshName](const auto& a_materialSlot)
                                                             {
                                                                 return a_materialSlot.m_subMeshName == l_subMeshName;
                                                             });

        // スロットのマテリアルが無い(読み込めなかった)ときは、エラーマテリアルにする
        auto l_material = l_errorMaterial;

        if (l_materialSlotITR != m_materialSlotList.end())
        {
            if (const auto& l_slotMaterial = l_materialSlotITR->m_material.FetchVALMaterial();
                l_slotMaterial)
            {
                l_material = l_slotMaterial;
            }
        }

        Struct::ModelDrawMaterial l_drawMaterial = {};

        if (l_material)
        {
            // マテリアルの種類は、GPUデータの型のStaticTypeIDで渡す(S5で、どの描画項目の一覧に入れるかを決めるのに使う)
            const auto& l_tableINFO = l_material->FetchREFTableINFO();

            l_drawMaterial.m_tableStaticTypeID = l_tableINFO.k_typeINFO->k_staticTypeID;
            l_drawMaterial.m_tableElementIndex = l_material->GetVALTableElementIndex();
        }

        l_drawMaterialList.emplace_back(l_drawMaterial);
    }

    m_renderer->ApplyMaterialList(l_drawMaterialList);
}

boost::uuids::uuid FWK::GameObjectModelComponent::FetchVALDefaultMaterialUUID(const std::wstring& a_subMeshName) const
{
    // モデルと同じフォルダにある既定の.mat(StandardLit_<名前>.mat)を、RegistryからUUIDで探す
    const auto& l_modelFilePath    = m_modelFilePath.FetchVALFilePath                                      ();
    const auto& l_materialFilePath = Graphics::ModelMaterialFileCreator::CreateDefaultModelMaterialFilePath(l_modelFilePath, a_subMeshName);

    const auto& l_application           = Application::GetInstance                 ();
    const auto& l_assetFilePathRegistry = l_application.GetREFAssetFilePathRegistry();

    if (const auto* l_assetUUID = l_assetFilePathRegistry.FindPTRAssetUUID(l_materialFilePath);
        l_assetUUID)
    {
        return *l_assetUUID;
    }

    return {};
}
```

> `l_materialSlot.m_materialFilePath.SetAllowedType(...)` のように、構造体のメンバ(クラス)の Set を呼ぶのは規約 11-11 の対象外(自作の関数の戻り値ではなく、変数のメンバ)。
> 禁止文字の置き換えは `CreateDefaultModelMaterialFilePath` の中で行うので、作る側と探す側で名前がずれない。

#### Renderer(Static / Skeletal)への追加

```cpp
        virtual std::vector<std::wstring> FetchVALSubMeshNameList() const = 0;

        virtual void ApplyMaterialList(const std::vector<Struct::ModelDrawMaterial>& a_drawMaterialList) = 0;
```

- `FetchVALSubMeshNameList` : Record のメッシュの `m_subMeshName` を、メッシュの順に並べて返す。
- `ApplyMaterialList` : メッシュごとのマテリアルを `std::vector<Struct::ModelDrawMaterial> m_drawMaterialList` に覚える。
  S5 で、これを使って描画項目(どのパスに、どの番号で)を作る。

#### JsonConverter(変更)

```json
"MaterialSlotList": [
    { "SubMeshName": "Body", "MaterialFilePath": { "AssetFilePathUUID": "..." } },
    { "SubMeshName": "Face", "MaterialFilePath": { "AssetFilePathUUID": "..." } }
]
```

- Deserialize : 配列を読んで `std::unordered_map<std::wstring, boost::uuids::uuid>` を作り、`SetMaterialAssignmentMap(std::move(...))`。
- Serialize : `GetREFMaterialAssignmentMap()` を全部書く(今のモデルに無い名前の組も残す)。
- サブメッシュ名は wstring なので、JSON へは UTF-8 の std::string に変換して書く(`Utility::WStringToString` / `StringToWString`)。

#### インスペクター(変更)

スロットごとに、サブメッシュ名のラベルと .mat の AssetFilePath のボタンを出す。ボタンへドロップして変わったら `ApplyMaterialFilePath(スロットの番号)`。

```cpp
    auto& l_materialSlotList = a_modelComponent.GetMutableREFMaterialSlotList();

    for (std::size_t l_slotIndex = 0ULL; l_slotIndex < l_materialSlotList.size(); ++l_slotIndex)
    {
        auto& l_materialSlot = l_materialSlotList[l_slotIndex];

        ImGui::TextUnformatted(Utility::WStringToString(l_materialSlot.m_subMeshName).c_str());

        if (l_materialSlot.m_materialFilePath.EditInspector())
        {
            a_modelComponent.ApplyMaterialFilePath(l_slotIndex);
        }
    }
```

> ここでは `GetMutableREFMaterialSlotList` を使ってよい(AssetFilePath の EditInspector = 子のオブジェクトの処理関数に任せる、規約 6-4)。

## テスト用のベンチマークのコンポーネントの登録(こちらが行う)

`Source/Framework/GameObject/Component/Benchmark/` のファイルは作成済み(写経しない)。S0 の骨組みを書くときに、次も一緒に行う。

- `GameObjectComponentTaggedFactoryConstant.h` に `inline constexpr std::string_view k_gameObjectComponentTagBenchmark = "ベンチマーク(テスト用)";`
- vcxproj / filters に4ファイルとフィルター `Source\Framework\GameObject\Component\Benchmark` / `...\Benchmark\Inspector` を登録
- Framework.h の GameObjectModelComponent の後に `Inspector/GameObjectBenchmarkComponentInspector.h` → `GameObjectBenchmarkComponent.h`
- 消すときは `00_Overview.md` の「テスト用に作ったもの」の表のとおりに消す

## 動作の確認

1. FBX を Asset フォルダに置く。
2. GameObject を作り、インスペクターの「コンポーネントを追加」→「モデル」。
3. アセットブラウザーから FBX を「モデル(FBX)」のボタンへドロップする。→ モデルが描かれる。
4. 骨で動く FBX なら「スケルタル」にチェック → 最初のモーションが再生される。
5. コンポーネントを削除 → 消える。Undo → また描かれる。GameObject を削除 → 消える。Undo → 描かれる。
6. シーンを保存して開き直す → 同じモデルが描かれる。

## 次のステップへのつながり

- S1 で、この状態(モデルを何体か置いたとき)の CPU・GPU の時間を測る。
- S3 で `ApplyWorldMatrix` は「行列が変わったときだけオブジェクトのテーブルへ書く」形に変わる。
- S5 で `Register` / `Unregister` は、ModelRenderSystem への描画項目の登録・解除に変わる(Attach / Detach の仕組みはそのまま使う)。
