# フェーズ2(GPU カリング・ポーズ・揺れもの・エディター・トゥーン)の設計

## なぜフェーズ2の写経コードを、今はまだ書かないか(こちらの提案)

ユーザーの希望は「すべての目標ステップまでのコードを先に考えておく」だった。フェーズ1(S0 ~ S6)は全コードを書いたが、
フェーズ2は **設計(ファイル・クラス・関数・処理の流れ・DirectX12 の要点)までにして、写経コードは S6 のビルドが通った直後に書く**ことを提案する。

- フェーズ2のコードは、フェーズ1で作るクラスの名前・関数・引数(ModelRenderSystem / ModelDrawItemList / 描き方 / マテリアル)を、そのまま呼ぶ。
- フェーズ1は約 70 ファイルで、写経のときにユーザーが手直しする(過去の実績: 引数の型・置き場所・揃え方を毎回数か所直している)。
  その手直しを真似てから書くほうが、フェーズ2で書き直す量が減る(規約とメモリー `mimic-user-edits` のとおり)。
- S6 のビルドで見つかるコンパイルエラー(API の細かい違い)も、フェーズ2の前に直しておける。
- 書く量は同じで、書く時期を「S6 のビルドの直後」にずらすだけ。写経を待たせることはない。

> 設計はここで決めておくので、写経の順番・ファイルの置き場所は変わらない。

## 順番とビルド

| ステップ | 内容 | ビルド |
|---|---|---|
| P1 | GPU でのカリング(コンピュートシェーダーが見えるものだけを ExecuteIndirect の引数へ詰める)と、スキニングの間引き | |
| P2 | SkeletalPose + PoseModifier(ポーズを書き換える処理の一覧) | |
| P3 | 揺れもの(SpringBone + 球 / カプセルのコライダー) | **ここでビルド** |
| P4 | エディター(マテリアルの編集・ModelComponent・揺れものの表示) | |
| P5 | トゥーン(段階陰影 → MatCap → アウトライン) | 最後にビルド |

---

## P1 GPU でのカリング(GPU 駆動の描画)

### 目的

- 今のカリングは Meshlet 単位(AS の中)だけ。画面外のオブジェクトでも、AS は起動される(グループの数だけ GPU が動く)。
- オブジェクト単位で「画面に入っているか」をコンピュートシェーダーで調べ、入っている描画項目だけを引数のバッファへ詰める。
- 詰めた数を「カウントバッファ」に書き、ExecuteIndirect にカウントバッファを渡す。CPU は数を知らないまま、見えるものだけが描かれる。
- 影は、カスケードごとに別の視錐台で同じことをする。

### DirectX12 の要点

- **UAV(アンオーダードアクセスビュー)** : シェーダーから書き込めるビュー。詰めた引数のバッファとカウントバッファは UAV で書く。
- **`InterlockedAdd`** : 複数のスレッドが同時に同じ数を増やしても、正しく1ずつ増える(書き込み位置の確保に使う)。
- **カウントバッファ** : `ExecuteIndirect(..., countBuffer, countBufferOffset)`。GPU が `min(最大件数, カウントの値)` 件だけ実行する。
- **状態の遷移** : 詰めるときは `UNORDERED_ACCESS`、ExecuteIndirect で読むときは `INDIRECT_ARGUMENT`。カウントは毎フレーム 0 に戻す(`CopyBufferRegion` で 0 を書く、または `ClearUnorderedAccessViewUint`)。
- コンピュートは **ダイレクトキューの上で** 動かす(描画の直前に、同じコマンドリストで)。コンピュートキューに分けるとフェンスの待ち合わせが増えるため。
- 詰めた引数のバッファ・カウントバッファの状態の遷移は、今の RenderGraph ではバッファを扱わないので、カリングのパスと描画のパスが自分で張る。
  このとき、バッファも RenderGraph に宣言させる(`ReadBuffer` / `WriteBuffer`)形にするかを決める(S3 の「テーブルのコピーを RenderGraph のパスにする」)。

### オブジェクトの境界球

- オブジェクトのテーブル(`ModelObjectGPUData`)に、ワールド空間の境界球(中心 + 半径)を足す(152 バイトになる)。
- 境界球は、モデルを読み込んだときにメッシュの Meshlet の境界から1回だけ作り、行列が変わったときに一緒に書く。
- Skeletal は、骨で動いて形が変わるので、バインドポーズの境界球を少し大きめ(拡大率 1.5 など、GraphicsCONFIG.json で指定)にする。

### ファイル(予定)

| ファイル | 内容 |
|---|---|
| `Graphics/Render/Model/Culling/ModelGPUCullingSystem.h/.cpp` | パスごとの「詰めた引数のバッファ(DEFAULT + UAV)」と「カウントバッファ」を持つ |
| `Graphics/Render/Graph/Pass/Model/Culling/ModelGPUCullingPass.h/.cpp` | 描画パスの前に、コンピュートで詰める(実行レイヤーを `ModelCulling` として `Animation` と `Shadow` の間に新設。S3 の `Upload` の後なので、コピー済みのオブジェクトのテーブルを読める) |
| `Shader/Model/Culling/ModelGPUCulling_CS.hlsl` / `ModelGPUCulling.hlsli` | 1 スレッド = 1 描画項目。境界球と視錐台の判定 → 見えれば `InterlockedAdd` で位置を確保して写す |
| `Definition/Struct/Graphics/Buffer/Constant/CBModelGPUCullingPassStruct.h` | 視錐台の6平面・件数 |
| `ModelDrawItemList` の変更 | 「全件の引数のバッファ(UPLOAD、今のもの)」を、コンピュートの入力(SRV)にする |

### スキニングの間引き

- `SkeletalAnimationComputePass` は今、登録された全プレイヤーをスキニングしている。
- P1 では、**前のフレームで1つでも描かれた(カウントされた)キャラクターだけ**スキニングする。
  - GPU のカリング結果を READBACK で CPU へ戻し(S1 のタイムスタンプと同じ仕組み)、数フレーム遅れの「見えた」情報で間引く。
  - 1 フレーム遅れで画面に入った瞬間に古いポーズになるのを防ぐため、画面の外側に少し余裕を持たせた視錐台で判定する。
- アニメーションが止まっている(ポーズが変わらない)キャラも、ポーズの版(番号)が変わらなければスキニングしない。

---

## P2 SkeletalPose と PoseModifier

### 目的

揺れもの(P3)・後の IK などが「アニメーションで作ったポーズを書き換える」ための土台。

```
フレームの流れ
1. アニメーション    : モーションからローカルの姿勢(骨ごとの位置・回転・拡大)を作る          → SkeletalPose(ローカル)
2. PoseModifier の一覧 : 揺れもの・IK などが、順番にローカルの姿勢を書き換える               → SkeletalPose(ローカル)
3. グローバルの行列    : 親から子へ掛けていき、骨ごとのモデル空間の行列を作る                  → SkeletalPose(グローバル)
4. 骨の行列のアップロード → スキニング(コンピュート) → Meshlet の境界の更新 → 描画
```

### 設計

| クラス | 役割 |
|---|---|
| `Graphics::SkeletalPose` | 骨ごとのローカルの姿勢(`Struct::SkeletalPoseLocalTransform` の一覧)と、グローバルの行列の一覧を持つ。`CalculateGlobalMatrixList(骨の親子)` |
| `Graphics::SkeletalAnimationPoseEvaluator`(変更) | 出力を「グローバルの行列」から「ローカルの姿勢(SkeletalPose)」に変える(グローバルの計算は SkeletalPose へ移す) |
| `Graphics::SkeletalPoseModifierBase` | `virtual void ModifyPose(const Struct::SkeletalAnimationModelData&, const float a_deltaTime, SkeletalPose&) = 0;` と、実行順(優先度) |
| `Graphics::SkeletalAnimationPlayer`(変更) | `std::vector<std::shared_ptr<SkeletalPoseModifierBase>>` を持ち、AdvanceTime の中で 1 → 2 → 3 を行う |

- PoseModifier は、ModelComponent の Skeletal の描き方が持つ Player に、揺れものコンポーネント(P3)が登録する(登録・解除は Attach / Detach)。
- 骨の名前 → 骨の番号の対応を、Record に1回だけ作る(揺れものの設定は骨の名前で書くため)。

---

## P3 揺れもの(SpringBone)

### 目的

髪・服・胸などを、骨の鎖として物理っぽく揺らす。Unity の SpringBone(VRM の揺れもの)と同じ方式。

### アルゴリズム(ベルレ積分)

骨の鎖の各骨について、骨の先端(子の位置)を点として扱う。

1. 慣性 : `次の位置 = 今の位置 + (今の位置 − 前の位置) × (1 − 抵抗)`
2. 戻る力 : アニメーションのポーズのときの先端の向きへ、硬さの分だけ引き戻す。`+= 元の向き × 硬さ × 経過時間`
3. 重力 : `+= 重力の向き × 重力の強さ × 経過時間`
4. 長さを保つ : 親の骨の根元から、骨の長さの位置に戻す(伸び縮みしない)。
5. コライダー : 球・カプセルの中に入っていたら、表面まで押し出す(半径 = コライダーの半径 + 骨の当たりの半径)。
6. 回転に戻す : 「元の向き → 新しい向き」の回転を、その骨のローカルの回転に掛ける(SkeletalPose を書き換える)。

- 計算は CPU(骨の数は 1 体あたり数十 ~ 百程度で、GPU へ送るより CPU のほうが簡単で速い)。
- 経過時間が大きすぎると暴れるので、固定の時間刻み(例 : 1/60 秒)で、必要な回数だけ繰り返す(最大回数あり)。

### 設計

| クラス | 役割 |
|---|---|
| `GameObjectSpringBoneComponent` | 揺れものの設定(骨の鎖の根元の骨の名前・硬さ・抵抗・重力・当たりの半径・使うコライダーの一覧)。JSON / インスペクター |
| `Graphics::SpringBonePoseModifier` | `SkeletalPoseModifierBase` の派生。上のアルゴリズム。鎖ごとに前の位置・今の位置を持つ |
| `GameObjectSpringBoneColliderComponent` | 球 / カプセルのコライダー。付けた骨(または GameObject の Transform)に追従する |
| `Struct::SpringBoneChain` / `Struct::SpringBoneJoint` | 鎖と関節のデータ |
| エディターの表示 | EditorDebugRenderer で、コライダーの形と鎖の線を描く(既存の線の描画の仕組みを使う) |

- VRChat 用モデルの揺れの設定(PhysBone)は prefab 側にあり FBX には無いので、**揺れの設定は手で行う**(2026-10-09 の合意)。

---

## P4 エディター

| 対象 | 内容 |
|---|---|
| マテリアル | アセットブラウザーで .mat を選ぶと、Details に値(色・テクスチャ・メタリック・ラフネス)を表示して編集 → `ApplyGPUData` → 保存。種類の変更(StandardLit ↔ UnLit ↔ トゥーン)は共通の値(ベースカラーとテクスチャ)を引き継ぐ |
| .mat の作成 | アセットブラウザーの右クリック → 作成 → マテリアル(種類を選ぶ) |
| ModelComponent | スロットごとに「この GameObject 専用にする」(.mat を複製して割り当て。Clone は入口で1回だけ) |
| 揺れもの | 鎖とコライダーの表示・ON / OFF |
| Undo | マテリアルの値の変更・スロットの変更を Undo できるようにする(Editor/Command/...) |

---

## P5 トゥーン

### 段階陰影(セルシェーディング)

- `ModelToonMaterial`(マテリアルの種類を1つ足す)と、`ModelToonMaterialGPUData`(クラスの一番下に `FWK_DEFINE_MODEL_MATERIAL_RENDER_TABLE_INFO`)。
  2026-10-10 の改訂で、テーブル・描画項目の一覧(Static × Toon / Skeletal × Toon)・コマンドシグネチャは自動で増えるので、enum や switch は書き足さない。
  パスは `RecordDraw<ModelToonMaterialGPUData>(..., Enum::ModelMeshType::Static)` で描く。
- 値 : ベースカラー・影色・影の境目(しきい値)・境目のぼかし幅・影色のテクスチャ・影の出やすさのマスク(テクスチャのチャンネル)。
- PS : `NdotL` を、しきい値とぼかし幅で 2 段(または 3 段)に分ける。カスケードの影も同じ段で扱う。
- マテリアルの値の調整はテクスチャのマスクで行う(頂点カラーは使わない、2026-10-09 のユーザーの決定)。

### MatCap

- 法線をビュー空間へ移し、その xy でテクスチャ(球に映った光沢の絵)を引いて足す。値 : MatCap のテクスチャ・強さ・マスク。

### アウトライン(裏返しの殻)

- トゥーンのマテリアルを使うメッシュを、**もう1回**描くパス(`StaticToonOutline` / `SkeletalToonOutline`)。
- MS で、頂点を **スムーズ法線**(`m_smoothedNormal`、Skeletal はスキニング後のもの)の方向へ押し出す。
- PSO は **前面カリング**(`D3D12_CULL_MODE_FRONT`)にして、押し出した殻の裏側だけを描く。色は単色(マテリアルの値)。
- 線の太さは、カメラからの距離で割って「画面上でほぼ一定」にする(遠くで線が太くなりすぎないため)。マスクのテクスチャで部分的に細くできるようにする。
- アウトラインのパスも、本体のパスと同じ「Static × Toon」の描画項目の一覧を `RecordDraw<ModelToonMaterialGPUData>` で描くだけでよい
  (一覧はマテリアルごとなので、描き方の登録を2つにする必要がない。PSO だけがアウトライン用)。
  アウトラインのルートシグネチャを別にするときも、`RCModelDrawItem` を持たせれば、コマンドシグネチャは ModelRenderSystem が自動で作る(S6)。
- スムーズ法線の限界(マテリアルの境目で割れる)は、このときにモデル全体でグループ化する形に直す(メモリー `smoothed-normal-outline-notes.md`)。
