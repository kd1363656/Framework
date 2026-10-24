#include "SkeletalAnimationBakedSkinning.h"

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
        const auto l_frameCount = static_cast<std::uint32_t>(std::floor(l_motionSequence.m_durationSecond * k_bakeFrameRate)) + k_singleFrameCount;

        m_motionFrameOffsetList.emplace_back(l_totalFrameCount);
        m_motionFrameCountList.emplace_back (l_frameCount);

        l_totalFrameCount += l_frameCount;
    }

    // 3. 全コマのスキニング行列を計算する
    m_bakedSkinningMatrixList.assign(static_cast<std::size_t>(l_totalFrameCount) * l_paletteStride, TypeAlias::Math::Matrix::Identity);
    m_maxMoveDistanceList.assign    (l_modelMeshList.size(), k_initialMoveDistance);

    m_skinningInfo.m_paletteStride = l_paletteStride;
    m_skinningInfo.m_frameCount    = l_totalFrameCount;

    std::vector<TypeAlias::Math::Matrix> l_globalBoneMatrixList = l_poseEvaluator.GetREFBindPoseGlobalBoneMatrixList();

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
            const float l_timeSecond = std::min(static_cast<float>(l_frameIndex) / k_bakeFrameRate, l_motionSequence.m_durationSecond);

            // その時刻のボーンの姿勢(モデル全体から見た行列)を求める
            const bool l_isEvaluated = l_poseEvaluator.EvaluatePose(l_modelData,
                                                                    l_timeSecond,
                                                                    SkeletalAnimationModelRecord::k_initialAnimationTimeSecond,
                                                                    SkeletalAnimationPlayer::k_initialBlendWeight,
                                                                    l_motionIndex,
                                                                    SkeletalAnimationPoseEvaluator::k_invalidMotionIndex,
                                                                    k_isBlending,
                                                                    l_globalBoneMatrixList);

            FWK_ASSERT_RETURN_VALUE_IF(!l_isEvaluated, "ボーンの姿勢の計算に失敗したため、アニメーションのベイクに失敗しました。", false);

            BakeMotionFrame(l_modelData, l_globalBoneMatrixList, m_motionFrameOffsetList[l_motionIndex] + l_frameIndex);
        }
    }

    // 4. アニメーションで動いても全頂点を囲めるように、Meshを囲む球の半径を広げる
    //    頂点はどれも「元の球の中」にあり、スキニング行列で動く量は「中心が動く量」以下なので、
    //    元の半径 + 中心が動く最大距離 の球なら、どのコマでも必ず全頂点を囲める
    m_skinningInfo.m_boundsRadiusList.clear();

    for (std::size_t l_meshIndex = 0ULL; l_meshIndex < l_modelMeshList.size(); ++l_meshIndex)
    {
        m_skinningInfo.m_boundsRadiusList.emplace_back(l_modelMeshList[l_meshIndex].m_modelMeshletData.m_boundsRadius + m_maxMoveDistanceList[l_meshIndex]);
    }

    // 5. GPUが読むBufferと、そこへコピーするためのUploadBufferを作成する
    auto& l_graphicsManager         = GraphicsManager::GetInstance                          ();
    auto& l_resourceContext         = l_graphicsManager.GetMutableREFResourceContext        ();
    auto& l_cbvSRVUAVDescriptorPool = l_resourceContext.GetMutableREFCBVSRVUAVDescriptorPool();

    const auto& l_device             = l_graphicsManager.GetREFDevice            ();
    const auto& l_gpuMemoryAllocator = l_resourceContext.GetREFGPUMemoryAllocator();

    FWK_ASSERT_RETURN_VALUE_IF(!m_skinningMatrixBuffer.Create<TypeAlias::Math::Matrix>(l_device,
                                                                                       l_gpuMemoryAllocator,
                                                                                       m_bakedSkinningMatrixList.size(),
                                                                                       l_cbvSRVUAVDescriptorPool),
                                                                                       "スキニング行列用Bufferの作成に失敗しました。",
                                                                                       false);

    const auto l_bufferByteSize = sizeof(TypeAlias::Math::Matrix) * m_bakedSkinningMatrixList.size();

    FWK_ASSERT_RETURN_VALUE_IF(!m_uploadBuffer.Create(l_device, l_bufferByteSize), "スキニング行列用UploadBufferの作成に失敗しました。", false);

    auto* const l_mappedData = m_uploadBuffer.FetchPTRMappedData();

    FWK_ASSERT_RETURN_VALUE_IF(!l_mappedData, "スキニング行列用UploadBufferがMapされていません。", false);

    std::memcpy(l_mappedData, m_bakedSkinningMatrixList.data(), l_bufferByteSize);

    m_skinningInfo.m_skinningMatrixBufferSRVDescriptorIndex = m_skinningMatrixBuffer.GetVALSRVDescriptorIndex();

    return true;
}

void FWK::Graphics::SkeletalAnimationBakedSkinning::UploadIfNeeded(const DirectCommandList& a_directCommandList)
{
    if (m_isUploaded) { return; }

    const auto& l_destinationResource = m_skinningMatrixBuffer.GetREFBufferGPUResource().m_resource;
    const auto& l_sourceResource      = m_uploadBuffer.GetREFUploadBuffer();

    FWK_ASSERT_RETURN_IF(!l_destinationResource, "スキニング行列用Bufferが作成されていないため、コピーできません。");
    FWK_ASSERT_RETURN_IF(!l_sourceResource,      "スキニング行列用UploadBufferが作成されていないため、コピーできません。");

    // コピー先にできる状態へ遷移 → コピー → シェーダーから読める状態へ遷移
    a_directCommandList.TransitionResourceBarrier(l_destinationResource, m_skinningMatrixBuffer.GetVALCurrentResourceState(), D3D12_RESOURCE_STATE_COPY_DEST);

    a_directCommandList.CopyBufferRegion(k_bufferStartOffset,
                                         k_bufferStartOffset,
                                         sizeof(TypeAlias::Math::Matrix) * m_bakedSkinningMatrixList.size(),
                                         *l_destinationResource.Get(),
                                         *l_sourceResource.Get());

    a_directCommandList.TransitionResourceBarrier(l_destinationResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    m_skinningMatrixBuffer.SetCurrentResourceState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    // CPU側の行列は、コピー命令を積んだ後は不要なので解放する
    // (UploadBufferはGPUがコピーし終わるまで必要なため残しておく)
    m_bakedSkinningMatrixList.clear        ();
    m_bakedSkinningMatrixList.shrink_to_fit();

    m_isUploaded = true;
}

std::uint32_t FWK::Graphics::SkeletalAnimationBakedSkinning::FetchVALFrameIndex(const std::uint32_t a_motionIndex, const float a_animationTimeSecond) const
{
    if (m_motionFrameCountList.empty()) { return k_initialFrameCount; }

    // 範囲外のモーション番号は、最初のモーションとして扱う
    const auto l_motionIndex = a_motionIndex < m_motionFrameCountList.size() ? a_motionIndex : k_firstMotionIndex;
    const auto l_frameCount  = m_motionFrameCountList[l_motionIndex];

    // 再生時刻をコマ番号に直し、モーションの最後まで行ったら最初に戻す(ループ再生)
    const auto l_frameInMotion = static_cast<std::uint32_t>(std::max(a_animationTimeSecond, SkeletalAnimationModelRecord::k_initialAnimationTimeSecond) * k_bakeFrameRate) % l_frameCount;

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