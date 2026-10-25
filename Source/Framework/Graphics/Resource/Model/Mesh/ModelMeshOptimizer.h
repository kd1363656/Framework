#pragma once

namespace FWK::Graphics
{
    template <Concept::IsDerivedAssetRecordBaseConcept ModelRecordType>
    class ModelMeshOptimizer final
    {
    public:

         ModelMeshOptimizer() = default;
        ~ModelMeshOptimizer() = default;

        bool OptimizeModelRecord(ModelRecordType& a_modelRecord) const
        {
            auto& l_modelData = a_modelRecord.GetMutableREFModelData();

            FWK_ASSERT_RETURN_VALUE_IF(l_modelData.m_modelMeshList.empty(), "ModelDataのMeshリストが空のため、ModelMeshの最適化に失敗しました。", false);

            for (auto& l_modelMesh : l_modelData.m_modelMeshList)
            {
                // ModelDataはMaterial単位などで複数のModelMeshを持つ。
                // meshoptimizerはMesh単位のVertexList/IndexListに対して適用するため、
                // StaticModelMeshごとに個別に最適化する
                FWK_ASSERT_RETURN_VALUE_IF(!OptimizeModelMesh(l_modelMesh), "ModelMeshの最適化に失敗しました。", false);
            }

            return true;
        }

    private:

        bool OptimizeModelMesh(typename ModelRecordType::ModelMesh& a_modelMesh) const
        {
            FWK_ASSERT_RETURN_VALUE_IF(a_modelMesh.m_modelVertexList.empty(), "ModelMeshの頂点リストが空のため、ModelMeshの最適化に失敗しました。",         false);
            FWK_ASSERT_RETURN_VALUE_IF(a_modelMesh.m_indexList.empty(),       "ModelMeshのインデックスリストが空のため、ModelMeshの最適化に失敗しました。", false);

            // 頂点と一緒に並び替える「頂点ごとのFBX内の頂点番号」(モーフを持つMeshだけ、持たない場合はnullptr)
            auto* l_sourceVertexIndexList = FetchMutablePTRMorphSourceVertexIndexList(a_modelMesh);

            std::vector<std::uint32_t> l_vertexRemapList = {};

            // Remap表は「古い頂点番号 -> 新しい頂点番号」の対応表。
            // そのため、Index数ではなく、元の頂点数と同じ要素数を確保する。
            l_vertexRemapList.resize(a_modelMesh.m_modelVertexList.size());

            // 重複している頂点をまとめるためのRemap表を作成する
            const std::size_t l_optimizedVertexCount = GenerateVertexRemap(a_modelMesh, l_sourceVertexIndexList, l_vertexRemapList);

            FWK_ASSERT_RETURN_VALUE_IF(l_optimizedVertexCount == k_invalidOptimizedVertexCount, "meshopt_generateVertexRemapMultiによる頂点リマップ作成に失敗しました。", false);

            std::vector<std::uint32_t> l_optimizedIndexList = {};

            l_optimizedIndexList.resize(a_modelMesh.m_indexList.size());

            // 現在のIndexListを、Remap表に従って新しい頂点番号へ変換する
            // meshopt_remapIndexBuffer(最適化後のインデックス配列の書き込み先、
            //                          現在のインデックス配列、
            //                          現在のインデックス数、
            //                          meshopt_generateVertexRemapMultiで作成した対応表);
            meshopt_remapIndexBuffer(l_optimizedIndexList.data(),
                                     a_modelMesh.m_indexList.data(),
                                     a_modelMesh.m_indexList.size(),
                                     l_vertexRemapList.data());

            // Remap表に従って、重複を取り除いた新しいVertexListを作成する(FBX内の頂点番号も同じように並び替える)
            RemapVertexStreamList(l_vertexRemapList, l_optimizedVertexCount, a_modelMesh, l_sourceVertexIndexList);

            // GPUの頂点キャッシュに乗りやすいように、IndexListの順番を並べ替える
            // meshopt_optimizeVertexCache(最適化後のインデックス配列の書き込み先、
            //                             現在のインデックス配列、
            //                             現在のインデックス、
            //                             現在の頂点数);
            meshopt_optimizeVertexCache(l_optimizedIndexList.data(),
                                        l_optimizedIndexList.data(),
                                        l_optimizedIndexList.size(),
                                        l_optimizedVertexCount);

            std::vector<std::uint32_t> l_vertexFetchRemapList = {};

            l_vertexFetchRemapList.resize(l_optimizedVertexCount);

            // IndexListから参照される順番に合わせて、VertexListのメモリ配置を並び替えるためのRemap表を作成する
            // 頂点とFBX内の頂点番号の2つの配列に同じ並び替えを行うため、並び替え方だけを受け取る
            // meshopt_optimizeVertexFetchRemap(並び替え方を書き込むRemap表、
            //                                  インデックス配列、
            //                                  インデックス数、
            //                                  頂点数);
            const std::size_t l_fetchedVertexCount = meshopt_optimizeVertexFetchRemap(l_vertexFetchRemapList.data(),
                                                                                      l_optimizedIndexList.data(),
                                                                                      l_optimizedIndexList.size(),
                                                                                      l_optimizedVertexCount);

            meshopt_remapIndexBuffer(l_optimizedIndexList.data(),
                                     l_optimizedIndexList.data(),
                                     l_optimizedIndexList.size(),
                                     l_vertexFetchRemapList.data());

            RemapVertexStreamList(l_vertexFetchRemapList, l_fetchedVertexCount, a_modelMesh, l_sourceVertexIndexList);

            // 最適化後のインデックスのリストを格納
            a_modelMesh.m_indexList = std::move(l_optimizedIndexList);

            return true;
        }

        std::size_t GenerateVertexRemap(const typename ModelRecordType::ModelMesh& a_modelMesh, const std::vector<std::uint32_t>* a_sourceVertexIndexList, std::vector<std::uint32_t>& a_vertexRemapList) const
        {
            // ModelRecordTypeに応じた頂点一つ分のバイトサイズを取得する
            const std::size_t l_modelVertexSize = sizeof(typename ModelRecordType::ModelVertex);

            // meshopt_Stream : 頂点ごとのデータの並び(先頭アドレス、1要素のサイズ、1要素の間隔)
            // 複数のStreamを渡すと、「全てのStreamで値が同じ頂点」だけを重複としてまとめる
            // モーフを持つMeshでは、見た目が同じでもFBX内の頂点番号が違う(=モーフの動きが違う)頂点をまとめないように、
            // FBX内の頂点番号もStreamとして渡す
            std::vector<meshopt_Stream> l_streamList = {};

            l_streamList.push_back({ a_modelMesh.m_modelVertexList.data(), l_modelVertexSize, l_modelVertexSize });

            if (a_sourceVertexIndexList)
            {
                l_streamList.push_back({ a_sourceVertexIndexList->data(), sizeof(std::uint32_t), sizeof(std::uint32_t) });
            }

            // meshopt_generateVertexRemapMulti(古い頂点番号から新しい頂点番号への対応表を書き込む配列、
            //                                  現在のインデックス配列、
            //                                  現在のインデックス数、
            //                                  現在の頂点数、
            //                                  Streamの配列、
            //                                  Streamの数);
            return meshopt_generateVertexRemapMulti(a_vertexRemapList.data(),
                                                    a_modelMesh.m_indexList.data(),
                                                    a_modelMesh.m_indexList.size(),
                                                    a_modelMesh.m_modelVertexList.size(),
                                                    l_streamList.data(),
                                                    l_streamList.size());
        }

        void RemapVertexStreamList(const std::vector<std::uint32_t>&          a_vertexRemapList,
                                   const std::size_t                          a_remappedVertexCount,
                                         typename ModelRecordType::ModelMesh& a_modelMesh,
                                         std::vector<std::uint32_t>*          a_sourceVertexIndexList) const
        {
            RemapVertexStream(a_vertexRemapList, a_remappedVertexCount, a_modelMesh.m_modelVertexList);

            if (!a_sourceVertexIndexList) { return; }

            RemapVertexStream(a_vertexRemapList, a_remappedVertexCount, *a_sourceVertexIndexList);
        }

        template <typename ElementType>
        void RemapVertexStream(const std::vector<std::uint32_t>& a_vertexRemapList, const std::size_t a_remappedVertexCount, std::vector<ElementType>& a_vertexStream) const
        {
            std::vector<ElementType> l_remappedVertexStream = {};

            l_remappedVertexStream.resize(a_remappedVertexCount);

            // Remap表に従って、頂点ごとのデータを新しい番号の位置へ移す
            // meshopt_remapVertexBuffer(並び替え後の書き込み先、
            //                           現在の配列、
            //                           現在の要素数、
            //                           1要素のバイトサイズ、
            //                           Remap表);
            meshopt_remapVertexBuffer(l_remappedVertexStream.data(),
                                      a_vertexStream.data(),
                                      a_vertexStream.size(),
                                      sizeof(ElementType),
                                      a_vertexRemapList.data());

            a_vertexStream = std::move(l_remappedVertexStream);
        }

        // モーフを持つModelMeshなら、頂点ごとのFBX内の頂点番号の一覧を返す
        // モーフを持たないModelMesh(StaticModelなど)や、一覧の数が頂点数と合わない場合はnullptrを返す
        std::vector<std::uint32_t>* FetchMutablePTRMorphSourceVertexIndexList(typename ModelRecordType::ModelMesh& a_modelMesh) const
        {
            if constexpr (Concept::IsModelMorphMeshConcept<typename ModelRecordType::ModelMesh>)
            {
                auto& l_sourceVertexIndexList = a_modelMesh.m_modelMorphData.m_sourceVertexIndexList;

                if (l_sourceVertexIndexList.size() == a_modelMesh.m_modelVertexList.size()) { return &l_sourceVertexIndexList; }
            }

            return nullptr;
        }

        static constexpr std::size_t k_invalidOptimizedVertexCount = 0ULL;
    };
}