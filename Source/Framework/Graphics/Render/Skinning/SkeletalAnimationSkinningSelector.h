#pragma once

namespace FWK::Graphics
{
    class RenderGraph;
}

namespace FWK::Graphics
{
    // SkeletalAnimationModelを「従来の方式」と「ベイクの方式」のどちらで描画するかを、負荷を見て自動で選ぶクラス
    //
    // 従来の方式(SkeletalAnimationPlayer + SkeletalAnimationComputePass)
    //   CPUが毎フレーム、ボーンの姿勢を計算する(IK、揺れものなど、姿勢をその場で書き換えられる)
    //   GPUはComputeShaderで全頂点を1回だけ変形し、カメラ、影の全てのViewで使い回す
    //   → CPUの負荷が1体ごとに増える。姿勢は正確(コマの間も補間、モーションのブレンドもできる)
    //
    // ベイクの方式(SkeletalAnimationModelGPUSkinned*PerObjectDrawRequest)
    //   読み込み時に計算した姿勢の表から「何コマ目か」を選ぶだけで、CPUは姿勢を計算しない
    //   GPUはメッシュシェーダーが、見えている頂点だけをViewごとにその場で変形する
    //   → 何体でもCPUの負荷はほぼ0。姿勢は1秒30コマの表の値になり、その場で書き換えられない
    //
    // 選び方
    //   ・m_isRuntimePoseRequiredがtrueの描画申請(IKなど)と、揺れもの(.spring)が有効なモデルは、必ず従来の方式にする
    //   ・それ以外は、CPUに余裕がある数(m_standardSkinningCapacity)までを従来の方式、残りをベイクの方式にする
    //   ・従来の方式の数は、実際に姿勢の計算にかかった時間から毎フレーム決め直す
    //     CPUの予算を超えたらすぐに減らし、CPUとGPUの両方に余裕があるときだけ少しずつ増やす
    //
    // 使い方
    //   AddDrawRequestで描画申請を登録し、SkeletalAnimationPlayer::AdvanceTimeは呼ばない
    //   (再生時刻を進めるのもこのクラスが行う。ベイクの方式の物は姿勢の計算を省くため)
    class SkeletalAnimationSkinningSelector final
    {
    private:

        // 描画申請1つ分と、今使っている方式へ渡している描画申請
        // 使っていない方式の描画申請はnullptrにする(描画申請側はweak_ptrで持つため、nullptrにすれば描画対象から外れる)
        struct Entry final
        {
            std::weak_ptr<Struct::SkeletalAnimationModelAutoSkinningDrawRequestData> m_drawRequestData = {};

            std::shared_ptr<Struct::SkeletalAnimationModelPerObjectDrawRequestData>  m_standardDrawRequestData   = nullptr;
            std::shared_ptr<Struct::SkeletalAnimationModelGPUSkinnedDrawRequestData> m_gpuSkinnedDrawRequestData = nullptr;
        };

        // 描画申請を渡す先(RenderGraphから毎フレーム探す)
        struct TargetRequestSet final
        {
            std::shared_ptr<SkeletalAnimationPerObjectComputeRequest>                          m_computeRequest                 = nullptr;
            std::shared_ptr<SkeletalAnimationModelStandardLitPerObjectDrawRequest>             m_standardLitDrawRequest         = nullptr;
            std::shared_ptr<SkeletalAnimationModelStandardUnLitPerObjectDrawRequest>           m_standardUnLitDrawRequest       = nullptr;
            std::shared_ptr<SkeletalAnimationModelRitchToonPerObjectDrawRequest>               m_standardRitchToonDrawRequest   = nullptr;
            std::shared_ptr<SkeletalAnimationModelCascadeShadowPerObjectDrawRequest>           m_standardShadowDrawRequest      = nullptr;
            std::shared_ptr<SkeletalAnimationModelGPUSkinnedLitPerObjectDrawRequest>           m_gpuSkinnedLitDrawRequest       = nullptr;
            std::shared_ptr<SkeletalAnimationModelGPUSkinnedRitchToonPerObjectDrawRequest>     m_gpuSkinnedRitchToonDrawRequest = nullptr;
            std::shared_ptr<SkeletalAnimationModelGPUSkinnedCascadeShadowPerObjectDrawRequest> m_gpuSkinnedShadowDrawRequest    = nullptr;
        };

    public:

         SkeletalAnimationSkinningSelector() = default;
        ~SkeletalAnimationSkinningSelector() = default;

        void AddDrawRequest(const std::shared_ptr<Struct::SkeletalAnimationModelAutoSkinningDrawRequestData>& a_drawRequestData);

        // 1フレームに1回、RenderGraphのBeginFrameより前に呼ぶ
        // a_frameResourceWaitSecond : このフレームでGPUの処理が終わるのをCPUが待った時間(長いほどGPUが間に合っていない)
        void Update(RenderGraph& a_renderGraph, const float a_frameResourceWaitSecond);

        auto GetVALStandardSkinningCapacity() const { return m_standardSkinningCapacity; }

    private:

        bool FindTargetRequestSet(RenderGraph& a_renderGraph, TargetRequestSet& a_targetRequestSet) const;

        void RemoveExpiredEntry(const TargetRequestSet& a_targetRequestSet);

        // 姿勢をその場で書き換えるため、必ず従来の方式で描画する描画申請か
        bool IsRuntimePoseRequired(const Struct::SkeletalAnimationModelAutoSkinningDrawRequestData& a_drawRequestData) const;

        // 従来の方式で描画する(ベイクの方式から外し、姿勢を計算する)
        void ApplyStandardSkinning(const TargetRequestSet&                                       a_targetRequestSet,
                                   const Struct::SkeletalAnimationModelAutoSkinningDrawRequestData& a_drawRequestData,
                                   const float                                                     a_deltaTime,
                                         Entry&                                                    a_entry) const;

        // ベイクの方式で描画する(従来の方式から外し、再生時刻だけを進める)
        void ApplyGPUSkinning(const TargetRequestSet&                                       a_targetRequestSet,
                              const Struct::SkeletalAnimationModelAutoSkinningDrawRequestData& a_drawRequestData,
                              const float                                                     a_deltaTime,
                                    Entry&                                                    a_entry) const;

        // 実際にかかった姿勢の計算時間から、次のフレームで従来の方式にする数を決める
        void UpdateStandardSkinningCapacity(const float a_poseEvaluationSecond, const std::size_t a_standardSkinningCount, const float a_frameResourceWaitSecond);

        // 1秒60フレームのときの1フレームの時間
        static constexpr float k_targetFrameSecond = 1.0F / 60.0F;

        // 1フレームのうち、従来の方式の姿勢の計算に使ってよい時間の割合
        static constexpr float k_poseEvaluationBudgetRatio = 0.15F;

        // CPUがGPUを待った時間が1フレームのこの割合を超えたら、GPUが間に合っていないとみなす
        static constexpr float k_gpuBoundWaitRatio = 0.25F;

        // 1体あたりの姿勢の計算時間を、前のフレームまでの値と混ぜる割合(1フレームだけ遅くても数が大きく変わらないようにする)
        static constexpr float k_poseEvaluationSecondSmoothingRatio = 0.1F;

        static constexpr float k_unmeasuredPoseEvaluationSecond = 0.0F;

        // 計測を続けるため、少なくともこの数は従来の方式にする
        static constexpr std::size_t k_minStandardSkinningCapacity      = 1ULL;
        static constexpr std::size_t k_initialStandardSkinningCapacity  = 16ULL;
        static constexpr std::size_t k_standardSkinningCapacityGrowStep = 1ULL;
        static constexpr std::size_t k_initialCount                     = 0ULL;

        std::vector<Entry> m_entryList = {};

        float m_averagePoseEvaluationSecond = k_unmeasuredPoseEvaluationSecond;

        std::size_t m_standardSkinningCapacity = k_initialStandardSkinningCapacity;
    };
}