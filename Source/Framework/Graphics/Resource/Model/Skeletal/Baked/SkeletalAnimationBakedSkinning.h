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
    class SkeletalAnimationBakedSkinning final
    {
    public:

         SkeletalAnimationBakedSkinning() = default;
        ~SkeletalAnimationBakedSkinning() = default;

        SkeletalAnimationBakedSkinning(const SkeletalAnimationBakedSkinning&) = delete;
        SkeletalAnimationBakedSkinning(      SkeletalAnimationBakedSkinning&&) = delete;

        SkeletalAnimationBakedSkinning& operator=(const SkeletalAnimationBakedSkinning&) = delete;
        SkeletalAnimationBakedSkinning& operator=(      SkeletalAnimationBakedSkinning&&) = delete;

        // モデルの全モーションをベイクし、GPU用のBufferを作成する
        bool Create(SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord);

        // ベイクした行列をGPU用のBufferへコピーする命令を積む(最初の1回だけ行う)
        void UploadIfNeeded(const DirectCommandList& a_directCommandList);

        // モーション番号と再生時刻から、全モーションを通したコマ番号を求める(ループ再生)
        std::uint32_t FetchVALFrameIndex(const std::uint32_t a_motionIndex, const float a_animationTimeSecond) const;

        const auto& GetREFSkinningInfo() const { return m_skinningInfo; }

        bool IsUploaded() const { return m_isUploaded; }

        // 1秒あたりのコマ数
        static constexpr float k_bakeFrameRate = 30.0F;

    private:

        void BakeMotionFrame(const SkeletalAnimationModelRecord::ModelData& a_modelData, const std::vector<TypeAlias::Math::Matrix>& a_globalBoneMatrixList, const std::uint32_t a_frameIndex);

        static constexpr std::uint32_t k_singleFrameCount  = 1U;
        static constexpr std::uint32_t k_initialFrameCount = 0U;
        static constexpr std::uint32_t k_firstMotionIndex  = 0U;

        static constexpr float k_initialMoveDistance = 0.0F;

        static constexpr UINT64 k_bufferStartOffset = 0ULL;

        // ベイクでは1つのモーションだけを計算するため、モーション同士の混ぜ合わせ(ブレンド)はしない
        static constexpr bool k_isBlending = false;

        // モーションごとの、全モーションを通したコマ番号の開始位置とコマ数
        std::vector<std::uint32_t> m_motionFrameOffsetList = {};
        std::vector<std::uint32_t> m_motionFrameCountList  = {};

        // ベイクした行列(GPUへコピーした後は不要なので解放する)
        std::vector<TypeAlias::Math::Matrix> m_bakedSkinningMatrixList = {};

        // Meshごとの「アニメーションで中心が動く最大距離」(カリング用の球を広げるために使う)
        std::vector<float> m_maxMoveDistanceList = {};

        Struct::ModelGPUDrivenSkinningInfo m_skinningInfo = {};

        // GPUが読むスキニング行列のBufferと、そこへコピーするためのUploadBuffer
        DynamicRWStructuredBuffer m_skinningMatrixBuffer = {};

        UploadBuffer m_uploadBuffer = {};

        bool m_isUploaded = false;
    };
}