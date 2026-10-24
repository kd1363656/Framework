#pragma once

namespace FWK::Graphics
{
    // GPUドリブン描画で使う1フレーム分の表(インスタンス、Mesh、LOD)を作るクラス
    //
    // 描画申請(モデル1体ごとのワールド行列など)を受け取り、同じモデルのインスタンスが
    // 表の中で連続して並ぶようにまとめる。
    // Meshの情報とLODは、モデルが初めて出てきたときに1回だけ表へ追加する(全インスタンスで共通のため)
    //
    // StaticModel、SkeletalAnimationModelのどちらでも使えるように、
    // モデルの種類(Record)はテンプレートで受け取る
    class ModelGPUDrivenTableBuilder final
    {
    public:

         ModelGPUDrivenTableBuilder() = default;
        ~ModelGPUDrivenTableBuilder() = default;

        // 前のフレームの表を消し、新しいフレームの表を作り始める
        void Clear();

        // インスタンス(モデル1体)を表へ追加する
        // a_skinningInfo : スキニングするモデルの場合のMeshごとの追加情報(スキニングしない場合はnullptr)
        template <typename ModelRecordType>
        void AddInstance(const ModelRecordType& a_modelRecord, const Struct::SBModelGPUDrivenInstance& a_instance, const Struct::ModelGPUDrivenSkinningInfo* a_skinningInfo)
        {
            auto l_batchIndexItr = m_modelBatchIndexMap.find(&a_modelRecord);

            // 初めて出てきたモデルなら、そのモデルのMeshとLODの情報を表へ追加する
            if (l_batchIndexItr == m_modelBatchIndexMap.end())
            {
                Struct::ModelGPUDrivenBatch l_modelBatch = {};

                AppendModelMeshInfo(a_modelRecord, a_skinningInfo, l_modelBatch);

                // 描画するMeshが無い(表に入りきらない、又はこの描画で扱うMaterialのMeshが無い)モデルは描画しない
                // 同じモデルの2体目以降で調べ直さないように、描画しないモデルとして覚えておく
                if (l_modelBatch.m_meshCount == Struct::ModelGPUDrivenBatch::k_initialValue)
                {
                    m_modelBatchIndexMap.try_emplace(&a_modelRecord, k_skippedBatchIndex);

                    return;
                }

                l_batchIndexItr = m_modelBatchIndexMap.try_emplace(&a_modelRecord, m_frameTable.m_batchList.size()).first;

                m_frameTable.m_batchList.emplace_back(l_modelBatch);

                if (m_batchInstanceListList.size() < m_frameTable.m_batchList.size())
                {
                    m_batchInstanceListList.emplace_back();
                }
            }

            if (l_batchIndexItr->second == k_skippedBatchIndex) { return; }

            m_batchInstanceListList[l_batchIndexItr->second].emplace_back(a_instance);
        }

        // 追加したインスタンスを、同じモデルが連続して並ぶように1本の表へまとめて返す
        const Struct::ModelGPUDrivenFrameTable& Build();

        // ワールド行列から、インスタンスの共通部分(行列、拡大率、裏返り)を作る
        Struct::SBModelGPUDrivenInstance CreateInstance(const TypeAlias::Math::Matrix& a_worldMatrix, const TypeAlias::Math::Matrix& a_worldInverseTransposeMatrix) const;

        // 表へ追加するMeshのMaterialの種類(無効値の場合はMaterialの種類を問わず全てのMeshを追加する)
        void SetTargetMaterialStaticTypeID(const TypeAlias::StaticTypeID a_set) { m_targetMaterialStaticTypeID = a_set; }

    private:

        template <typename ModelRecordType>
        void AppendModelMeshInfo(const ModelRecordType& a_modelRecord, const Struct::ModelGPUDrivenSkinningInfo* a_skinningInfo, Struct::ModelGPUDrivenBatch& a_modelBatch)
        {
            const auto& l_modelMeshList = a_modelRecord.GetREFModelData().m_modelMeshList;

            auto& l_meshInfoList = m_frameTable.m_meshInfoList;
            auto& l_lodList      = m_frameTable.m_lodList;

            // 表に入りきらない場合はこのモデルを追加しない
            if (l_meshInfoList.size() + l_modelMeshList.size() > ModelGPUDrivenDrawer::k_maxMeshInfoCount) { return; }

            a_modelBatch.m_meshInfoOffset = static_cast<std::uint32_t>(l_meshInfoList.size());

            for (std::size_t l_meshIndex = 0ULL; l_meshIndex < l_modelMeshList.size(); ++l_meshIndex)
            {
                const auto& l_modelMesh            = l_modelMeshList[l_meshIndex];
                const auto& l_modelMeshletData     = l_modelMesh.m_modelMeshletData;
                const auto& l_modelMeshRuntimeData = l_modelMesh.m_modelMeshRuntimeData;
                const auto& l_modelMaterial        = l_modelMesh.m_modelMaterial;
                const auto& l_material             = l_modelMaterial.m_material.GetREFMaterial().lock();

                // この描画で扱わない種類のMaterialを使うMeshは、表へ追加しない(別のPassが描画する)
                if (!Utility::IsDrawTargetMaterial(l_material, m_targetMaterialStaticTypeID)) { continue; }

                Struct::SBModelGPUDrivenMeshInfo l_meshInfo = {};

                l_meshInfo.m_boundsCenter = l_modelMeshletData.m_boundsCenter;
                l_meshInfo.m_boundsRadius = l_modelMeshletData.m_boundsRadius;

                // Materialの種類を知らなくても、GPUDataBufferの番号を渡すだけでシェーダー側が自身の型として読む
                l_meshInfo.m_materialBufferSRVDescriptorIndex = l_material->GetVALGPUDataBufferSRVDescriptorIndex();

                l_meshInfo.m_vertexBufferSRVDescriptorIndex            = l_modelMeshRuntimeData.m_vertexBuffer.GetVALSRVDescriptorIndex           ();
                l_meshInfo.m_meshletBufferSRVDescriptorIndex           = l_modelMeshRuntimeData.m_meshletBuffer.GetVALSRVDescriptorIndex          ();
                l_meshInfo.m_uniqueVertexIndexBufferSRVDescriptorIndex = l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer.GetVALSRVDescriptorIndex();
                l_meshInfo.m_primitiveIndexBufferSRVDescriptorIndex    = l_modelMeshRuntimeData.m_primitiveIndexBuffer.GetVALSRVDescriptorIndex   ();
                l_meshInfo.m_meshletBoundsBufferSRVDescriptorIndex     = l_modelMeshRuntimeData.m_meshletBoundsBuffer.GetVALSRVDescriptorIndex    ();

                l_meshInfo.m_allMeshletCount  = static_cast<std::uint32_t>(l_modelMeshletData.m_meshletList.size());
                l_meshInfo.m_allTriangleCount = static_cast<std::uint32_t>(l_modelMesh.m_indexList.size() / Constant::k_triangleVertexCount);

                // LODが表に入りきる場合だけLODを使う(入りきらなければ元の形で描画する)
                if (l_lodList.size() + l_modelMeshletData.m_lodList.size() <= ModelGPUDrivenDrawer::k_maxLODCount)
                {
                    l_meshInfo.m_lodOffset = static_cast<std::uint32_t>(l_lodList.size());
                    l_meshInfo.m_lodCount  = static_cast<std::uint32_t>(l_modelMeshletData.m_lodList.size());

                    l_lodList.insert(l_lodList.end(), l_modelMeshletData.m_lodList.begin(), l_modelMeshletData.m_lodList.end());
                }

                // スキニングするモデルの場合は、スキニング行列の場所と、動いても全頂点を囲める球の半径を設定する
                if (a_skinningInfo                                         &&
                    l_meshIndex < a_skinningInfo->m_paletteBaseList.size() &&
                    l_meshIndex < a_skinningInfo->m_boundsRadiusList.size())
                {
                    l_meshInfo.m_boundsRadius                           = a_skinningInfo->m_boundsRadiusList[l_meshIndex];
                    l_meshInfo.m_skinningMatrixBufferSRVDescriptorIndex = a_skinningInfo->m_skinningMatrixBufferSRVDescriptorIndex;
                    l_meshInfo.m_skinningPaletteBase                    = a_skinningInfo->m_paletteBaseList[l_meshIndex];
                    l_meshInfo.m_skinningPaletteStride                  = a_skinningInfo->m_paletteStride;
                    l_meshInfo.m_skinningFrameCount                     = a_skinningInfo->m_frameCount;
                }

                l_meshInfoList.emplace_back(l_meshInfo);
            }

            // 表へ追加したMeshの数(この描画で扱わないMaterialのMeshは含まない)
            a_modelBatch.m_meshCount = static_cast<std::uint32_t>(l_meshInfoList.size()) - a_modelBatch.m_meshInfoOffset;
        }

        // 描画しないモデルを表す、まとまりの番号
        static constexpr std::size_t k_skippedBatchIndex = std::numeric_limits<std::size_t>::max();

        // 作成中の表
        Struct::ModelGPUDrivenFrameTable m_frameTable = {};

        // モデルの種類ごとのインスタンスの一時置き場(メモリ確保を減らすため使い回す)
        std::vector<std::vector<Struct::SBModelGPUDrivenInstance>> m_batchInstanceListList = {};

        // モデル(Record)のアドレス → 表の中でのまとまりの番号
        std::unordered_map<const void*, std::size_t> m_modelBatchIndexMap = {};

        TypeAlias::StaticTypeID m_targetMaterialStaticTypeID = StaticTypeIDGenerator::k_invalidStaticTypeID;
    };
}