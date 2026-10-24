#include "ModelGPUDrivenTableBuilder.h"

void FWK::Graphics::ModelGPUDrivenTableBuilder::Clear()
{
    m_frameTable.m_batchList.clear   ();
    m_frameTable.m_instanceList.clear();
    m_frameTable.m_meshInfoList.clear();
    m_frameTable.m_lodList.clear     ();

    m_frameTable.m_totalPairCount = Constant::k_initialGPUDrivenFrameTableTotalPairCount;

    m_modelBatchIndexMap.clear();

    // 中身だけ消して、確保したメモリは次のフレームでも使い回す
    for (auto& l_batchInstanceList : m_batchInstanceListList)
    {
        l_batchInstanceList.clear();
    }
}

const FWK::Struct::ModelGPUDrivenFrameTable& FWK::Graphics::ModelGPUDrivenTableBuilder::Build()
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

    return m_frameTable;
}

FWK::Struct::SBModelGPUDrivenInstance FWK::Graphics::ModelGPUDrivenTableBuilder::CreateInstance(const TypeAlias::Math::Matrix& a_worldMatrix, const TypeAlias::Math::Matrix& a_worldInverseTransposeMatrix) const
{
    Struct::SBModelGPUDrivenInstance l_instance = {};

    l_instance.m_worldMatrix                 = a_worldMatrix;
    l_instance.m_worldInverseTransposeMatrix = a_worldInverseTransposeMatrix;
    l_instance.m_worldMaxScale               = Utility::CalculateWorldMaxScale(a_worldMatrix);

    // 拡大率がマイナスで裏返っている(鏡写しの)行列なら、三角形の向きを反転させる
    l_instance.m_worldOrientationSign = a_worldMatrix.Determinant() < Constant::k_modelWorldOrientationDeterminantBoundary ? Constant::k_mirrorModelWorldOrientationSign : Constant::k_normalModelWorldOrientationSign;

    return l_instance;
}