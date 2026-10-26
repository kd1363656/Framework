#include "ModelGPUDrivenTableBuilder.h"

void FWK::Graphics::ModelGPUDrivenTableBuilder::BeginTable(const std::size_t a_drawRequestCount)
{
    ++m_frameCount;

    // 描画申請の数が変わった(追加された、破棄された)場合も、並びが変わっているため作り直す
    m_isRebuilding      = m_isRebuildRequired || a_drawRequestCount != m_instanceSlotList.size();
    m_isRebuildRequired = false;

    if (!m_isRebuilding) { return; }

    m_frameTable.m_batchList.clear      ();
    m_frameTable.m_instanceList.clear   ();
    m_frameTable.m_meshInfoList.clear   ();
    m_frameTable.m_lodList.clear        ();
    m_frameTable.m_morphWeightList.clear();

    m_frameTable.m_totalPairCount = Constant::k_initialGPUDrivenFrameTableTotalPairCount;

    m_modelEntryList.clear    ();
    m_modelEntryIndexMap.clear();

    m_instanceSlotList.assign(a_drawRequestCount, InstanceSlot{});

    // 中身だけ消して、確保したメモリは次の作り直しでも使い回す
    for (auto& l_batchInstanceList : m_batchInstanceListList)
    {
        l_batchInstanceList.clear();
    }
}

void FWK::Graphics::ModelGPUDrivenTableBuilder::SetInvalidInstance(const std::size_t a_drawRequestIndex)
{
    FWK_ASSERT_RETURN_IF(a_drawRequestIndex >= m_instanceSlotList.size(), "描画申請の番号がBeginTableで渡した数を超えています。");

    // 作り直し中なら、表に入れないだけでよい(InstanceSlotは無効値のまま)
    if (m_isRebuilding) { return; }

    const auto& l_instanceSlot = m_instanceSlotList[a_drawRequestIndex];

    if (l_instanceSlot.m_modelEntryIndex != k_invalidIndex)
    {
        m_isRebuildRequired = true;
    }
}

const FWK::Struct::ModelGPUDrivenFrameTable& FWK::Graphics::ModelGPUDrivenTableBuilder::EndTable()
{
    if (!m_isRebuilding) { return m_frameTable; }

    m_isRebuilding = false;

    BuildInstanceList();

    // 作り直した表は全て中身が変わっているため、全てのVersionを進める
    ++m_frameTable.m_meshTableVersion;
    ++m_frameTable.m_instanceTableVersion;
    ++m_frameTable.m_morphWeightTableVersion;

    return m_frameTable;
}

void FWK::Graphics::ModelGPUDrivenTableBuilder::ApplyTargetMaterialStaticTypeID(const TypeAlias::StaticTypeID a_targetMaterialStaticTypeID)
{
    if (m_targetMaterialStaticTypeID == a_targetMaterialStaticTypeID) { return; }

    m_targetMaterialStaticTypeID = a_targetMaterialStaticTypeID;
    m_isRebuildRequired          = true;
}

FWK::Struct::SBModelGPUDrivenInstance FWK::Graphics::ModelGPUDrivenTableBuilder::CreateInstance(const Struct::ModelGPUDrivenInstanceSource& a_instanceSource) const
{
    const auto& l_worldMatrix = *a_instanceSource.m_worldMatrix;

    Struct::SBModelGPUDrivenInstance l_instance = {};

    l_instance.m_worldMatrix                 = l_worldMatrix;
    l_instance.m_worldInverseTransposeMatrix = *a_instanceSource.m_worldInverseTransposeMatrix;
    l_instance.m_worldMaxScale               = Utility::CalculateWorldMaxScale(l_worldMatrix);
    l_instance.m_animationFrameIndex         = a_instanceSource.m_animationFrameIndex;

    // 拡大率がマイナスで裏返っている(鏡写しの)行列なら、三角形の向きを反転させる
    l_instance.m_worldOrientationSign = l_worldMatrix.Determinant() < Constant::k_modelWorldOrientationDeterminantBoundary ? Constant::k_mirrorModelWorldOrientationSign : Constant::k_normalModelWorldOrientationSign;

    return l_instance;
}

std::uint32_t FWK::Graphics::ModelGPUDrivenTableBuilder::RegisterMorphWeightList(const Struct::ModelGPUDrivenInstanceSource& a_instanceSource, InstanceSlot& a_instanceSlot)
{
    const auto* const l_sourceMorphWeightList = a_instanceSource.m_morphWeightList;

    if (!l_sourceMorphWeightList ||
        l_sourceMorphWeightList->empty())
    {
        return Constant::k_invalidGPUDrivenMorphWeightOffset;
    }

    // 入りきらなかった場合でも、次のフレームで数が変わったと判定しないように、数は覚えておく
    a_instanceSlot.m_requestedMorphWeightCount = l_sourceMorphWeightList->size();

    auto& l_morphWeightList = m_frameTable.m_morphWeightList;

    if (l_morphWeightList.size() + l_sourceMorphWeightList->size() > Constant::k_modelGPUDrivenMaxMorphWeightCount) { return Constant::k_invalidGPUDrivenMorphWeightOffset; }

    a_instanceSlot.m_morphWeightOffset = static_cast<std::uint32_t>(l_morphWeightList.size());

    l_morphWeightList.insert(l_morphWeightList.end(), l_sourceMorphWeightList->begin(), l_sourceMorphWeightList->end());

    return a_instanceSlot.m_morphWeightOffset;
}
void FWK::Graphics::ModelGPUDrivenTableBuilder::UpdateMorphWeightList(const Struct::ModelGPUDrivenInstanceSource& a_instanceSource, const InstanceSlot& a_instanceSlot)
{
    const auto* const l_sourceMorphWeightList = a_instanceSource.m_morphWeightList;

    const std::size_t l_sourceMorphWeightCount = l_sourceMorphWeightList ? l_sourceMorphWeightList->size() : k_emptyMorphWeightCount;

    if (l_sourceMorphWeightCount != a_instanceSlot.m_requestedMorphWeightCount)
    {
        m_isRebuildRequired = true;

        return;
    }

    if (a_instanceSlot.m_morphWeightOffset == Constant::k_invalidGPUDrivenMorphWeightOffset) { return; }

    auto* const       l_destinationMorphWeight = m_frameTable.m_morphWeightList.data() + a_instanceSlot.m_morphWeightOffset;
    const std::size_t l_morphWeightByteSize    = sizeof(float) * l_sourceMorphWeightCount;

    if (std::memcmp(l_destinationMorphWeight, l_sourceMorphWeightList->data(), l_morphWeightByteSize) == k_sameMemoryCompareResult) { return; }

    std::memcpy(l_destinationMorphWeight, l_sourceMorphWeightList->data(), l_morphWeightByteSize);

    ++m_frameTable.m_morphWeightTableVersion;
}

void FWK::Graphics::ModelGPUDrivenTableBuilder::BuildInstanceList()
{
    auto& l_instanceList = m_frameTable.m_instanceList;

    // 同じモデルのインスタンスが連続して並ぶように、表を1本にまとめる
    for (std::size_t l_batchIndex = 0ULL; l_batchIndex < m_frameTable.m_batchList.size(); ++l_batchIndex)
    {
        auto&       l_modelBatch        = m_frameTable.m_batchList[l_batchIndex];
        const auto& l_batchInstanceList = m_batchInstanceListList [l_batchIndex];

        // インスタンスが表に入りきらない分は描画しない
        const auto l_remainCount   = Constant::k_modelGPUDrivenMaxInstanceCount - l_instanceList.size();
        const auto l_instanceCount = std::min(l_batchInstanceList.size(), l_remainCount);

        l_modelBatch.m_instanceOffset = static_cast<std::uint32_t>(l_instanceList.size());
        l_modelBatch.m_instanceCount  = static_cast<std::uint32_t>(l_instanceCount);

        m_frameTable.m_totalPairCount += l_instanceCount * l_modelBatch.m_meshCount;

        l_instanceList.insert(l_instanceList.end(), l_batchInstanceList.begin(), l_batchInstanceList.begin() + static_cast<std::ptrdiff_t>(l_instanceCount));
    }

    // 描画申請ごとに、まとめた表の中での番号を覚えておく(作り直さないフレームはこの番号の位置を書き換える)
    for (auto& l_instanceSlot : m_instanceSlotList)
    {
        if (l_instanceSlot.m_modelEntryIndex    == k_invalidIndex ||
            l_instanceSlot.m_localInstanceIndex == k_invalidIndex)
        {
            continue;
        }

        const auto& l_modelEntry = m_modelEntryList[l_instanceSlot.m_modelEntryIndex];
        const auto& l_modelBatch = m_frameTable.m_batchList[l_modelEntry.m_batchIndex];

        if (l_instanceSlot.m_localInstanceIndex >= l_modelBatch.m_instanceCount) { continue; }

        l_instanceSlot.m_instanceIndex = l_modelBatch.m_instanceOffset + l_instanceSlot.m_localInstanceIndex;
    }
}