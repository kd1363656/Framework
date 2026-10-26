#include "SkeletalAnimationSkinningSelector.h"
#include "../../../../Application/Application.h"

void FWK::Graphics::SkeletalAnimationSkinningSelector::AddDrawRequest(const std::shared_ptr<Struct::SkeletalAnimationModelAutoSkinningDrawRequestData>& a_drawRequestData)
{
    FWK_ASSERT_RETURN_IF(!a_drawRequestData, "DrawRequestDataが無効のため、描画申請の追加が出来ませんでした。");

    Entry l_entry = {};

    l_entry.m_drawRequestData = a_drawRequestData;

    m_entryList.emplace_back(l_entry);
}

void FWK::Graphics::SkeletalAnimationSkinningSelector::Update(RenderGraph& a_renderGraph, const float a_frameResourceWaitSecond)
{
    if (m_entryList.empty()) { return; }

    TargetRequestSet l_targetRequestSet = {};

    FWK_ASSERT_RETURN_IF(!FindTargetRequestSet(a_renderGraph, l_targetRequestSet), "SkeletalAnimationModelの描画申請が見つからないため、描画方式を選べません。");

    RemoveExpiredEntry(l_targetRequestSet);

    const auto& l_application   = Application::GetInstance             ();
    const auto& l_fpsController = l_application.GetREFFPSController    ();
    const float l_deltaTime     = l_fpsController.GetVALScaledDeltaTime();

    // 1. 従来の方式で描画する数を決める(姿勢を書き換える物は、数に関係なく必ず従来の方式にする)
    std::size_t l_runtimePoseRequiredCount = k_initialCount;

    for (const auto& l_entry : m_entryList)
    {
        const auto& l_drawRequestData = l_entry.m_drawRequestData.lock();

        if (l_drawRequestData &&
            IsRuntimePoseRequired(*l_drawRequestData))
        {
            ++l_runtimePoseRequiredCount;
        }
    }

    const std::size_t l_standardSkinningCount = std::min(m_entryList.size(), std::max(l_runtimePoseRequiredCount, m_standardSkinningCapacity));
    const std::size_t l_optionalStandardCount = l_standardSkinningCount - l_runtimePoseRequiredCount;

    // 2. 従来の方式の物の姿勢を計算する(かかった時間を測り、次のフレームの数を決めるのに使う)
    //    姿勢を書き換える物以外は、登録した順に従来の方式にする
    std::size_t l_optionalIndex = k_initialCount;

    const auto l_poseEvaluationBeginTime = std::chrono::steady_clock::now();

    for (auto& l_entry : m_entryList)
    {
        const auto& l_drawRequestData = l_entry.m_drawRequestData.lock();

        if (!l_drawRequestData) { continue; }

        if (!IsRuntimePoseRequired(*l_drawRequestData))
        {
            if (l_optionalIndex >= l_optionalStandardCount)
            {
                ++l_optionalIndex;

                continue;
            }

            ++l_optionalIndex;
        }

        ApplyStandardSkinning(l_targetRequestSet,
                              *l_drawRequestData,
                              l_deltaTime,
                              l_entry);
    }

    const std::chrono::duration<float> l_poseEvaluationSecond = std::chrono::steady_clock::now() - l_poseEvaluationBeginTime;

    // 3. 残りはベイクの方式にする
    l_optionalIndex = k_initialCount;

    for (auto& l_entry : m_entryList)
    {
        const auto& l_drawRequestData = l_entry.m_drawRequestData.lock();

        if (!l_drawRequestData)                         { continue; }
        if (IsRuntimePoseRequired(*l_drawRequestData))  { continue; }

        const bool l_isStandardSkinning = l_optionalIndex < l_optionalStandardCount;

        ++l_optionalIndex;

        if (l_isStandardSkinning) { continue; }

        ApplyGPUSkinning(l_targetRequestSet,
                         *l_drawRequestData,
                         l_deltaTime,
                         l_entry);
    }

    UpdateStandardSkinningCapacity(l_poseEvaluationSecond.count(), l_standardSkinningCount, a_frameResourceWaitSecond);
}

bool FWK::Graphics::SkeletalAnimationSkinningSelector::FindTargetRequestSet(RenderGraph& a_renderGraph, TargetRequestSet& a_targetRequestSet) const
{
    a_targetRequestSet.m_computeRequest              = a_renderGraph.FindVALComputeRequestPerObject<SkeletalAnimationPerObjectComputeRequest>                       ().lock();
    a_targetRequestSet.m_standardLitDrawRequest      = a_renderGraph.FindVALDrawRequestPerObject   <SkeletalAnimationModelStandardLitPerObjectDrawRequest>           ().lock();
    a_targetRequestSet.m_standardUnLitDrawRequest    = a_renderGraph.FindVALDrawRequestPerObject   <SkeletalAnimationModelStandardUnLitPerObjectDrawRequest>         ().lock();
    a_targetRequestSet.m_standardShadowDrawRequest   = a_renderGraph.FindVALDrawRequestPerObject   <SkeletalAnimationModelCascadeShadowPerObjectDrawRequest>         ().lock();
    a_targetRequestSet.m_gpuSkinnedLitDrawRequest    = a_renderGraph.FindVALDrawRequestPerObject   <SkeletalAnimationModelGPUSkinnedLitPerObjectDrawRequest>         ().lock();
    a_targetRequestSet.m_gpuSkinnedShadowDrawRequest = a_renderGraph.FindVALDrawRequestPerObject   <SkeletalAnimationModelGPUSkinnedCascadeShadowPerObjectDrawRequest>().lock();

    return a_targetRequestSet.m_computeRequest            &&
           a_targetRequestSet.m_standardLitDrawRequest    &&
           a_targetRequestSet.m_standardUnLitDrawRequest  &&
           a_targetRequestSet.m_standardShadowDrawRequest &&
           a_targetRequestSet.m_gpuSkinnedLitDrawRequest  &&
           a_targetRequestSet.m_gpuSkinnedShadowDrawRequest;
}

void FWK::Graphics::SkeletalAnimationSkinningSelector::RemoveExpiredEntry(const TargetRequestSet& a_targetRequestSet)
{
    std::erase_if(m_entryList, [&a_targetRequestSet](const Entry& a_entry)
    {
        if (!a_entry.m_drawRequestData.expired()) { return false; }

        // PlayerはComputeRequestへshared_ptrから登録しているため、描画申請が破棄されてもPlayerが生きていれば外す
        if (a_entry.m_standardDrawRequestData)
        {
            a_targetRequestSet.m_computeRequest->RemoveComputeRequest(a_entry.m_standardDrawRequestData->m_skeletalAnimationPlayer);
        }

        return true;
    });
}

bool FWK::Graphics::SkeletalAnimationSkinningSelector::IsRuntimePoseRequired(const Struct::SkeletalAnimationModelAutoSkinningDrawRequestData& a_drawRequestData) const
{
    if (a_drawRequestData.m_isRuntimePoseRequired) { return true; }

    const auto& l_skeletalAnimationPlayer = a_drawRequestData.m_skeletalAnimationPlayer.lock();

    // 揺れものはその場で計算した姿勢を揺らすため、ベイクの方式(事前計算した姿勢の表)では描画できない
    return l_skeletalAnimationPlayer &&
           l_skeletalAnimationPlayer->IsRuntimePoseRequired();
}

void FWK::Graphics::SkeletalAnimationSkinningSelector::ApplyStandardSkinning(const TargetRequestSet&                                       a_targetRequestSet,
                                                                             const Struct::SkeletalAnimationModelAutoSkinningDrawRequestData& a_drawRequestData,
                                                                             const float                                                     a_deltaTime,
                                                                                   Entry&                                                    a_entry) const
{
    const auto& l_skeletalAnimationPlayer = a_drawRequestData.m_skeletalAnimationPlayer.lock();

    if (!l_skeletalAnimationPlayer) { return; }

    // ベイクの方式から外す(描画申請側はweak_ptrで持っているため、ここで破棄すれば描画対象から外れる)
    a_entry.m_gpuSkinnedDrawRequestData.reset();

    // 初めて従来の方式になったフレームだけ、各描画申請へ登録する
    if (!a_entry.m_standardDrawRequestData)
    {
        a_entry.m_standardDrawRequestData = std::make_shared<Struct::SkeletalAnimationModelPerObjectDrawRequestData>();

        a_entry.m_standardDrawRequestData->m_skeletalAnimationPlayer = l_skeletalAnimationPlayer;

        a_targetRequestSet.m_computeRequest->AddComputeRequest        (l_skeletalAnimationPlayer);
        a_targetRequestSet.m_standardLitDrawRequest->AddDrawRequest   (a_entry.m_standardDrawRequestData);
        a_targetRequestSet.m_standardUnLitDrawRequest->AddDrawRequest (a_entry.m_standardDrawRequestData);
        a_targetRequestSet.m_standardShadowDrawRequest->AddDrawRequest(a_entry.m_standardDrawRequestData);
    }

    a_entry.m_standardDrawRequestData->m_worldMatrix                 = a_drawRequestData.m_worldMatrix;
    a_entry.m_standardDrawRequestData->m_worldInverseTransposeMatrix = a_drawRequestData.m_worldInverseTransposeMatrix;

    // 再生時刻を進め、その時刻の姿勢をCPUで計算する(揺れものはワールド座標で計算するため、ワールド行列も渡す)
    l_skeletalAnimationPlayer->AdvanceTime(a_deltaTime, a_drawRequestData.m_worldMatrix);
}
void FWK::Graphics::SkeletalAnimationSkinningSelector::ApplyGPUSkinning(const TargetRequestSet&                                       a_targetRequestSet,
                                                                        const Struct::SkeletalAnimationModelAutoSkinningDrawRequestData& a_drawRequestData,
                                                                        const float                                                     a_deltaTime,
                                                                              Entry&                                                    a_entry) const
{
    const auto& l_skeletalAnimationPlayer = a_drawRequestData.m_skeletalAnimationPlayer.lock();

    if (!l_skeletalAnimationPlayer) { return; }

    // 従来の方式から外す(ComputeShaderでの頂点の変形もやめる)
    if (a_entry.m_standardDrawRequestData)
    {
        a_targetRequestSet.m_computeRequest->RemoveComputeRequest(l_skeletalAnimationPlayer);

        a_entry.m_standardDrawRequestData.reset();
    }

    // 初めてベイクの方式になったフレームだけ、各描画申請へ登録する
    if (!a_entry.m_gpuSkinnedDrawRequestData)
    {
        a_entry.m_gpuSkinnedDrawRequestData = std::make_shared<Struct::SkeletalAnimationModelGPUSkinnedDrawRequestData>();

        a_entry.m_gpuSkinnedDrawRequestData->m_skeletalAnimationModelRecord = l_skeletalAnimationPlayer->GetREFSkeletalAnimationModelRecord();

        a_targetRequestSet.m_gpuSkinnedLitDrawRequest->AddDrawRequest   (a_entry.m_gpuSkinnedDrawRequestData);
        a_targetRequestSet.m_gpuSkinnedShadowDrawRequest->AddDrawRequest(a_entry.m_gpuSkinnedDrawRequestData);
    }

    // 姿勢は計算せず、再生時刻だけを進める(何コマ目を使うかは描画申請側がこの時刻から決める)
    l_skeletalAnimationPlayer->AdvancePlaybackTime(a_deltaTime);

    const auto& l_animation = l_skeletalAnimationPlayer->GetREFAnimation();

    auto& l_gpuSkinnedDrawRequestData = *a_entry.m_gpuSkinnedDrawRequestData;

    l_gpuSkinnedDrawRequestData.m_worldMatrix                 = a_drawRequestData.m_worldMatrix;
    l_gpuSkinnedDrawRequestData.m_worldInverseTransposeMatrix = a_drawRequestData.m_worldInverseTransposeMatrix;
    l_gpuSkinnedDrawRequestData.m_motionIndex                 = l_animation.m_motionIndex;
    l_gpuSkinnedDrawRequestData.m_animationTimeSecond         = l_skeletalAnimationPlayer->GetVALAnimationTimeSecond();

    // ウェイトが変わったときだけ写す(毎フレームの配列のコピーを避ける)
    if (l_gpuSkinnedDrawRequestData.m_morphWeightList != a_drawRequestData.m_morphWeightList)
    {
        l_gpuSkinnedDrawRequestData.m_morphWeightList = a_drawRequestData.m_morphWeightList;
    }
}

void FWK::Graphics::SkeletalAnimationSkinningSelector::UpdateStandardSkinningCapacity(const float a_poseEvaluationSecond, const std::size_t a_standardSkinningCount, const float a_frameResourceWaitSecond)
{
    if (a_standardSkinningCount == k_initialCount) { return; }

    // 1体あたりの姿勢の計算時間を、前のフレームまでの値と少しずつ混ぜて求める
    const float l_poseEvaluationSecondPerInstance = a_poseEvaluationSecond / static_cast<float>(a_standardSkinningCount);

    m_averagePoseEvaluationSecond = m_averagePoseEvaluationSecond == k_unmeasuredPoseEvaluationSecond ? l_poseEvaluationSecondPerInstance : std::lerp(m_averagePoseEvaluationSecond, l_poseEvaluationSecondPerInstance, k_poseEvaluationSecondSmoothingRatio);

    if (m_averagePoseEvaluationSecond <= k_unmeasuredPoseEvaluationSecond) { return; }

    // CPUの予算の中で、姿勢を計算できる数
    const float       l_poseEvaluationBudgetSecond = k_targetFrameSecond * k_poseEvaluationBudgetRatio;
    const std::size_t l_cpuCapacity                = std::max(static_cast<std::size_t>(l_poseEvaluationBudgetSecond / m_averagePoseEvaluationSecond), k_minStandardSkinningCapacity);

    // CPUの予算を超えた場合は、フレームが遅れないようにすぐに減らす
    if (l_cpuCapacity < m_standardSkinningCapacity)
    {
        m_standardSkinningCapacity = l_cpuCapacity;

        return;
    }

    // GPUが間に合っていない間は増やさない
    // (従来の方式はComputeShaderで全頂点を変形するため、増やすとGPUの仕事も増える)
    if (a_frameResourceWaitSecond > k_targetFrameSecond * k_gpuBoundWaitRatio) { return; }

    // CPUにもGPUにも余裕があれば、1フレームに少しずつ増やす(一度に増やすと、そのフレームだけ重くなるため)
    m_standardSkinningCapacity = std::min(l_cpuCapacity, m_standardSkinningCapacity + k_standardSkinningCapacityGrowStep);
}