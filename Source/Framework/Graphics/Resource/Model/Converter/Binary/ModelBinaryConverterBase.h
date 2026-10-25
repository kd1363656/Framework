#pragma once

namespace FWK::Converter
{
    class ModelBinaryConverterBase : public BinaryConverterBase
    {
    protected:

        struct ModelMeshBinaryHeader final
        {
            std::uint64_t m_vertexCount = Constant::k_emptyModelVertexCount;
            std::uint64_t m_indexCount  = Constant::k_emptyModelIndexCount;

            std::uint64_t m_materialFilePathSize = k_emptyMaterialFilePathSize;

            std::uint64_t m_meshletCount           = k_emptyModelMeshletCount;
            std::uint64_t m_uniqueVertexIndexCount = k_emptyModelUniqueVertexIndexCount;
            std::uint64_t m_primitiveIndexCount    = k_emptyModelPrimitiveIndexCount;
            std::uint64_t m_meshletBoundsCount     = k_emptyModelMeshletBoundsCount;
            std::uint64_t m_lodCount               = k_emptyModelLODCount;
        };

    public:

         ModelBinaryConverterBase()          = default;
        ~ModelBinaryConverterBase() override = default;

    protected:

        bool CanLoadAsset(const std::filesystem::path& a_filePath, const std::filesystem::path& a_assetExtension) const;

        template <typename ModelMeshType>
        ModelMeshBinaryHeader CreateModelMeshBinaryHeader(const ModelMeshType& a_modelMesh) const
        {
            ModelMeshBinaryHeader l_modelMeshBinaryHeader = {};

            const auto& l_modelMaterial    = a_modelMesh.m_modelMaterial;
            const auto& l_modelMeshletData = a_modelMesh.m_modelMeshletData;

            // 頂点・Index数。
            l_modelMeshBinaryHeader.m_vertexCount = a_modelMesh.m_modelVertexList.size();
            l_modelMeshBinaryHeader.m_indexCount  = a_modelMesh.m_indexList.size();

            // .materialファイルパスのバイナリ保存サイズ。
            // std::wstringは可変長なので、Headerに保存サイズを持たせておく
            l_modelMeshBinaryHeader.m_materialFilePathSize = CalculateWStringBinaryFileSize(l_modelMaterial.m_materialFilePath);

            // Meshlet関連データ数。
            l_modelMeshBinaryHeader.m_meshletCount           = l_modelMeshletData.m_meshletList.size();
            l_modelMeshBinaryHeader.m_uniqueVertexIndexCount = l_modelMeshletData.m_uniqueVertexIndexList.size();
            l_modelMeshBinaryHeader.m_primitiveIndexCount    = l_modelMeshletData.m_primitiveIndexList.size();
            l_modelMeshBinaryHeader.m_meshletBoundsCount     = l_modelMeshletData.m_meshletBoundsList.size();
            l_modelMeshBinaryHeader.m_lodCount               = l_modelMeshletData.m_lodList.size();

            return l_modelMeshBinaryHeader;
        }

        template <typename ModelMeshType>
        bool TryReadModelMeshBinaryDataCommon(ModelMeshType& a_modelMesh, std::uint64_t& a_memoryReadOffset) const
        {
            ModelMeshBinaryHeader l_modelMeshBinaryHeader = {};

            // ModelMesh単位Headerを読み込む
            // 頂点数、Index数、TexturePathサイズ、Meshlet関連配列数が入っている
            if (!TryReadSingleBinaryData(l_modelMeshBinaryHeader, a_memoryReadOffset)) { return false; }

            // ModelMeshの頂点配列を読み込む
            // StaticModelならStatic用頂点、SkeletalAnimationModelならBoneIndex/BoneWeight付き頂点になる
            if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_vertexCount, a_modelMesh.m_modelVertexList, a_memoryReadOffset)) { return false; }

            // 通常Index配列を読み込む
            // Meshlet生成元、または通常Index描画用のIndex
            if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_indexCount, a_modelMesh.m_indexList, a_memoryReadOffset)) { return false; }

            auto& l_modelMaterial = a_modelMesh.m_modelMaterial;

            // .materialファイルパスを読み込む
            // Materialの値やTexturePathは.materialファイル側に保存されているため、モデルのバイナリーファイルにはパスだけを持つ
            // std::wstringは可変長なので、Headerに保存されたサイズを使って読む
            if (!TryReadWStringBinaryData(l_modelMeshBinaryHeader.m_materialFilePathSize, l_modelMaterial.m_materialFilePath, a_memoryReadOffset)) { return false; }

            // Material本体はモデルのバイナリーファイルに保存しない
            // 実行時にMaterialSystemから読み込む
            l_modelMaterial.m_material = {};

            auto& l_modelMeshletData = a_modelMesh.m_modelMeshletData;

            // Meshlet本体を読み込む
            // 各Meshletの頂点範囲、三角形範囲が入っている
            if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_meshletCount, l_modelMeshletData.m_meshletList, a_memoryReadOffset)) { return false; }

            // Meshlet内LocalVertexIndexからModelVertexIndexへ変換するIndex配列を読み込む
            if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_uniqueVertexIndexCount, l_modelMeshletData.m_uniqueVertexIndexList, a_memoryReadOffset)) { return false; }

            // Meshlet内の三角形Index情報を読み込む
            if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_primitiveIndexCount, l_modelMeshletData.m_primitiveIndexList, a_memoryReadOffset)) { return false; }

            // Meshlet単位のカリング用Boundsを読み込む
            if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_meshletBoundsCount, l_modelMeshletData.m_meshletBoundsList, a_memoryReadOffset)) { return false; }

            // LOD一覧と、LOD選択で使うMesh全体の囲み球
            if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_lodCount, l_modelMeshletData.m_lodList, a_memoryReadOffset)) { return false; }

            if (!TryReadSingleBinaryData(l_modelMeshletData.m_boundsCenter, a_memoryReadOffset)) { return false; }
            if (!TryReadSingleBinaryData(l_modelMeshletData.m_boundsRadius, a_memoryReadOffset)) { return false; }

            // 頂点ごとの覆われている層の数(頂点数と同じ数)と、LODで表面が実際にずれた量の倍率
            if (!TryReadBinaryDataList (l_modelMeshBinaryHeader.m_vertexCount, l_modelMeshletData.m_lodCoveredLayerCountList, a_memoryReadOffset)) { return false; }
            if (!TryReadSingleBinaryData(l_modelMeshletData.m_lodSurfaceDeviationScale, a_memoryReadOffset))                                         { return false; }

            return true;
        }

        template <typename ModelMeshType>
        void WriteModelMeshBinaryDataCommon(const ModelMeshType& a_modelMesh, std::uint64_t& a_memoryWriteOffset) const
        {
            const auto& l_modelMeshBinaryHeader = CreateModelMeshBinaryHeader(a_modelMesh);

            // ModelMesh単位Headerを書き込む
            // この後に続く可変長配列やTexturePathのサイズ情報を持つ
            WriteBinaryData(k_singleBinaryElementCount, &l_modelMeshBinaryHeader, a_memoryWriteOffset);

            // ModelMeshの頂点配列を書き込む
            WriteBinaryData(l_modelMeshBinaryHeader.m_vertexCount, a_modelMesh.m_modelVertexList.data(), a_memoryWriteOffset);

            // 通常Index配列を書き込む
            WriteBinaryData(l_modelMeshBinaryHeader.m_indexCount,  a_modelMesh.m_indexList.data(),       a_memoryWriteOffset);

            const auto& l_modelMaterial = a_modelMesh.m_modelMaterial;

            // .materialファイルパスを書き込む
            // Material本体はモデルのバイナリーファイルには保存しない
            WriteWStringBinaryData(l_modelMaterial.m_materialFilePath, a_memoryWriteOffset);

            const auto& l_modelMeshletData = a_modelMesh.m_modelMeshletData;

            // Meshlet本体を書き込む
            WriteBinaryData(l_modelMeshBinaryHeader.m_meshletCount, l_modelMeshletData.m_meshletList.data(), a_memoryWriteOffset);

            // Meshlet内LocalVertexIndexからModelVertexIndexへ変換するIndex配列を書き込む
            WriteBinaryData(l_modelMeshBinaryHeader.m_uniqueVertexIndexCount, l_modelMeshletData.m_uniqueVertexIndexList.data(), a_memoryWriteOffset);

            // Meshlet内の三角形Index情報を書き込む
            WriteBinaryData(l_modelMeshBinaryHeader.m_primitiveIndexCount, l_modelMeshletData.m_primitiveIndexList.data(), a_memoryWriteOffset);

            // Meshlet単位のカリング用Boundsを書き込む
            WriteBinaryData(l_modelMeshBinaryHeader.m_meshletBoundsCount, l_modelMeshletData.m_meshletBoundsList.data(), a_memoryWriteOffset);

            // LOD一覧と、LOD選択で使うMesh全体の囲み球
            WriteBinaryData(l_modelMeshBinaryHeader.m_lodCount, l_modelMeshletData.m_lodList.data(), a_memoryWriteOffset);

            WriteBinaryData(k_singleBinaryElementCount, &l_modelMeshletData.m_boundsCenter, a_memoryWriteOffset);
            WriteBinaryData(k_singleBinaryElementCount, &l_modelMeshletData.m_boundsRadius, a_memoryWriteOffset);

            // 頂点ごとの覆われている層の数(頂点数と同じ数)と、LODで表面が実際にずれた量の倍率
            WriteBinaryData(l_modelMeshBinaryHeader.m_vertexCount, l_modelMeshletData.m_lodCoveredLayerCountList.data(), a_memoryWriteOffset);
            WriteBinaryData(k_singleBinaryElementCount,            &l_modelMeshletData.m_lodSurfaceDeviationScale,      a_memoryWriteOffset);
        }

        template <typename ModelMeshType>
        std::uint64_t CalculateModelMeshBinaryFileSizeCommon(const ModelMeshType& a_modelMesh) const
        {
            using ModelVertexType = typename std::remove_cvref_t<decltype(a_modelMesh.m_modelVertexList)>::value_type;

            const auto& l_modelMaterial    = a_modelMesh.m_modelMaterial;
            const auto& l_modelMeshletData = a_modelMesh.m_modelMeshletData;

            // ヘッダーサイズの計算(indexSizeなどの入っているヘッダー)
            auto l_modelMeshBinaryFileSize = CalculateBinaryDataSize<ModelMeshBinaryHeader>(k_singleBinaryElementCount);

            // モデル頂点、インデックスリストのサイズ計算
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<ModelVertexType>(a_modelMesh.m_modelVertexList.size());
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<std::uint32_t>  (a_modelMesh.m_indexList.size());

            // .materialファイルパスのサイズ計算
            l_modelMeshBinaryFileSize += CalculateWStringBinaryFileSize(l_modelMaterial.m_materialFilePath);

            // メッシュレット、及びそのカリング用Boundの計算
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<Struct::ModelMeshlet>      (l_modelMeshletData.m_meshletList.size());
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<std::uint32_t>             (l_modelMeshletData.m_uniqueVertexIndexList.size());
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<std::uint32_t>             (l_modelMeshletData.m_primitiveIndexList.size());
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<Struct::ModelMeshletBounds>(l_modelMeshletData.m_meshletBoundsList.size());

            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<Struct::ModelMeshLOD>    (l_modelMeshletData.m_lodList.size());
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<TypeAlias::Math::Vector3>(k_singleBinaryElementCount);
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<float>                   (k_singleBinaryElementCount);
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<std::uint32_t>           (l_modelMeshletData.m_lodCoveredLayerCountList.size());
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<float>                   (k_singleBinaryElementCount);

            return l_modelMeshBinaryFileSize;
        }

        static constexpr std::uint64_t k_emptyMaterialFilePathSize        = 0ULL;
        static constexpr std::uint64_t k_emptyModelMeshletCount           = 0ULL;
        static constexpr std::uint64_t k_emptyModelUniqueVertexIndexCount = 0ULL;
        static constexpr std::uint64_t k_emptyModelPrimitiveIndexCount    = 0ULL;
        static constexpr std::uint64_t k_emptyModelMeshletBoundsCount     = 0ULL;
        static constexpr std::uint64_t k_emptyModelLODCount               = 0ULL;
    };
}