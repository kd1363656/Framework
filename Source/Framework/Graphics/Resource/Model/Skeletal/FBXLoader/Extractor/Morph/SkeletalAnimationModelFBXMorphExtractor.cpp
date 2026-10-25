#include "SkeletalAnimationModelFBXMorphExtractor.h"

bool FWK::Graphics::SkeletalAnimationModelFBXMorphExtractor::ExtractModelMorphSource(const ufbx_mesh* a_fbxMesh, Struct::ModelMorphData& a_modelMorphData) const
{
    a_modelMorphData.m_targetList.clear         ();
    a_modelMorphData.m_sourceDeltaListList.clear();

    FWK_ASSERT_RETURN_VALUE_IF(!a_fbxMesh, "モーフを取り出すufbx_meshがnullptrです。", false);

    for (std::size_t l_deformerIndex = 0ULL; l_deformerIndex < a_fbxMesh->blend_deformers.count; ++l_deformerIndex)
    {
        const auto* l_fbxBlendDeformer = a_fbxMesh->blend_deformers.data[l_deformerIndex];

        if (!l_fbxBlendDeformer) { continue; }

        for (std::size_t l_channelIndex = 0ULL; l_channelIndex < l_fbxBlendDeformer->channels.count; ++l_channelIndex)
        {
            const auto* l_fbxBlendChannel = l_fbxBlendDeformer->channels.data[l_channelIndex];

            if (!l_fbxBlendChannel) { continue; }

            const auto* l_fbxBlendShape = FindPTRTargetBlendShape(l_fbxBlendChannel);

            // 形を持たないターゲットは、頂点を動かさないため取り出さない
            if (!l_fbxBlendShape) { continue; }

            std::vector<Struct::ModelMorphSourceDelta> l_sourceDeltaList = {};

            FWK_ASSERT_RETURN_VALUE_IF(!ExtractModelMorphSourceDeltaList(a_fbxMesh, l_fbxBlendShape, l_sourceDeltaList), "ブレンドシェイプの移動量の取り出しに失敗しました。", false);

            if (l_sourceDeltaList.empty()) { continue; }

            Struct::ModelMorphTarget l_target = {};

            l_target.m_targetName = Utility::ConvertUFBXStringToWString(l_fbxBlendChannel->name);

            a_modelMorphData.m_targetList.emplace_back         (std::move(l_target));
            a_modelMorphData.m_sourceDeltaListList.emplace_back(std::move(l_sourceDeltaList));
        }
    }

    return true;
}

bool FWK::Graphics::SkeletalAnimationModelFBXMorphExtractor::ExtractModelMorphWeightTrackList(const ufbx_scene*                                 a_fbxScene,
                                                                                              const ufbx_baked_anim*                            a_fbxBakedAnimation,
                                                                                              const double                                      a_animationDurationSecond,
                                                                                              const double                                      a_animationFrameRate,
                                                                                                    std::vector<Struct::ModelMorphWeightTrack>& a_modelMorphWeightTrackList) const
{
    a_modelMorphWeightTrackList.clear();

    FWK_ASSERT_RETURN_VALUE_IF(!a_fbxScene,          "モーフのウェイトを取り出すufbx_sceneがnullptrです。",       false);
    FWK_ASSERT_RETURN_VALUE_IF(!a_fbxBakedAnimation, "モーフのウェイトを取り出すufbx_baked_animがnullptrです。", false);

    // ボーンのキーフレームと同じ時刻でウェイトを取り出す
    const auto& l_sampleTimeSecondList = Utility::CreateAnimationSampleTimeSecondList(a_animationDurationSecond, a_animationFrameRate);

    FWK_ASSERT_RETURN_VALUE_IF(l_sampleTimeSecondList.empty(), "モーフのウェイトを取り出す時刻が作れませんでした。", false);

    // Bake済みアニメーションのelementsには、「アニメーションで値が変わるプロパティ」が要素ごとに入っている
    // ブレンドシェイプのウェイトは、ufbx_blend_channelのDeformPercentプロパティとして入っている
    for (std::size_t l_bakedElementIndex = 0ULL; l_bakedElementIndex < a_fbxBakedAnimation->elements.count; ++l_bakedElementIndex)
    {
        const auto& l_fbxBakedElement = a_fbxBakedAnimation->elements.data[l_bakedElementIndex];

        FWK_ASSERT_RETURN_VALUE_IF(l_fbxBakedElement.element_id >= a_fbxScene->elements.count, "Bake済み要素のElementIDがufbx_scene::elementsの範囲外です。", false);

        const auto* l_fbxBlendChannel = ufbx_as_blend_channel(a_fbxScene->elements.data[l_fbxBakedElement.element_id]);

        // ブレンドシェイプのターゲット以外(カメラなど)の要素は対象外
        if (!l_fbxBlendChannel) { continue; }

        const auto& l_targetName = Utility::ConvertUFBXStringToWString(l_fbxBlendChannel->name);

        // 複数のMeshが同じ名前のターゲットを持つ場合も、ウェイトは名前ごとに1つだけ持つ
        if (const auto& l_itr = std::ranges::find(a_modelMorphWeightTrackList, l_targetName, &Struct::ModelMorphWeightTrack::m_targetName);
            l_itr != a_modelMorphWeightTrackList.end())
        {
            continue;
        }

        for (std::size_t l_bakedPropIndex = 0ULL; l_bakedPropIndex < l_fbxBakedElement.props.count; ++l_bakedPropIndex)
        {
            const auto& l_fbxBakedProp = l_fbxBakedElement.props.data[l_bakedPropIndex];

            if (std::string_view{ l_fbxBakedProp.name.data, l_fbxBakedProp.name.length } != UFBX_DeformPercent) { continue; }

            Struct::ModelMorphWeightTrack l_modelMorphWeightTrack = {};

            if (!CreateModelMorphWeightTrack(l_fbxBakedProp,
                                             l_sampleTimeSecondList,
                                             l_targetName,
                                             l_modelMorphWeightTrack))
            {
                continue;
            }

            a_modelMorphWeightTrackList.emplace_back(std::move(l_modelMorphWeightTrack));
        }
    }

    return true;
}

bool FWK::Graphics::SkeletalAnimationModelFBXMorphExtractor::HasMorph(const ufbx_mesh* a_fbxMesh) const
{
    if (!a_fbxMesh) { return false; }

    return a_fbxMesh->blend_deformers.count != Constant::k_emptyUFBXElementCount;
}

bool FWK::Graphics::SkeletalAnimationModelFBXMorphExtractor::ExtractModelMorphSourceDeltaList(const ufbx_mesh* a_fbxMesh, const ufbx_blend_shape* a_fbxBlendShape, std::vector<Struct::ModelMorphSourceDelta>& a_sourceDeltaList) const
{
    a_sourceDeltaList.clear();

    FWK_ASSERT_RETURN_VALUE_IF(!a_fbxMesh,      "移動量を取り出すufbx_meshがnullptrです。",        false);
    FWK_ASSERT_RETURN_VALUE_IF(!a_fbxBlendShape, "移動量を取り出すufbx_blend_shapeがnullptrです。", false);

    const auto l_offsetCount = a_fbxBlendShape->num_offsets;

    FWK_ASSERT_RETURN_VALUE_IF(a_fbxBlendShape->offset_vertices.count  < l_offsetCount, "ブレンドシェイプの頂点番号の数が移動量の数より少ないです。", false);
    FWK_ASSERT_RETURN_VALUE_IF(a_fbxBlendShape->position_offsets.count < l_offsetCount, "ブレンドシェイプの位置の移動量の数が足りません。",           false);

    // 法線の移動量と、頂点ごとの重み(Blenderだけが書き出す)は無い場合がある
    const bool l_hasNormalOffset = a_fbxBlendShape->normal_offsets.count >= l_offsetCount;
    const bool l_hasOffsetWeight = a_fbxBlendShape->offset_weights.count >= l_offsetCount;

    a_sourceDeltaList.reserve(l_offsetCount);

    for (std::size_t l_offsetIndex = 0ULL; l_offsetIndex < l_offsetCount; ++l_offsetIndex)
    {
        const auto l_sourceVertexIndex = a_fbxBlendShape->offset_vertices.data[l_offsetIndex];

        // ufbxの説明にある通り、頂点番号がMeshの頂点数を超える場合があるので取り除く
        if (l_sourceVertexIndex >= a_fbxMesh->num_vertices) { continue; }

        const float l_offsetWeight = l_hasOffsetWeight ? static_cast<float>(a_fbxBlendShape->offset_weights.data[l_offsetIndex]) : k_defaultOffsetWeight;

        Struct::ModelMorphSourceDelta l_sourceDelta = {};

        l_sourceDelta.m_sourceVertexIndex = l_sourceVertexIndex;
        l_sourceDelta.m_deltaPosition     = Utility::ConvertUFBXVector3ToVector3(a_fbxBlendShape->position_offsets.data[l_offsetIndex]) * l_offsetWeight;

        if (l_hasNormalOffset)
        {
            l_sourceDelta.m_deltaNormal = Utility::ConvertUFBXVector3ToVector3(a_fbxBlendShape->normal_offsets.data[l_offsetIndex]) * l_offsetWeight;
        }

        // ほとんど動かない頂点は、移動量を持たせない(頂点ごとの移動量の数を減らす)
        if (l_sourceDelta.m_deltaPosition.LengthSquared() <= k_minDeltaLengthSquared &&
            l_sourceDelta.m_deltaNormal.LengthSquared()   <= k_minDeltaLengthSquared)
        {
            continue;
        }

        a_sourceDeltaList.emplace_back(l_sourceDelta);
    }

    return true;
}

bool FWK::Graphics::SkeletalAnimationModelFBXMorphExtractor::CreateModelMorphWeightTrack(const ufbx_baked_prop&               a_fbxBakedProp,
                                                                                         const std::vector<double>&           a_sampleTimeSecondList,
                                                                                         const std::wstring&                  a_targetName,
                                                                                               Struct::ModelMorphWeightTrack& a_modelMorphWeightTrack) const
{
    a_modelMorphWeightTrack = {};

    if (a_fbxBakedProp.keys.count == Constant::k_emptyUFBXElementCount) { return false; }

    a_modelMorphWeightTrack.m_targetName = a_targetName;

    a_modelMorphWeightTrack.m_keyFrameList.reserve(a_sampleTimeSecondList.size());

    bool l_isAnyWeightUsed = false;

    for (const auto l_sampleTimeSecond : a_sampleTimeSecondList)
    {
        // Bake済みのキーは値をvec3で持つため、1つの値のプロパティはxに入っている
        const auto& l_fbxValue = ufbx_evaluate_baked_vec3(a_fbxBakedProp.keys, l_sampleTimeSecond);

        Struct::ModelMorphWeightKeyFrame l_keyFrame = {};

        l_keyFrame.m_timeSecond = static_cast<float>(l_sampleTimeSecond);
        l_keyFrame.m_weight     = static_cast<float>(l_fbxValue.x * k_deformPercentToWeightScale);

        l_isAnyWeightUsed = l_isAnyWeightUsed || l_keyFrame.m_weight != k_emptyWeight;

        a_modelMorphWeightTrack.m_keyFrameList.emplace_back(l_keyFrame);
    }

    // 最初から最後までウェイトが0のターゲットは、アニメーションしないので保存しない
    return l_isAnyWeightUsed;
}

const ufbx_blend_shape* FWK::Graphics::SkeletalAnimationModelFBXMorphExtractor::FindPTRTargetBlendShape(const ufbx_blend_channel* a_fbxBlendChannel) const
{
    if (!a_fbxBlendChannel) { return nullptr; }

    // target_shapeは、途中の形(インビトウィーン)を除いた最終的な形
    if (a_fbxBlendChannel->target_shape) { return a_fbxBlendChannel->target_shape; }

    // target_shapeが無い場合は、最後のキーの形を使う
    if (a_fbxBlendChannel->keyframes.count == Constant::k_emptyUFBXElementCount) { return nullptr; }

    return a_fbxBlendChannel->keyframes.data[a_fbxBlendChannel->keyframes.count - k_lastKeyFrameOffset].shape;
}