# 実装計画(目標まで)

2026-10-10 作成。新しいセッションでも、この文書とメモリー(`dx12-optimization-roadmap.md` / `shakyo-scaffold-workflow.md`)を読めば続きから進められるようにしてある。

## 目標

1. DirectX12 の描画を高速化する(値が変わったときだけ GPU へ送る / ExecuteIndirect でまとめて投げる)。
2. 途中でマテリアル(.mat、ハンドル式、種類ごとの GPU テーブル)を作り直す。
3. 統合レンダラー(ModelComponent)で Static / Skeletal を描く。
4. 揺れもの(SpringBone)。
5. トゥーン(段階陰影 → MatCap → アウトライン)。

## 進め方(ユーザーとの約束)

- ステップに入ると決めたら、**処理の中身以外**(フィルター・vcxproj・Framework.h・.h / .cpp・クラス・構造体・定数・Enum・Concept・Factory の TypeAlias・コンストラクタ / デストラクタ・メンバ・必要なゲッター / セッター・空の関数定義・HLSL の骨組み・CONFIG)はこちらがファイルへ書く。
- 関数の中身は、この計画の各ステップに書いた「写経コード」をチャットへ貼り、1つずつ解説する。
- ビルドは毎回しない。**ExecuteIndirect(S6)が終わったとき**と、**揺れもの(P3)が終わったとき**にこちらでビルドする。
  - S1 の後にも1回だけビルドし、高速化する前の速さ(基準値)を測っておく(テストとして許可済み)。S3 ~ S5 の間はシェーダーと C++ の書き換えの途中なので、ビルドは通らない。
- DirectX12 の用語・API・構造体は、毎回その場で説明する(ユーザーは DirectX12 の素人)。
- コメントは「何をするか・なぜか」を初学者向けに書く。変更履歴は書かない。ヘッダーの関数の外側には書かない。
- ユーザーの案より良い案があれば、数値例で比べてこちらを優先する。

## 現状(2026-10-10 夜)

- マテリアル回り(旧 A2-3 / A1-1 / A1-2 / A1-3-1 / A2-4-1)はユーザーが取り下げた。メッシュは `Struct::StaticModelMesh`(旧 `Struct::ModelMaterial` 入り)のまま。
- スムーズ法線(`ModelSmoothedNormalBuilder.h`、頂点の `m_smoothedNormal`)は残っている。
- **描画申請(`AddDrawRequest`)を出す側がどこにも無い。** 今のままではモデルは1体も描かれず、速さも測れない。
- **コンポーネントの削除・GameObject の破棄を、コンポーネントへ知らせる仕組みが無い。** Undo のためにコマンドが実体(shared_ptr)を持ち続けるので、「登録は1回だけ」にすると、消したはずのモデルが描かれ続ける。

## 順番の変更(2026-10-10 夜、ユーザー指示)

- **ベンチマークはしない。** 「テスト用に作ったもの」の表とベンチマークの節は、S6 の後に4ファイルとタグを消すだけでよい(登録はしていない)。
- **S0(ModelComponent)は S6(ExecuteIndirect)の後に作る。** 理由 : 描画の仕組みを先に作らないと、コンポーネントの設計が分からなくなる。
  - 進める順番 : S2 → S3 → S4 → S5 → S6 → S0。S1(プロファイラー)は計測専用なので後回し。
  - S3 / S5 の「ModelComponent の StaticRenderer / SkeletalRenderer の変更」は、S0 を作るときにまとめて書く(S3 の骨組みでは書かない)。
  - 描画項目(ModelDrawItem)を出す側が無い間は、モデルは描かれない(ビルドも S6 の後)。

## ステップ一覧

### フェーズ1(高速化 + マテリアル)… 最後にビルド

| ステップ | 内容 | 文書 |
|---|---|---|
| S0 | ModelComponent(最小)。描画申請を出す側を作る。コンポーネントの Attach / Detach。AssetFilePathType::Model | `S0_ModelComponent.md` |
| S1 | GPU の計測(タイムスタンプクエリ)と CPU の計測、プロファイラーのウィンドウ | `S1_GPUProfiler.md` |
| S2 | GPUElementTable(変わった要素だけ GPU へ送るテーブル)。オブジェクト・メッシュ・マテリアルで使い回す | `S2_GPUElementTable.md` |
| S3 | ModelRenderSystem(オブジェクトのテーブル・メッシュのテーブル)。CBModelPerObject をやめてルート定数にする | `S3_ModelRenderSystem.md` |
| S4 | マテリアル(クラスと .mat / System とハンドル / GPU テーブル / サブメッシュ名と取り込み時の .mat / ModelComponent のスロット) | `S4_Material.md` |
| S5 | 描画の登録(状態が変わった時だけ描画項目を作る。パスは項目を回すだけ) | `S5_DrawRegistration.md` |
| S6 | ExecuteIndirect(コマンドシグネチャ・引数のバッファ)… **ここでビルド** | `S6_ExecuteIndirect.md` |

### フェーズ2(揺れもの + トゥーン)… 設計は `P_Phase2.md`

| ステップ | 内容 |
|---|---|
| P1 | GPU でのカリングと、画面外・静止キャラのスキニングの間引き(ExecuteIndirect のカウントバッファ) |
| P2 | SkeletalPose + PoseModifier(ポーズを書き換える処理の一覧) |
| P3 | 揺れもの(SpringBone + 球 / カプセルのコライダー)… **ここでビルド** |
| P4 | エディター(マテリアル・ModelComponent・揺れものの表示) |
| P5 | トゥーン(段階陰影 → MatCap → アウトライン) |

フェーズ2の写経コードは、S6 のビルドが通った時点のコードに合わせて作る(理由は `P_Phase2.md` の冒頭)。

## 計画を作るときに見つけて、設計に入れたこと

- テーブル(GPUElementTable)と描画項目の一覧は Renderer が `shared_ptr` で持ち、使う側(描き方・マテリアル)は `weak_ptr` で持つ。
  アプリの終了時に Renderer が先に破棄されても、壊れたメモリに触らない(撤回した設計の終了時アサートの原因への対策)。
- `Struct::ModelDrawItem`(描画項目)を、ExecuteIndirect の1件(ルート定数 12 バイト + DispatchMesh の引数 12 バイト)と同じ並びにした。S6 は memcpy だけで済む。
- Static と Skeletal が同じ `.asset` をキャッシュにしていたため、スケルタルの切り替えのたびに FBX を読み直していた。S4-3 で `.staticModel` / `.skeletalModel` に分ける。
- Static と Skeletal の AS は中身が同じだったので、S3 で `Model/Model_AS.hlsl` の1つにまとめる。
- テーブルのコピーは `Renderer::BeginFrame` ではなく、RenderGraph のパス `ModelRenderTableUploadPass`(実行レイヤー `Upload` = 一番前)で行う(2026-10-10)。
  RenderGraph が並べる順番・S1 のプロファイラー・P1 のカリングのパスとの前後が、すべて実行レイヤーで決まる。S5 / S6 の設計は変わらない。
  コピーはダイレクトコマンドリストに積む(コピーキューにする案は撤回、2026-10-11)。
- GraphicsCONFIG.json の `ModelPerObjectDynamicConstantBufferUploader`(300000 × 256 バイト × 3 フレーム ≒ 220MB の UPLOAD)は S5 で不要になる。

## テーブルの種類を登録マクロにした改訂(2026-10-10 夜、ユーザー指示)

- ユーザー指示 : 「クラスを足すと増えていく if / switch は、FWK_TYPE_INFO のようなマクロを用意して、定義した場所に書く形にする」(規約 18-9)。
- S3 の写経の後に、テーブルの GPU データ(オブジェクト・メッシュ・マテリアル)をクラスにして、`FWK_DEFINE_MODEL_RENDER_TABLE_INFO` /
  `FWK_DEFINE_MODEL_MATERIAL_RENDER_TABLE_INFO` で登録する形に変えた。骨組み(マクロ・レジストリ・GPU データのクラス・登録・CONFIG)は書き換え済み。
- その結果、消えた「種類ごとの分岐」:
  - S3 : `Enum::ModelRenderTableType` と `FetchVALElementByteStride` の switch
  - S4 : マテリアルの `FetchVALTableType`(enum を返す)→ `FetchREFTableINFO`(GPU データの型の情報を返す1行)
  - S5 : `Enum::ModelRenderPassType` と、描き方の「マテリアル → パス」の switch(一覧は「メッシュの種類 × マテリアルのテーブル」で自動で作る)
  - S6 : 「影のパスか」でルートシグネチャを選ぶ分岐(コマンドシグネチャは、RCModelDrawItem を持つルートシグネチャごとに自動で作る)
- 写経し直すところは、S3 の文書の冒頭の表。

## 設計の決まり(2026-10-10 追加)

- **設計としておかしい継承はしない**(規約 7-13)。「派生は基底の一種」が成り立つときだけ継承し、使わない仮想関数を意味のない中身で派生に書かせない。
  - 計画の見直しで直した例 : S4 の `ModelMaterialRecord` に、何も解放しない `ReserveRelease` を書かせていた
    → `AssetRecordBase` から `ReserveRelease` を外し、`AssetStorage` が Concept で見分ける形にした(規約 18-8)。
- **テスト専用のものは作ってよいが、必ず消す前提で最小構成にする。** 作ったらこの下の表に書き、目的を果たしたら消して、表からも消す。
  できるだけコードを書かずに済ませる(エディターの複製・テスト用のシーンの JSON など)。

### テスト用に作ったもの(必ず消す)

| もの | 目的 | 消す時期 |
|---|---|---|
| `Source/Framework/GameObject/Component/Benchmark/`(`GameObjectBenchmarkComponent.h/.cpp` と `Inspector/GameObjectBenchmarkComponentInspector.h/.cpp`) | 速さの比較。指定したモデルを格子状に N 体並べ、指定した割合だけ毎フレーム動かす(シーンに入れないので保存されない) | フェーズ1の比較(S1 の基準値 → S6 の後)が終わったら |
| `GameObjectComponentTaggedFactoryConstant.h` の `k_gameObjectComponentTagBenchmark` の1行 | 上のコンポーネントを「コンポーネントを追加」に出すためのタグ | 同上 |
| vcxproj / filters / Framework.h の上の4ファイルの登録(フィルター `...\Component\Benchmark` / `...\Benchmark\Inspector`) | 同上 | 同上 |

> - ベンチマークのコンポーネントは写経しない(ユーザーの指示で、こちらが書いて比較に使う)。2026-10-10 にファイルだけ作成済み。
> - S0 の `GameObjectModelComponent` と `GameObject::ApplyIsInScene` を使うので、**vcxproj / filters / Framework.h への登録とタグの定数は、S0 の骨組みを書くときに一緒に行う**(それまではビルドに含めない)。
> - 比べ方 : 同じシーン・同じカメラで「並べる数」(100 / 1000 / 4096)と「動かす割合」(0 / 0.1 / 1.0)を変え、プロファイラーの CPU「描画命令の記録」と GPU の各パスの平均を記録する。結果はこのファイルの下に表で残す。

## ベンチマークの手順(テスト用。終わったら全部消す)

ユーザーの指示(2026-10-10):「ベンチマーク用のコンポーネントを作ってベンチマークし、終わったらコンポーネントとデータを削除する」。
ベンチマーク用のコンポーネントは写経せず、こちらが書く。**S0(ModelComponent・ApplyIsInScene)と S1(プロファイラー)が無いと測れない**ため、次の順で行う。

| 時期 | こちらがすること |
|---|---|
| 2026-10-10 | `GameObjectBenchmarkComponent` のファイルだけ作成済み(ビルドには未登録) |
| S0 の骨組みを書くとき | ベンチマークの4ファイルとタグの定数を登録する(`S0_ModelComponent.md` の末尾) |
| **S1 の写経が終わったとき** | ① 下の「自動計測モード」をベンチマークのコンポーネントに足す ② 計測用シーン(JSON)を作る ③ ビルドする ④ こちらで exe を起動して計測し、結果をこのファイルの「ベンチマークの結果」に書く(**高速化の前の基準値**) |
| **S6 のビルドが通ったとき** | 同じシーン・同じ条件でもう一度計測し、結果を並べて書く。ユーザーに比較を報告する |
| 比較を報告した後 | ベンチマークのコンポーネント・タグの定数・登録・計測用シーン・結果のファイル(CSV)をすべて消す。このファイルの「テスト用に作ったもの」の表とこの節も消す。メモリーの記述も消す |

### 自動計測モード(S1 の後に、こちらが足す)

こちらはアプリの画面を操作できないので、起動するだけで測って終わる形にする。テスト用なので、すべてベンチマークのコンポーネントの中に入れる(他のクラスは変えない)。

- 計測用シーン `Asset/Data/Scene/Test/Benchmark.json` を作り、ベンチマークのコンポーネントを付けた GameObject とカメラを置く。
  モデルは Asset にある FBX を1つ使う(Registry の UUID をシーンの JSON に書く)。起動するシーンは、計測の間だけ ApplicationCONFIG / 最初に読むシーンを一時的に差し替え、終わったら元に戻す。
- 条件の一覧(並べる数 100 / 1000 / 4096 × 動かす割合 0 / 0.1 / 1.0、Static と Skeletal)を順に実行する。
  1つの条件ごとに : 並べる → 120 フレーム待つ(読み込み・GPU の立ち上がりを除く)→ 300 フレームぶんの CPU / GPU の値(プロファイラーの「今回」)を足して平均を出す。
- 結果を `Benchmark/BenchmarkResult_<時期>.csv`(exe と同じフォルダ)に書き、全条件が終わったら `PostMessage(HWND, WM_CLOSE, ...)` でアプリを閉じる。
- 記録する値 : CPU「シーンの更新」「描画命令の記録」「描画の終了」、GPU「フレーム全体」と各パス(影・Lit・スキニング)。

### ベンチマークの結果

(まだ無い。S1 の後に基準値、S6 の後に比較を書く)

## 順番を変えた理由(元の計画からの変更)

- **S0 を最初にした。** 描画申請を出す側が無いので、先に作らないと計測しても 0 ミリ秒しか出ない。
- **マテリアルは S2 の後(S4)。** S2 で作る「変わった要素だけ送るテーブル」を、S3(オブジェクト・メッシュ)と S4(マテリアル)で続けて2回使うので、仕組みが記憶に残りやすい。
- **カリングとスキニングの間引きはフェーズ2の最初。** ExecuteIndirect の「引数のバッファ」を GPU が詰める形にするため、S6 の後でないと作れない。

## 各ステップの文書の読み方

各ステップの文書は、次の順に書いてある。

1. 目的と、その結果どれだけ速くなるか(数値の例)
2. DirectX12 の解説(そのステップで初めて出てくる用語・API)
3. ファイルの一覧(新規 / 変更)と、vcxproj・filters・Framework.h への登録
4. 各ファイルのコード(ヘッダーは全文、.cpp は全文)。**.cpp の関数の中身と、テンプレートの中身、HLSL の main と関数の中身が写経の対象**
5. CONFIG(GraphicsCONFIG.json など)の変更
6. 動作の確認方法

こちらが骨組みを書くときは、4 の .cpp から関数の中身を抜いた形(戻り値が要る関数は仮の return)をファイルへ書く。
