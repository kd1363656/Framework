#pragma once

namespace FWK::Struct
{
    // LODで消えた頂点が、LODの三角形のどこに描かれるかを探すための、Mesh1つ分のデータ(LODを作る前に1回だけ作る)
    // m_vertexNeighborOffsetList : 頂点ごとの、m_vertexNeighborListの何番目から隣の頂点が並ぶか(頂点数 + 1個、最後は全体の数)
    // m_vertexNeighborList       : 辺でつながった隣の頂点番号(LODで消えた頂点の周りに残った頂点を探すために使う)
    // m_vertexPositionList       : 頂点の元の位置(消えた頂点に一番近いLODの三角形を探すために使う)
    // m_vertexNormalList         : 頂点の元の法線(長さ1。LODの表面が元の表面より内側か外側かを調べるために使う)
    struct ModelLODSurfaceSearchData final
    {
        std::vector<std::uint32_t> m_vertexNeighborOffsetList = {};
        std::vector<std::uint32_t> m_vertexNeighborList       = {};

        std::vector<TypeAlias::Math::Vector3> m_vertexPositionList = {};
        std::vector<TypeAlias::Math::Vector3> m_vertexNormalList   = {};
    };

    // LOD1段階を調べる間だけ使う作業用のデータ(消えた頂点ごとに作り直さずに使い回す)
    // m_vertexTriangleOffsetList : 頂点ごとの、m_vertexTriangleListの何番目から「その頂点を角に持つLODの三角形」が並ぶか(頂点数 + 1個)
    // m_vertexTriangleList       : LODの三角形の先頭のインデックス番号
    // m_visitedStampList         : 頂点ごとに、最後に訪れた探索の番号(m_currentVisitedStampと同じなら、今の探索で訪れた)
    struct ModelLODSurfaceSearchWorkData final
    {
        std::vector<bool> m_isKeptVertexList = {};

        std::vector<std::uint32_t> m_visitedStampList = {};

        std::vector<std::uint32_t> m_vertexTriangleOffsetList = {};
        std::vector<std::uint32_t> m_vertexTriangleList       = {};

        std::vector<std::uint32_t> m_currentLayerVertexList = {};
        std::vector<std::uint32_t> m_nextLayerVertexList    = {};
        std::vector<std::uint32_t> m_keptVertexList         = {};

        std::uint32_t m_currentVisitedStamp = Constant::k_initialModelLODSurfaceVisitedStamp;
    };

    // 消えた頂点が描かれる、LODの三角形の上の点
    // m_triangleBegin : LODの三角形の先頭のインデックス番号(見つからない場合はConstant::k_invalidModelLODSurfaceTriangleBegin)
    // m_barycentric   : 三角形の3つの角の混ぜ具合(重心座標。合計1)
    struct ModelLODSurfacePoint final
    {
        std::size_t m_triangleBegin = Constant::k_invalidModelLODSurfaceTriangleBegin;

        TypeAlias::Math::Vector3 m_barycentric = TypeAlias::Math::Vector3::Zero;
    };

    // LOD1段階で、LODの表面が元の表面からどれだけずれたか(ローカル座標の距離。どちらも0以上)
    // m_sinkDistance  : 元の表面より内側(法線と逆向き)へ沈んだ距離(95%の頂点が収まる距離)
    // m_bulgeDistance : 元の表面より外側(法線の向き)へ出た距離(95%の頂点が収まる距離)
    struct ModelLODSurfaceDeviation final
    {
        float m_sinkDistance  = Constant::k_initialModelLODSurfaceDeviationDistance;
        float m_bulgeDistance = Constant::k_initialModelLODSurfaceDeviationDistance;
    };
}