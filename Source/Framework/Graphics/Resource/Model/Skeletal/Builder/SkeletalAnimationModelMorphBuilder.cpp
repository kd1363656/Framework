#include "SkeletalAnimationModelMorphBuilder.h"

bool FWK::Graphics::SkeletalAnimationModelMorphBuilder::BuildModelRecordMorphData(SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord) const
{
    auto& l_modelData = a_skeletalAnimationModelRecord.GetMutableREFModelData();

    for (auto& l_modelMesh : l_modelData.m_modelMeshList)
    {
        auto& l_modelMorphData = l_modelMesh.m_modelMorphData;

        // ブレンドシェイプを持たないMesh
        if (l_modelMorphData.m_targetList.empty())
        {
            ClearSourceData(l_modelMorphData);

            l_modelMorphData = {};

            continue;
        }

        FWK_ASSERT_RETURN_VALUE_IF(!BuildModelMorphData(l_modelMesh.m_modelVertexList.size(), l_modelMorphData), "ModelMeshのモーフのデータ作成に失敗しました。", false);
    }

    return true;
}

void FWK::Graphics::SkeletalAnimationModelMorphBuilder::BuildModelRecordMorphChannel(SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord) const
{
    auto& l_modelData = a_skeletalAnimationModelRecord.GetMutableREFModelData();

    auto& l_morphChannelNameList = l_modelData.m_morphChannelNameList;

    l_morphChannelNameList.clear();

    // チャンネル名 -> チャンネル番号(初めて出てきた順に番号を付ける)
    std::unordered_map<std::wstring, std::uint32_t> l_channelIndexMap = {};

    for (auto& l_modelMesh : l_modelData.m_modelMeshList)
    {
        auto& l_modelMorphData = l_modelMesh.m_modelMorphData;

        l_modelMorphData.m_targetChannelIndexList.clear  ();
        l_modelMorphData.m_targetChannelIndexList.reserve(l_modelMorphData.m_targetList.size());

        for (const auto& l_target : l_modelMorphData.m_targetList)
        {
            const auto& [l_itr, l_isInserted] = l_channelIndexMap.try_emplace(l_target.m_targetName, static_cast<std::uint32_t>(l_morphChannelNameList.size()));

            if (l_isInserted)
            {
                l_morphChannelNameList.emplace_back(l_target.m_targetName);
            }

            l_modelMorphData.m_targetChannelIndexList.emplace_back(l_itr->second);
        }
    }
}

bool FWK::Graphics::SkeletalAnimationModelMorphBuilder::BuildModelMorphData(const std::size_t a_vertexCount, Struct::ModelMorphData& a_modelMorphData) const
{
    const auto& l_sourceVertexIndexList = a_modelMorphData.m_sourceVertexIndexList;
    const auto& l_sourceDeltaListList   = a_modelMorphData.m_sourceDeltaListList;

    FWK_ASSERT_RETURN_VALUE_IF(l_sourceVertexIndexList.size() != a_vertexCount,                      "頂点ごとのFBX内の頂点番号の数が、頂点数と一致しません。", false);
    FWK_ASSERT_RETURN_VALUE_IF(l_sourceDeltaListList.size()   != a_modelMorphData.m_targetList.size(), "移動量の一覧の数が、ターゲット数と一致しません。",         false);

    const auto& l_sourceVertexToVertexListTable = CreateSourceVertexToVertexListTable(l_sourceVertexIndexList);

    // 1. 頂点ごとに、受ける移動量を集める
    std::vector<std::vector<Struct::ModelMorphVertexDelta>> l_vertexDeltaListList = {};

    l_vertexDeltaListList.resize(a_vertexCount);

    for (std::size_t l_targetIndex = 0ULL; l_targetIndex < l_sourceDeltaListList.size(); ++l_targetIndex)
    {
        for (const auto& l_sourceDelta : l_sourceDeltaListList[l_targetIndex])
        {
            // このMeshの頂点が使っていないFBX内の頂点(別のMaterialの部分など)は対象外
            if (l_sourceDelta.m_sourceVertexIndex >= l_sourceVertexToVertexListTable.size()) { continue; }

            Struct::ModelMorphVertexDelta l_vertexDelta = {};

            l_vertexDelta.m_deltaPosition = l_sourceDelta.m_deltaPosition;
            l_vertexDelta.m_deltaNormal   = l_sourceDelta.m_deltaNormal;
            l_vertexDelta.m_targetIndex   = static_cast<std::uint32_t>(l_targetIndex);

            // 同じFBX内の頂点から作られた全ての頂点(UVの継ぎ目などで分かれた頂点)に同じ移動量を割り当てる
            for (const auto l_vertexIndex : l_sourceVertexToVertexListTable[l_sourceDelta.m_sourceVertexIndex])
            {
                l_vertexDeltaListList[l_vertexIndex].emplace_back(l_vertexDelta);
            }
        }
    }

    // 2. 頂点番号の順に1本のリストへ並べ、頂点ごとに「何番目から何個か」を記録する(CSR)
    a_modelMorphData.m_vertexRangeList.clear();
    a_modelMorphData.m_vertexDeltaList.clear();

    a_modelMorphData.m_vertexRangeList.resize(a_vertexCount);

    for (std::size_t l_vertexIndex = 0ULL; l_vertexIndex < a_vertexCount; ++l_vertexIndex)
    {
        auto& l_vertexRange = a_modelMorphData.m_vertexRangeList[l_vertexIndex];

        const auto& l_vertexDeltaList = l_vertexDeltaListList[l_vertexIndex];

        l_vertexRange.m_deltaOffset = static_cast<std::uint32_t>(a_modelMorphData.m_vertexDeltaList.size());
        l_vertexRange.m_deltaCount  = static_cast<std::uint32_t>(l_vertexDeltaList.size());

        a_modelMorphData.m_vertexDeltaList.insert(a_modelMorphData.m_vertexDeltaList.end(), l_vertexDeltaList.begin(), l_vertexDeltaList.end());
    }

    ClearSourceData   (a_modelMorphData);
    RemoveUnusedTarget(a_modelMorphData);

    // どのターゲットもこのMeshの頂点を動かさない場合は、モーフを持たないMeshとして扱う
    if (a_modelMorphData.m_targetList.empty())
    {
        a_modelMorphData = {};
    }

    return true;
}

std::vector<std::vector<std::uint32_t>> FWK::Graphics::SkeletalAnimationModelMorphBuilder::CreateSourceVertexToVertexListTable(const std::vector<std::uint32_t>& a_sourceVertexIndexList) const
{
    std::vector<std::vector<std::uint32_t>> l_sourceVertexToVertexListTable = {};

    if (a_sourceVertexIndexList.empty()) { return l_sourceVertexToVertexListTable; }

    // 表の大きさは、一番大きいFBX内の頂点番号 + 1
    const std::uint32_t l_maxSourceVertexIndex = std::ranges::max(a_sourceVertexIndexList);

    l_sourceVertexToVertexListTable.resize(static_cast<std::size_t>(l_maxSourceVertexIndex) + k_maxIndexToCountOffset);

    for (std::size_t l_vertexIndex = 0ULL; l_vertexIndex < a_sourceVertexIndexList.size(); ++l_vertexIndex)
    {
        l_sourceVertexToVertexListTable[a_sourceVertexIndexList[l_vertexIndex]].emplace_back(static_cast<std::uint32_t>(l_vertexIndex));
    }

    return l_sourceVertexToVertexListTable;
}

void FWK::Graphics::SkeletalAnimationModelMorphBuilder::RemoveUnusedTarget(Struct::ModelMorphData& a_modelMorphData) const
{
    const std::size_t l_targetCount = a_modelMorphData.m_targetList.size();

    std::vector<std::uint32_t> l_deltaCountList = {};

    l_deltaCountList.assign(l_targetCount, k_emptyDeltaCount);

    for (const auto& l_vertexDelta : a_modelMorphData.m_vertexDeltaList)
    {
        ++l_deltaCountList[l_vertexDelta.m_targetIndex];
    }

    // 古いターゲット番号 -> 詰めた後のターゲット番号
    std::vector<std::uint32_t> l_targetIndexRemapList = {};

    l_targetIndexRemapList.assign(l_targetCount, Constant::k_invalidModelMorphTargetIndex);

    std::vector<Struct::ModelMorphTarget> l_usedTargetList = {};

    for (std::size_t l_targetIndex = 0ULL; l_targetIndex < l_targetCount; ++l_targetIndex)
    {
        if (l_deltaCountList[l_targetIndex] == k_emptyDeltaCount) { continue; }

        l_targetIndexRemapList[l_targetIndex] = static_cast<std::uint32_t>(l_usedTargetList.size());

        l_usedTargetList.emplace_back(std::move(a_modelMorphData.m_targetList[l_targetIndex]));
    }

    for (auto& l_vertexDelta : a_modelMorphData.m_vertexDeltaList)
    {
        l_vertexDelta.m_targetIndex = l_targetIndexRemapList[l_vertexDelta.m_targetIndex];
    }

    a_modelMorphData.m_targetList = std::move(l_usedTargetList);
}

void FWK::Graphics::SkeletalAnimationModelMorphBuilder::ClearSourceData(Struct::ModelMorphData& a_modelMorphData) const
{
    // FBXから作るときだけ使うデータは、メモリを返すためにshrink_to_fitまで行う
    a_modelMorphData.m_sourceVertexIndexList.clear        ();
    a_modelMorphData.m_sourceVertexIndexList.shrink_to_fit();
    a_modelMorphData.m_sourceDeltaListList.clear          ();
    a_modelMorphData.m_sourceDeltaListList.shrink_to_fit  ();
}