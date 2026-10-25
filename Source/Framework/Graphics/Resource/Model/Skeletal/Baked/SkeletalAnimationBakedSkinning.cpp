#include "SkeletalAnimationBakedSkinning.h"

template <typename Type>
bool FWK::Graphics::SkeletalAnimationBakedSkinning::CreateBakedBuffer(const std::vector<Type>& a_bakedDataList, DynamicRWStructuredBuffer& a_buffer, UploadBuffer& a_uploadBuffer) const
{
    auto& l_graphicsManager         = GraphicsManager::GetInstance                          ();
    auto& l_resourceContext         = l_graphicsManager.GetMutableREFResourceContext        ();
    auto& l_cbvSRVUAVDescriptorPool = l_resourceContext.GetMutableREFCBVSRVUAVDescriptorPool();

    const auto& l_device             = l_graphicsManager.GetREFDevice            ();
    const auto& l_gpuMemoryAllocator = l_resourceContext.GetREFGPUMemoryAllocator();

    FWK_ASSERT_RETURN_VALUE_IF(!a_buffer.Create<Type>(l_device,
                                                      l_gpuMemoryAllocator,
                                                      a_bakedDataList.size(),
                                                      l_cbvSRVUAVDescriptorPool),
                                                      "ベイクしたデータ用Bufferの作成に失敗しました。",
                                                      false);

    const std::size_t l_bufferByteSize = sizeof(Type) * a_bakedDataList.size();

    FWK_ASSERT_RETURN_VALUE_IF(!a_uploadBuffer.Create(l_device, l_bufferByteSize), "ベイクしたデータ用UploadBufferの作成に失敗しました。", false);

    auto* const l_mappedData = a_uploadBuffer.FetchPTRMappedData();

    FWK_ASSERT_RETURN_VALUE_IF(!l_mappedData, "ベイクしたデータ用UploadBufferがMapされていません。", false);

    std::memcpy(l_mappedData, a_bakedDataList.data(), l_bufferByteSize);

    return true;
}

bool FWK::Graphics::SkeletalAnimationBakedSkinning::Create(SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord)
{
    auto& l_modelData = a_skeletalAnimationModelRecord.GetMutableREFModelData();

    const auto& l_modelMeshList      = l_modelData.m_modelMeshList;
    const auto& l_modelBoneList      = l_modelData.m_boneList;
    const auto& l_motionSequenceList = l_modelData.m_motionSequenceList;

    FWK_ASSERT_RETURN_VALUE_IF(l_modelMeshList.empty(), "ModelMeshListが空のため、アニメーションのベイクに失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF(l_modelBoneList.empty(), "ModelBoneListが空のため、アニメーションのベイクに失敗しました。", false);

    // ボーンの姿勢を計算するクラス(従来はモデル1体ごとに毎フレーム使っていた物を、ベイクのために1回だけ使う)
    SkeletalAnimationPoseEvaluator l_poseEvaluator = {};

    FWK_ASSERT_RETURN_VALUE_IF(!l_poseEvaluator.Create(l_modelData), "SkeletalAnimationPoseEvaluatorの作成に失敗したため、アニメーションのベイクに失敗しました。", false);

    // 1. Meshごとに、スキニング行列の並びの中での開始位置を決める
    //    (1コマ分の行列は「Mesh0のパレット、Mesh1のパレット…」の順に並べる)
    m_skinningInfo.m_paletteBaseList.clear();

    std::uint32_t l_paletteStride = k_initialFrameCount;

    for (const auto& l_modelMesh : l_modelMeshList)
    {
        m_skinningInfo.m_paletteBaseList.emplace_back(l_paletteStride);

        l_paletteStride += static_cast<std::uint32_t>(l_modelMesh.m_bonePaletteList.size());
    }

    FWK_ASSERT_RETURN_VALUE_IF(l_paletteStride == k_initialFrameCount, "ボーンパレットが存在しないため、アニメーションのベイクに失敗しました。", false);

    // 2. モーションごとのコマ数を決める(モーションが無い場合は、元の姿勢1コマだけにする)
    m_motionFrameOffsetList.clear();
    m_motionFrameCountList.clear ();

    std::uint32_t l_totalFrameCount = k_initialFrameCount;

    if (l_motionSequenceList.empty())
    {
        m_motionFrameOffsetList.emplace_back(l_totalFrameCount);
        m_motionFrameCountList.emplace_back (k_singleFrameCount);

        l_totalFrameCount = k_singleFrameCount;
    }

    for (const auto& l_motionSequence : l_motionSequenceList)
    {
        // 0秒から最後の時刻までを、1秒あたり30コマで区切る(最後の時刻も含めるため+1する)
        const auto l_frameCount = static_cast<std::uint32_t>(std::floor(l_motionSequence.m_durationSecond * Constant::k_skeletalAnimationBakeFrameRate)) + k_singleFrameCount;

        m_motionFrameOffsetList.emplace_back(l_totalFrameCount);
        m_motionFrameCountList.emplace_back (l_frameCount);

        l_totalFrameCount += l_frameCount;
    }

    // 3. 全コマのスキニング行列を計算する
    m_bakedSkinningMatrixList.assign(static_cast<std::size_t>(l_totalFrameCount) * l_paletteStride, TypeAlias::Math::Matrix::Identity);
    m_maxMoveDistanceList.assign    (l_modelMeshList.size(), k_initialMoveDistance);

    m_skinningInfo.m_paletteStride = l_paletteStride;
    m_skinningInfo.m_frameCount    = l_totalFrameCount;

    auto l_globalBoneMatrixList = l_poseEvaluator.GetREFBindPoseGlobalBoneMatrixList();

    if (l_motionSequenceList.empty())
    {
        BakeMotionFrame(l_modelData, l_globalBoneMatrixList, k_initialFrameCount);
    }

    for (std::uint32_t l_motionIndex = 0U; l_motionIndex < static_cast<std::uint32_t>(l_motionSequenceList.size()); ++l_motionIndex)
    {
        const auto& l_motionSequence = l_motionSequenceList[l_motionIndex];

        for (std::uint32_t l_frameIndex = 0U; l_frameIndex < m_motionFrameCountList[l_motionIndex]; ++l_frameIndex)
        {
            // コマの時刻(最後のコマはモーションの長さを超えないようにする)
            const float l_timeSecond = std::min(static_cast<float>(l_frameIndex) / Constant::k_skeletalAnimationBakeFrameRate, l_motionSequence.m_durationSecond);

            // その時刻のボーンの姿勢(モデル全体から見た行列)を求める
            const bool l_isEvaluated = l_poseEvaluator.EvaluatePose(l_modelData,
                                                                    l_timeSecond,
                                                                    Constant::k_initialAnimationTimeSecond,
                                                                    Constant::k_initialAnimationBlendWeight,
                                                                    l_motionIndex,
                                                                    SkeletalAnimationPoseEvaluator::k_invalidMotionIndex,
                                                                    k_isBlending,
                                                                    l_globalBoneMatrixList);

            FWK_ASSERT_RETURN_VALUE_IF(!l_isEvaluated, "ボーンの姿勢の計算に失敗したため、アニメーションのベイクに失敗しました。", false);

            BakeMotionFrame(l_modelData, l_globalBoneMatrixList, m_motionFrameOffsetList[l_motionIndex] + l_frameIndex);
        }
    }

    // 4. 全コマのモーフのウェイトを求める(スキニング行列と同じコマ番号で引けるようにする)
    BuildMorphChannelWeightRangeList(a_skeletalAnimationModelRecord);
    BakeMorphWeight                 (a_skeletalAnimationModelRecord);

    // 5. アニメーションで動いても全頂点を囲めるように、Meshを囲む球の半径を広げる
    //    頂点はどれも「元の球の中」にあり、スキニング行列で動く量は「中心が動く量」以下なので、
    //    元の半径 + 中心が動く最大距離 の球なら、どのコマでも必ず全頂点を囲める
    //    モーフはスキニングの前に頂点を動かすため、モーフで動く最大の距離も足す
    m_skinningInfo.m_boundsRadiusList.clear();

    for (std::size_t l_meshIndex = 0ULL; l_meshIndex < l_modelMeshList.size(); ++l_meshIndex)
    {
        const auto& l_modelMesh = l_modelMeshList[l_meshIndex];

        const float l_maxMorphMoveDistance = CalculateMaxMorphMoveDistance(l_modelMesh.m_modelMorphData);

        m_skinningInfo.m_boundsRadiusList.emplace_back(l_modelMesh.m_modelMeshletData.m_boundsRadius + m_maxMoveDistanceList[l_meshIndex] + l_maxMorphMoveDistance);
    }

    // 6. GPUが読むBufferと、そこへコピーするためのUploadBufferを作成する
    m_skinningMatrixByteSize = sizeof(TypeAlias::Math::Matrix) * m_bakedSkinningMatrixList.size();

    FWK_ASSERT_RETURN_VALUE_IF(!CreateBakedBuffer(m_bakedSkinningMatrixList, m_skinningMatrixBuffer, m_uploadBuffer), "スキニング行列用Bufferの作成に失敗しました。", false);

    m_skinningInfo.m_skinningMatrixBufferSRVDescriptorIndex = m_skinningMatrixBuffer.GetVALSRVDescriptorIndex();

    // モーフを持つモデルだけ、ウェイト用のBufferを作成する
    if (!m_bakedMorphWeightList.empty())
    {
        m_morphWeightByteSize = sizeof(float) * m_bakedMorphWeightList.size();

        FWK_ASSERT_RETURN_VALUE_IF(!CreateBakedBuffer(m_bakedMorphWeightList, m_morphWeightBuffer, m_morphWeightUploadBuffer), "モーフのウェイト用Bufferの作成に失敗しました。", false);

        m_skinningInfo.m_morphWeightBufferSRVDescriptorIndex = m_morphWeightBuffer.GetVALSRVDescriptorIndex();
    }

    return true;
}

void FWK::Graphics::SkeletalAnimationBakedSkinning::UploadIfNeeded(const DirectCommandList& a_directCommandList)
{
    if (m_isUploaded) { return; }

    CopyBakedBuffer(a_directCommandList, m_skinningMatrixByteSize, m_skinningMatrixBuffer, m_uploadBuffer);

    if (m_morphWeightByteSize != k_bufferStartOffset)
    {
        CopyBakedBuffer(a_directCommandList, m_morphWeightByteSize, m_morphWeightBuffer, m_morphWeightUploadBuffer);
    }

    // CPU側の行列とウェイトは、コピー命令を積んだ後は不要なので解放する
    // (UploadBufferはGPUがコピーし終わるまで必要なため残しておく)
    m_bakedSkinningMatrixList.clear        ();
    m_bakedSkinningMatrixList.shrink_to_fit();
    m_bakedMorphWeightList.clear           ();
    m_bakedMorphWeightList.shrink_to_fit   ();

    m_isUploaded = true;
}

float FWK::Graphics::SkeletalAnimationBakedSkinning::ClampMorphWeight(const std::uint32_t a_channelIndex, const float a_weight) const
{
    if (a_channelIndex >= m_morphChannelWeightRangeList.size()) { return a_weight; }

    const auto& l_weightRange = m_morphChannelWeightRangeList[a_channelIndex];

    return std::clamp(a_weight, l_weightRange.m_minWeight, l_weightRange.m_maxWeight);
}

std::uint32_t FWK::Graphics::SkeletalAnimationBakedSkinning::FetchVALFrameIndex(const std::uint32_t a_motionIndex, const float a_animationTimeSecond) const
{
    if (m_motionFrameCountList.empty()) { return k_initialFrameCount; }

    // 範囲外のモーション番号は、最初のモーションとして扱う
    const auto l_motionIndex = a_motionIndex < m_motionFrameCountList.size() ? a_motionIndex : k_firstMotionIndex;
    const auto l_frameCount  = m_motionFrameCountList[l_motionIndex];

    // 再生時刻をコマ番号に直し、モーションの最後まで行ったら最初に戻す(ループ再生)
    const auto l_frameInMotion = static_cast<std::uint32_t>(std::max(a_animationTimeSecond, Constant::k_initialAnimationTimeSecond) * Constant::k_skeletalAnimationBakeFrameRate) % l_frameCount;

    return m_motionFrameOffsetList[l_motionIndex] + l_frameInMotion;
}

void FWK::Graphics::SkeletalAnimationBakedSkinning::BakeMotionFrame(const SkeletalAnimationModelRecord::ModelData& a_modelData, const std::vector<TypeAlias::Math::Matrix>& a_globalBoneMatrixList, const std::uint32_t a_frameIndex)
{
    const auto& l_modelMeshList = a_modelData.m_modelMeshList;

    const auto l_frameMatrixOffset = static_cast<std::size_t>(a_frameIndex) * m_skinningInfo.m_paletteStride;

    for (std::size_t l_meshIndex = 0ULL; l_meshIndex < l_modelMeshList.size(); ++l_meshIndex)
    {
        const auto& l_modelMesh     = l_modelMeshList[l_meshIndex];
        const auto& l_bonePalette   = l_modelMesh.m_bonePaletteList;
        const auto& l_boundsCenter  = l_modelMesh.m_modelMeshletData.m_boundsCenter;
        const auto  l_meshBaseIndex = l_frameMatrixOffset + m_skinningInfo.m_paletteBaseList[l_meshIndex];

        for (std::size_t l_paletteIndex = 0ULL; l_paletteIndex < l_bonePalette.size(); ++l_paletteIndex)
        {
            const auto& l_paletteElement = l_bonePalette[l_paletteIndex];

            if (l_paletteElement.m_boneIndex >= a_globalBoneMatrixList.size()) { continue; }

            // スキニング行列 = 逆バインドポーズ行列 × ボーンのグローバル行列
            // (頂点を「ボーンから見た位置」へ戻してから、今の姿勢のボーンに合わせて動かす)
            const auto l_skinningMatrix = l_paletteElement.m_inverseBindPoseMatrix * a_globalBoneMatrixList[l_paletteElement.m_boneIndex];

            m_bakedSkinningMatrixList[l_meshBaseIndex + l_paletteIndex] = l_skinningMatrix;

            // Meshの中心がこのボーンの影響でどれだけ動くかを調べ、最大値を覚えておく
            const auto  l_movedCenter  = TypeAlias::Math::Vector3::Transform(l_boundsCenter, l_skinningMatrix);
            const float l_moveDistance = TypeAlias::Math::Vector3::Distance (l_movedCenter,  l_boundsCenter);

            m_maxMoveDistanceList[l_meshIndex] = std::max(m_maxMoveDistanceList[l_meshIndex], l_moveDistance);
        }
    }
}

void FWK::Graphics::SkeletalAnimationBakedSkinning::BakeMorphWeight(const SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord)
{
    const auto& l_modelData    = a_skeletalAnimationModelRecord.GetREFModelData   ();
    const auto& l_morphSetting = a_skeletalAnimationModelRecord.GetREFMorphSetting();

    const auto& l_morphChannelNameList = l_modelData.m_morphChannelNameList;
    const auto& l_motionSequenceList   = l_modelData.m_motionSequenceList;

    const auto l_channelCount = static_cast<std::uint32_t>(l_morphChannelNameList.size());

    m_bakedMorphWeightList.clear();

    m_skinningInfo.m_morphChannelCount = l_channelCount;

    // モーフを持たないモデルは、ウェイトをベイクしない
    if (l_channelCount == k_initialFrameCount) { return; }

    // チャンネルごとの既定のウェイト(エディターで設定した値。設定が無ければ0)
    std::vector<float> l_defaultWeightList = {};

    l_defaultWeightList.reserve(l_channelCount);

    for (const auto& l_channelName : l_morphChannelNameList)
    {
        const auto* l_targetSetting = l_morphSetting.FindPTRTargetSetting(l_channelName);

        l_defaultWeightList.emplace_back(l_targetSetting ? l_targetSetting->m_defaultWeight : k_emptyMorphWeight);
    }

    // まず全コマを既定のウェイトで埋める(モーションが無いモデルの1コマも、これで決まる)
    m_bakedMorphWeightList.resize(static_cast<std::size_t>(m_skinningInfo.m_frameCount) * l_channelCount);

    for (std::uint32_t l_frameIndex = 0U; l_frameIndex < m_skinningInfo.m_frameCount; ++l_frameIndex)
    {
        std::ranges::copy(l_defaultWeightList, m_bakedMorphWeightList.begin() + static_cast<std::ptrdiff_t>(static_cast<std::size_t>(l_frameIndex) * l_channelCount));
    }

    // モーションにウェイトのアニメーションがあるチャンネルは、そのコマの時刻のウェイトで上書きする
    for (std::uint32_t l_motionIndex = 0U; l_motionIndex < static_cast<std::uint32_t>(l_motionSequenceList.size()); ++l_motionIndex)
    {
        const auto& l_motionSequence = l_motionSequenceList[l_motionIndex];

        for (const auto& l_morphWeightTrack : l_motionSequence.m_morphWeightTrackList)
        {
            const std::uint32_t l_channelIndex = a_skeletalAnimationModelRecord.FindVALMorphChannelIndex(l_morphWeightTrack.m_targetName);

            // どのMeshも持っていないターゲットのアニメーションは使わない
            if (l_channelIndex == Constant::k_invalidModelMorphTargetIndex) { continue; }

            for (std::uint32_t l_frameIndex = 0U; l_frameIndex < m_motionFrameCountList[l_motionIndex]; ++l_frameIndex)
            {
                // コマの時刻(最後のコマはモーションの長さを超えないようにする。スキニング行列のベイクと同じ計算)
                const float l_timeSecond = std::min(static_cast<float>(l_frameIndex) / Constant::k_skeletalAnimationBakeFrameRate, l_motionSequence.m_durationSecond);

                const std::size_t l_weightIndex = static_cast<std::size_t>(m_motionFrameOffsetList[l_motionIndex] + l_frameIndex) * l_channelCount + l_channelIndex;

                m_bakedMorphWeightList[l_weightIndex] = SampleMorphWeight(l_morphWeightTrack.m_keyFrameList, l_timeSecond);
            }
        }
    }
}

float FWK::Graphics::SkeletalAnimationBakedSkinning::SampleMorphWeight(const std::vector<Struct::ModelMorphWeightKeyFrame>& a_keyFrameList, const float a_timeSecond) const
{
    if (a_keyFrameList.empty()) { return k_emptyMorphWeight; }

    // 最初のキーより前、最後のキーより後は、端のキーの値を使う
    if (a_timeSecond <= a_keyFrameList.front().m_timeSecond) { return a_keyFrameList.front().m_weight; }
    if (a_timeSecond >= a_keyFrameList.back().m_timeSecond)  { return a_keyFrameList.back().m_weight; }

    // 時刻をはさむ2つのキーを探す(std::ranges::upper_bound : 時刻が「指定した時刻より大きい」最初のキー)
    const auto& l_nextItr = std::ranges::upper_bound(a_keyFrameList, a_timeSecond, std::less<>{}, &Struct::ModelMorphWeightKeyFrame::m_timeSecond);
    const auto& l_prevItr = std::prev(l_nextItr);

    const float l_keyInterval = l_nextItr->m_timeSecond - l_prevItr->m_timeSecond;

    if (l_keyInterval <= k_emptyMorphWeight) { return l_prevItr->m_weight; }

    // 2つのキーの間を直線で補間する(std::lerp(始めの値, 終わりの値, 0～1の割合))
    const float l_ratio = (a_timeSecond - l_prevItr->m_timeSecond) / l_keyInterval;

    return std::lerp(l_prevItr->m_weight, l_nextItr->m_weight, std::clamp(l_ratio, k_emptyMorphWeight, k_interpolationScale));
}

void FWK::Graphics::SkeletalAnimationBakedSkinning::BuildMorphChannelWeightRangeList(const SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord)
{
    const auto& l_modelData    = a_skeletalAnimationModelRecord.GetREFModelData   ();
    const auto& l_morphSetting = a_skeletalAnimationModelRecord.GetREFMorphSetting();

    m_morphChannelWeightRangeList.clear  ();
    m_morphChannelWeightRangeList.reserve(l_modelData.m_morphChannelNameList.size());

    for (const auto& l_channelName : l_modelData.m_morphChannelNameList)
    {
        MorphChannelWeightRange l_weightRange = {};

        if (const auto* l_targetSetting = l_morphSetting.FindPTRTargetSetting(l_channelName);
            l_targetSetting)
        {
            l_weightRange.m_minWeight = l_targetSetting->m_minWeight;
            l_weightRange.m_maxWeight = l_targetSetting->m_maxWeight;
        }

        m_morphChannelWeightRangeList.emplace_back(l_weightRange);
    }
}

float FWK::Graphics::SkeletalAnimationBakedSkinning::CalculateMaxMorphMoveDistance(const Struct::ModelMorphData& a_modelMorphData) const
{
    const auto& l_vertexRangeList        = a_modelMorphData.m_vertexRangeList;
    const auto& l_vertexDeltaList        = a_modelMorphData.m_vertexDeltaList;
    const auto& l_targetChannelIndexList = a_modelMorphData.m_targetChannelIndexList;

    float l_maxMoveDistance = k_initialMoveDistance;

    for (const auto& l_vertexRange : l_vertexRangeList)
    {
        // 全てのチャンネルを、ウェイトの範囲の端(絶対値が大きい方)まで効かせたときに動く距離
        float l_moveDistance = k_initialMoveDistance;

        for (std::uint32_t l_deltaIndex = l_vertexRange.m_deltaOffset; l_deltaIndex < l_vertexRange.m_deltaOffset + l_vertexRange.m_deltaCount; ++l_deltaIndex)
        {
            const auto& l_vertexDelta = l_vertexDeltaList[l_deltaIndex];

            float l_maxAbsWeight = Constant::k_defaultModelMorphTargetMAXWeight;

            if (l_vertexDelta.m_targetIndex < l_targetChannelIndexList.size() &&
                l_targetChannelIndexList[l_vertexDelta.m_targetIndex] < m_morphChannelWeightRangeList.size())
            {
                const auto& l_weightRange = m_morphChannelWeightRangeList[l_targetChannelIndexList[l_vertexDelta.m_targetIndex]];

                l_maxAbsWeight = std::max(std::abs(l_weightRange.m_minWeight), std::abs(l_weightRange.m_maxWeight));
            }

            l_moveDistance += l_vertexDelta.m_deltaPosition.Length() * l_maxAbsWeight;
        }

        l_maxMoveDistance = std::max(l_maxMoveDistance, l_moveDistance);
    }

    return l_maxMoveDistance;
}

void FWK::Graphics::SkeletalAnimationBakedSkinning::CopyBakedBuffer(const DirectCommandList& a_directCommandList, const UINT64 a_byteSize, DynamicRWStructuredBuffer& a_buffer, const UploadBuffer& a_uploadBuffer) const
{
    const auto& l_destinationResource = a_buffer.GetREFBufferGPUResource().m_resource;
    const auto& l_sourceResource      = a_uploadBuffer.GetREFUploadBuffer      ();

    FWK_ASSERT_RETURN_IF(!l_destinationResource, "ベイクしたデータ用Bufferが作成されていないため、コピーできません。");
    FWK_ASSERT_RETURN_IF(!l_sourceResource,      "ベイクしたデータ用UploadBufferが作成されていないため、コピーできません。");

    // コピー先にできる状態へ遷移 → コピー → シェーダーから読める状態へ遷移
    a_directCommandList.TransitionResourceBarrier(l_destinationResource, a_buffer.GetVALCurrentResourceState(), D3D12_RESOURCE_STATE_COPY_DEST);

    a_directCommandList.CopyBufferRegion(k_bufferStartOffset,
                                         k_bufferStartOffset,
                                         a_byteSize,
                                         *l_destinationResource.Get(),
                                         *l_sourceResource.Get());

    a_directCommandList.TransitionResourceBarrier(l_destinationResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    a_buffer.SetCurrentResourceState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}