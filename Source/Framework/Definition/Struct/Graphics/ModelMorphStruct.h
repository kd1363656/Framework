#pragma once

namespace FWK::Struct
{
    // モーフ(ブレンドシェイプ)で使用する構造体
    //
    // モーフとは、元の形の頂点に「移動量 × ウェイト(0～1)」を足して形を変える仕組み
    // 例 : 「笑顔」ターゲットのウェイトを1にすると、口の頂点が笑顔の位置まで動く
    //
    // 頂点ごとの移動量は「頂点番号で引ける形(CSR : Compressed Sparse Row)」で持つ
    //   ModelMorphVertexRange(頂点数と同じ数) : その頂点の移動量が、移動量リストの何番目から何個あるか
    //   ModelMorphVertexDelta(影響のある分だけ) : 移動量本体と、どのターゲットの移動量か
    // こうすると、ターゲットの影響を受けない頂点は「0個」と分かるだけで済み、メモリも計算も少なくなる

    // ModelMesh1つが持つモーフターゲット1つ分の情報
    struct ModelMorphTarget final
    {
        std::wstring m_targetName = {};

        Enum::ModelMorphLODProtection m_lodProtection = Enum::ModelMorphLODProtection::Normal;
    };

    // 頂点1つが受けるモーフターゲット1つ分の移動量
    // ※ モデルのバイナリーファイルにこの並びのまま保存するため、メンバを変えたらバージョンを上げる
    struct ModelMorphVertexDelta final
    {
        TypeAlias::Math::Vector3 m_deltaPosition = TypeAlias::Math::Vector3::Zero;

        std::uint32_t m_targetIndex = Constant::k_invalidModelMorphTargetIndex;

        TypeAlias::Math::Vector3 m_deltaNormal = TypeAlias::Math::Vector3::Zero;

        std::uint32_t m_padding = Constant::k_initialModelMorphVertexDeltaPadding;
    };

    // 頂点1つが持つ移動量の範囲(移動量リストの何番目から何個か)
    // ※ モデルのバイナリーファイルにこの並びのまま保存するため、メンバを変えたらバージョンを上げる
    struct ModelMorphVertexRange final
    {
        std::uint32_t m_deltaOffset = Constant::k_initialModelMorphVertexDeltaOffset;
        std::uint32_t m_deltaCount  = Constant::k_initialModelMorphVertexDeltaCount;
    };

    // FBXの頂点(FBX内の頂点番号)1つが受ける移動量
    // FBXから読み込んだ直後だけ使い、ModelMorphVertexDeltaへ変換した後は捨てる
    struct ModelMorphSourceDelta final
    {
        TypeAlias::Math::Vector3 m_deltaPosition = TypeAlias::Math::Vector3::Zero;
        TypeAlias::Math::Vector3 m_deltaNormal   = TypeAlias::Math::Vector3::Zero;

        std::uint32_t m_sourceVertexIndex = Constant::k_invalidModelMorphSourceVertexIndex;
    };

    // ModelMesh1つ分のモーフのデータ
    struct ModelMorphData final
    {
         ModelMorphData() = default;
        ~ModelMorphData() = default;

        ModelMorphData(const ModelMorphData&)           = delete;
        ModelMorphData(      ModelMorphData&&) noexcept = default;

        ModelMorphData& operator=(const ModelMorphData&)           = delete;
        ModelMorphData& operator=(      ModelMorphData&&) noexcept = default;

        std::vector<ModelMorphTarget> m_targetList = {};

        // m_vertexRangeListはモーフを持たないMeshでは空、持つMeshでは頂点数と同じ数になる
        std::vector<ModelMorphVertexRange> m_vertexRangeList = {};
        std::vector<ModelMorphVertexDelta> m_vertexDeltaList = {};

        // LODごと・ターゲットごとの「そのターゲットをウェイト1で効かせたときの崩れ」(モデルのローカル座標での距離)
        // 番号 = LOD番号 × ターゲット数 + ターゲット番号(LOD0は全て0、要素数 = LOD数 × ターゲット数)
        std::vector<float> m_lodTargetMorphErrorList = {};

        // ここから下はFBXから作るときだけ使い、バイナリーファイルには保存しない
        // m_sourceVertexIndexList : 頂点ごとの「FBX内の頂点番号」(頂点の重複削除や並び替えに合わせて一緒に並び替える)
        // m_sourceDeltaListList   : ターゲットごとの、FBX内の頂点番号で表した移動量(m_targetListと同じ番号)
        std::vector<std::uint32_t>                      m_sourceVertexIndexList = {};
        std::vector<std::vector<ModelMorphSourceDelta>> m_sourceDeltaListList   = {};

        // 読み込んだ後に作り、バイナリーファイルには保存しない
        // m_targetChannelIndexList : ターゲットごとの、モデル全体のチャンネル番号(m_targetListと同じ番号)
        //                            チャンネルは同じ名前のターゲットをモデル全体で1つにまとめた物で、ウェイトはチャンネルごとに持つ
        std::vector<std::uint32_t> m_targetChannelIndexList = {};
    };

    // モーフのウェイトのキーフレーム1つ分
    // ※ モデルのバイナリーファイルにこの並びのまま保存するため、メンバを変えたらバージョンを上げる
    struct ModelMorphWeightKeyFrame final
    {
        float m_timeSecond = Constant::k_initialModelMorphWeightTimeSecond;
        float m_weight     = Constant::k_initialModelMorphWeight;
    };

    // モーション1つの中で、モーフターゲット1つのウェイトが時間でどう変わるか
    // ターゲットは名前で指す(同じ名前のターゲットを持つ全Meshに同じウェイトを使う)
    struct ModelMorphWeightTrack final
    {
        std::vector<ModelMorphWeightKeyFrame> m_keyFrameList = {};

        std::wstring m_targetName = {};
    };

    // LODを作るときに使う、モーフの情報(ModelMorphLODEvaluatorが作る)
    struct ModelMorphLODData final
    {
        // m_vertexLockList : 頂点ごとのmeshopt_SimplifyVertex_*の組み合わせ(LOD保護が「固定」のターゲットで動く頂点だけLock)
        // ※ 消えた頂点に一番近いLODの三角形を探すためのデータは、Struct::ModelLODSurfaceSearchDataに分けている
        std::vector<unsigned char> m_vertexLockList = {};

        bool m_hasLockVertex = false;
    };
}