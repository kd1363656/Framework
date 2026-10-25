#pragma once

namespace FWK::Graphics
{
    // スケルタルアニメーションを、事前に「スキニング行列の表」へ計算しておくクラス(アニメーションのベイク)
    //
    // 従来 : モデル1体ごとに毎フレームCPUでボーンの姿勢を計算し、ComputeShaderで全頂点を変形して
    //        「変形後の頂点バッファ」をモデル1体ごとに持っていた(500体では数GBのメモリが必要になる)
    //
    // 今回 : 読み込み時に、全モーションを1秒に30コマずつ計算し、
    //        「コマ番号 × ボーンパレット」のスキニング行列を1本のBufferへまとめてGPUに置く
    //        描画時は「何コマ目か」だけを決め、メッシュシェーダーが見えている頂点だけをその場で変形する
    //        → モデル1体ごとのメモリは不要になり、CPUの計算も「コマ番号を決める」だけになる
    //
    // スキニング行列の番号 = コマ番号 × 1コマ分の行列数(全Meshのパレット数の合計) + Meshの開始位置 + パレット番号
    //
    // モーフ(ブレンドシェイプ)のウェイトも同じコマ番号でベイクする
    // ウェイトの番号 = コマ番号 × チャンネル数 + チャンネル番号
    // (モーションにウェイトのアニメーションが無いチャンネルは、エディターで設定した既定のウェイトを使う)
    class SkeletalAnimationBakedSkinning final
    {
    private:

        // チャンネルごとの、ウェイトの最小値と最大値
        struct MorphChannelWeightRange final
        {
            float m_minWeight = Constant::k_defaultModelMorphTargetMINWeight;
            float m_maxWeight = Constant::k_defaultModelMorphTargetMAXWeight;
        };

    public:

         SkeletalAnimationBakedSkinning() = default;
        ~SkeletalAnimationBakedSkinning() = default;

        SkeletalAnimationBakedSkinning(const SkeletalAnimationBakedSkinning&) = delete;
        SkeletalAnimationBakedSkinning(      SkeletalAnimationBakedSkinning&&) = delete;

        SkeletalAnimationBakedSkinning& operator=(const SkeletalAnimationBakedSkinning&) = delete;
        SkeletalAnimationBakedSkinning& operator=(      SkeletalAnimationBakedSkinning&&) = delete;

        // モデルの全モーションをベイクし、GPU用のBufferを作成する
        bool Create(SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord);

        // ベイクした行列とモーフのウェイトをGPU用のBufferへコピーする命令を積む(最初の1回だけ行う)
        void UploadIfNeeded(const DirectCommandList& a_directCommandList);

        // チャンネルのウェイトを、エディターで設定したウェイトの範囲へ収める(インスタンスごとにウェイトを上書きするときに使う)
        float ClampMorphWeight(const std::uint32_t a_channelIndex, const float a_weight) const;

        // モーション番号と再生時刻から、全モーションを通したコマ番号を求める(ループ再生)
        std::uint32_t FetchVALFrameIndex(const std::uint32_t a_motionIndex, const float a_animationTimeSecond) const;

        const auto& GetREFSkinningInfo() const { return m_skinningInfo; }

        bool IsUploaded() const { return m_isUploaded; }

    private:

        void BakeMotionFrame(const SkeletalAnimationModelRecord::ModelData& a_modelData, const std::vector<TypeAlias::Math::Matrix>& a_globalBoneMatrixList, const std::uint32_t a_frameIndex);

        // 全コマのモーフのウェイトを求める(モーフを持たないモデルでは何もしない)
        void BakeMorphWeight(const SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord);

        // キーフレームから、指定した時刻のウェイトを求める(キーフレームの間は直線で補間する)
        float SampleMorphWeight(const std::vector<Struct::ModelMorphWeightKeyFrame>& a_keyFrameList, const float a_timeSecond) const;

        // チャンネルごとのウェイトの範囲を、エディターの設定から決める
        void BuildMorphChannelWeightRangeList(const SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord);

        // Mesh1つの頂点が、モーフで動く最大の距離(全てのチャンネルをウェイトの範囲の端まで効かせたとき)
        float CalculateMaxMorphMoveDistance(const Struct::ModelMorphData& a_modelMorphData) const;

        // ベイクしたデータを入れるGPU用のBufferと、そこへコピーするためのUploadBufferを作成する
        // ※ このクラスの.cppの中だけで使うため、定義は.cppに書いている
        template <typename Type>
        bool CreateBakedBuffer(const std::vector<Type>& a_bakedDataList, DynamicRWStructuredBuffer& a_buffer, UploadBuffer& a_uploadBuffer) const;

        // UploadBufferの中身をGPU用のBufferへコピーし、シェーダーから読める状態にする命令を積む
        void CopyBakedBuffer(const DirectCommandList& a_directCommandList, const UINT64 a_byteSize, DynamicRWStructuredBuffer& a_buffer, const UploadBuffer& a_uploadBuffer) const;

        static constexpr std::uint32_t k_singleFrameCount  = 1U;
        static constexpr std::uint32_t k_initialFrameCount = 0U;
        static constexpr std::uint32_t k_firstMotionIndex  = 0U;

        static constexpr float k_initialMoveDistance = 0.0F;
        static constexpr float k_emptyMorphWeight    = 0.0F;
        static constexpr float k_interpolationScale  = 1.0F;

        static constexpr std::size_t k_singleKeyFrameCount = 1ULL;

        static constexpr UINT64 k_bufferStartOffset = 0ULL;

        static constexpr bool k_isBlending = false;

        std::vector<std::uint32_t> m_motionFrameOffsetList = {};
        std::vector<std::uint32_t> m_motionFrameCountList  = {};

        // ベイクした行列(GPUへコピーした後は不要なので解放する)
        std::vector<TypeAlias::Math::Matrix> m_bakedSkinningMatrixList = {};

        // ベイクしたモーフのウェイト(GPUへコピーした後は不要なので解放する)
        std::vector<float> m_bakedMorphWeightList = {};

        std::vector<float> m_maxMoveDistanceList = {};

        std::vector<MorphChannelWeightRange> m_morphChannelWeightRangeList = {};

        Struct::ModelGPUDrivenSkinningInfo m_skinningInfo = {};

        DynamicRWStructuredBuffer m_skinningMatrixBuffer = {};
        DynamicRWStructuredBuffer m_morphWeightBuffer    = {};

        UploadBuffer m_uploadBuffer            = {};
        UploadBuffer m_morphWeightUploadBuffer = {};

        UINT64 m_skinningMatrixByteSize = k_bufferStartOffset;
        UINT64 m_morphWeightByteSize    = k_bufferStartOffset;

        bool m_isUploaded = false;
    };
}