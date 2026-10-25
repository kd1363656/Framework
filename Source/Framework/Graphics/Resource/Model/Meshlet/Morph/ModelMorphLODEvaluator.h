#pragma once

namespace FWK::Graphics
{
    // LOD(三角形を減らした形)で、モーフ(ブレンドシェイプ)の形がどれだけ崩れるかを測るクラス
    //
    // LODの簡略化は辺を縮めて頂点をまとめるため、移動量が違う隣どうしの頂点がまとめられると、モーフを効かせたときに形が崩れる
    // 崩れの大きさはモーフのウェイトに比例する(ウェイト0なら崩れは0、ウェイト1で最大)
    // そこで次のようにして、「モーフを効かせても崩れが見えないLOD」を描画時に選べるようにする
    //
    // 1. LODを作るたびに、ターゲットごとに「そのターゲットをウェイト1で効かせたときの崩れ」を測って保存する
    //    (ModelMorphData::m_lodTargetMorphErrorList。全ターゲットの最大値はModelMeshLOD::m_morphError)
    //    → 描画時は「形の崩れ + Σ(ターゲットのウェイト × そのターゲットの崩れ)」でLODを選ぶ
    //      効かせているターゲットの分だけで選ぶため、口を開けただけで体まで細かいLODになることがない
    //      モーフを効かせていないモデルは、モーフを持たないモデルと同じ距離でLODを切り替えられる(速さを落とさない)
    // 2. LOD保護が「固定」のターゲットで動く頂点は、簡略化で動かさない(meshopt_SimplifyVertex_Lock)
    //    LOD保護が「重視」のターゲットは、崩れを大きく見積もる(遠くなるまで崩れるLODを使わない)
    //
    // 崩れの測り方
    //   消えた頂点の位置は、LODでは「一番近いLODの三角形の上の点」として描かれ、その点は三角形の3つの角の移動量を補間して動く
    //   そこで消えた頂点ごとに、元の位置に一番近いLODの三角形と、その上の点の重心座標(3つの角の混ぜ具合)を求め、
    //   「消えた頂点自身の移動量」と「3つの角の移動量を重心座標で混ぜた移動量」の差を崩れとする
    //   (以前は周りに残った頂点1つずつとの差の最大値を使っていたが、粗いLODほど離れた頂点と比べるため、崩れを大きく見積もりすぎていた)
    //   一番近いLODの三角形を探す処理は、形のずれを測る処理と同じなのでModelLODSurfaceSearcherで共通にしている
    //
    // ※ 以前は移動量を頂点の属性として簡略化に渡していたが、形の崩れが大きくなって遠くでも細かいLODが選ばれ、
    //   負荷が2倍になったうえに、モーフの崩れもほとんど減らなかったため使わない
    class ModelMorphLODEvaluator final
    {
    private:

        // LOD1段階の崩れを測る間だけ使う作業用のデータ(消えた頂点ごとに作り直さずに使い回す)
        struct LODWorkData final
        {
            Struct::ModelLODSurfaceSearchWorkData m_surfaceSearchWorkData = {};

            std::vector<TypeAlias::Math::Vector3> m_targetDeltaSumList  = {};
            std::vector<bool>                     m_isTouchedTargetList = {};
            std::vector<std::uint32_t>            m_touchedTargetList   = {};
        };

    public:

         ModelMorphLODEvaluator() = default;
        ~ModelMorphLODEvaluator() = default;

        // LODを作る前に1回呼び、簡略化で動かさない頂点の印を作る
        bool Build(const Struct::ModelMorphData& a_modelMorphData, Struct::ModelMorphLODData& a_modelMorphLODData) const;

        // LODで、ターゲットごとに「そのターゲットをウェイト1で効かせたときに起きる崩れ(メートル)」を測り、a_targetMorphErrorListへ書く
        // 戻り値は全ターゲットの中で一番大きい崩れ
        // a_surfaceSearchData : 消えた頂点に一番近いLODの三角形を探すためのデータ(ModelLODSurfaceSearcher::BuildSearchDataで作った物)
        float CalculateLODMorphError(const Struct::ModelMorphData&            a_modelMorphData,
                                     const Struct::ModelLODSurfaceSearchData& a_surfaceSearchData,
                                     const std::vector<std::uint32_t>&        a_lodIndexList,
                                           std::vector<float>&                a_targetMorphErrorList) const;

    private:

        void BuildVertexLockList(const Struct::ModelMorphData& a_modelMorphData, Struct::ModelMorphLODData& a_modelMorphLODData) const;

        // 「消えた頂点の移動量 - 3つの角の移動量を重心座標で混ぜた移動量」の長さをターゲットごとに求め、
        // a_targetMorphErrorListへ足し込む(大きい方を残す)。LOD保護が「重視」のターゲットは大きく見積もる
        void AccumulateInterpolatedDeltaDifference(const Struct::ModelMorphData&       a_modelMorphData,
                                                   const std::vector<std::uint32_t>&   a_lodIndexList,
                                                   const std::uint32_t                 a_removedVertexIndex,
                                                   const Struct::ModelLODSurfacePoint& a_lodSurfacePoint,
                                                         LODWorkData&                  a_lodWorkData,
                                                         std::vector<float>&           a_targetMorphErrorList) const;

        // 頂点の移動量 × a_scaleを、ターゲットごとの合計へ足す
        void AddScaledVertexDelta(const Struct::ModelMorphData& a_modelMorphData, const std::uint32_t a_vertexIndex, const float a_scale, LODWorkData& a_lodWorkData) const;

        // ターゲットのLOD保護から、崩れに掛ける大きさを求める
        float FetchVALTargetScale(const Enum::ModelMorphLODProtection a_lodProtection) const;

        static constexpr std::size_t k_secondCornerOffset = 1ULL;
        static constexpr std::size_t k_thirdCornerOffset  = 2ULL;

        static constexpr float k_normalTargetScale       = 1.0F;
        static constexpr float k_importantTargetScale    = 4.0F;
        static constexpr float k_lockTargetMINDelta      = 1.0e-5F;
        static constexpr float k_emptyMorphError         = 0.0F;
        static constexpr float k_removedVertexDeltaScale = 1.0F;

        static constexpr unsigned char k_emptyVertexLock = 0U;

        ModelLODSurfaceSearcher m_lodSurfaceSearcher = {};
    };
}