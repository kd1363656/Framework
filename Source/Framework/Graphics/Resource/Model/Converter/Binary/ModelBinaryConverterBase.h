#pragma once

namespace FWK::Converter
{
    class ModelBinaryConverterBase : public BinaryConverterBase
    {
    public:

         ModelBinaryConverterBase()          = default;
        ~ModelBinaryConverterBase() override = default;

    protected:

        bool CanLoad(const std::filesystem::path& a_filePath) const;

        template <typename ModelMeshType>
        Struct::ModelMeshBinaryHeader CreateModelMeshBinaryHeader(const ModelMeshType& a_modelMesh) const
        {
            Struct::ModelMeshBinaryHeader l_modelMeshBinaryHeader = {};

            const auto& l_materialAssetData = a_modelMesh.m_material.m_assetData;
            const auto& l_meshletData       = a_modelMesh.m_meshletData;

            // 頂点・Index数。
            l_modelMeshBinaryHeader.m_vertexCount = a_modelMesh.m_vertexList.size();
            l_modelMeshBinaryHeader.m_indexCount  = a_modelMesh.m_indexList.size ();

            // Materialが参照しているTexturePathのバイナリ保存サイズ。
            // std::wstringは可変長なので、Headerに保存サイズを持たせておく
            l_modelMeshBinaryHeader.m_baseColorTextureFilePathSize = CalculateWStringBinaryFileSize(l_materialAssetData.m_baseColorTextureFilePath);
            l_modelMeshBinaryHeader.m_normalTextureFilePathSize    = CalculateWStringBinaryFileSize(l_materialAssetData.m_normalTextureFilePath);
            l_modelMeshBinaryHeader.m_roughnessTextureFilePathSize = CalculateWStringBinaryFileSize(l_materialAssetData.m_roughnessTextureFilePath);
            l_modelMeshBinaryHeader.m_metallicTextureFilePathSize  = CalculateWStringBinaryFileSize(l_materialAssetData.m_metallicTextureFilePath);

            // Meshlet関連データ数。
            l_modelMeshBinaryHeader.m_meshletCount           = l_meshletData.m_meshletList.size          ();
            l_modelMeshBinaryHeader.m_uniqueVertexIndexCount = l_meshletData.m_uniqueVertexIndexList.size();
            l_modelMeshBinaryHeader.m_primitiveIndexCount    = l_meshletData.m_primitiveIndexList.size   ();
            l_modelMeshBinaryHeader.m_meshletBoundsCount     = l_meshletData.m_meshletBoundsList.size    ();

            return l_modelMeshBinaryHeader;
        }

        template <typename ModelMeshType>
        bool TryReadModelMeshBinaryDataCommon(ModelMeshType& a_modelMesh, std::uint64_t& a_memoryReadOffset) const
        {
            Struct::ModelMeshBinaryHeader l_modelMeshBinaryHeader = {};

            // ModelMesh単位Headerを読み込む
            // 頂点数、Index数、TexturePathサイズ、Meshlet関連配列数が入っている
            if (!TryReadSingleBinaryData(l_modelMeshBinaryHeader, a_memoryReadOffset)) { return false; }

            // ModelMeshの頂点配列を読み込む
            // StaticModelならStatic用頂点、SkeletalAnimationModelならBoneIndex/BoneWeight付き頂点になる
            if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_vertexCount, a_modelMesh.m_vertexList, a_memoryReadOffset)) { return false; }

            // 通常Index配列を読み込む
            // Meshlet生成元、または通常Index描画用のIndex
            if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_indexCount, a_modelMesh.m_indexList, a_memoryReadOffset)) { return false; }

            auto& l_materialAssetData = a_modelMesh.m_material.m_assetData;

            // MaterialのPBR係数を読み込む
            // Texture本体ではなく、.assetに直接保存できる固定長データ
            if (!TryReadSingleBinaryData(l_materialAssetData.m_baseColorFactor, a_memoryReadOffset)) { return false; }
            if (!TryReadSingleBinaryData(l_materialAssetData.m_roughnessFactor, a_memoryReadOffset)) { return false; }
            if (!TryReadSingleBinaryData(l_materialAssetData.m_metallicFactor,  a_memoryReadOffset)) { return false; }

            // Materialが参照するTexturePathを読み込む
            // std::wstringは可変長なので、Headerに保存されたサイズを使って読む
            if (!TryReadWStringBinaryData(l_modelMeshBinaryHeader.m_baseColorTextureFilePathSize, l_materialAssetData.m_baseColorTextureFilePath, a_memoryReadOffset)) { return false; }
            if (!TryReadWStringBinaryData(l_modelMeshBinaryHeader.m_normalTextureFilePathSize,    l_materialAssetData.m_normalTextureFilePath,    a_memoryReadOffset)) { return false; }
            if (!TryReadWStringBinaryData(l_modelMeshBinaryHeader.m_roughnessTextureFilePathSize, l_materialAssetData.m_roughnessTextureFilePath, a_memoryReadOffset)) { return false; }
            if (!TryReadWStringBinaryData(l_modelMeshBinaryHeader.m_metallicTextureFilePathSize,  l_materialAssetData.m_metallicTextureFilePath,  a_memoryReadOffset)) { return false; }

            // Texture本体は.assetに保存しない
            // RuntimeDataはTextureSystem登録時に別途設定する
            a_modelMesh.m_material.m_runtimeData = {};

            auto& l_meshletData = a_modelMesh.m_meshletData;

            // Meshlet本体を読み込む
            // 各Meshletの頂点範囲、三角形範囲が入っている
            if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_meshletCount, l_meshletData.m_meshletList, a_memoryReadOffset)) { return false; }

            // Meshlet内LocalVertexIndexからModelVertexIndexへ変換するIndex配列を読み込む
            if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_uniqueVertexIndexCount, l_meshletData.m_uniqueVertexIndexList, a_memoryReadOffset)) { return false; }

            // Meshlet内の三角形Index情報を読み込む
            if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_primitiveIndexCount, l_meshletData.m_primitiveIndexList, a_memoryReadOffset)) { return false; }

            // Meshlet単位のカリング用Boundsを読み込む
            if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_meshletBoundsCount, l_meshletData.m_meshletBoundsList, a_memoryReadOffset)) { return false; }

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
            WriteBinaryData(l_modelMeshBinaryHeader.m_vertexCount, a_modelMesh.m_vertexList.data(), a_memoryWriteOffset);

            // 通常Index配列を書き込む
            WriteBinaryData(l_modelMeshBinaryHeader.m_indexCount,  a_modelMesh.m_indexList.data(),       a_memoryWriteOffset);

            const auto& l_materialAssetData = a_modelMesh.m_material.m_assetData;

            // MaterialのPBR係数を書き込む
            WriteBinaryData(k_singleBinaryElementCount, &l_materialAssetData.m_baseColorFactor, a_memoryWriteOffset);
            WriteBinaryData(k_singleBinaryElementCount, &l_materialAssetData.m_roughnessFactor, a_memoryWriteOffset);
            WriteBinaryData(k_singleBinaryElementCount, &l_materialAssetData.m_metallicFactor,  a_memoryWriteOffset);

            // Materialが参照するTexturePathを書き込む
            // Texture本体は.assetには保存しない
            WriteWStringBinaryData(l_materialAssetData.m_baseColorTextureFilePath, a_memoryWriteOffset);
            WriteWStringBinaryData(l_materialAssetData.m_normalTextureFilePath,    a_memoryWriteOffset);
            WriteWStringBinaryData(l_materialAssetData.m_roughnessTextureFilePath, a_memoryWriteOffset);
            WriteWStringBinaryData(l_materialAssetData.m_metallicTextureFilePath,  a_memoryWriteOffset);

            const auto& l_meshletData = a_modelMesh.m_meshletData;

            // Meshlet本体を書き込む
            WriteBinaryData(l_modelMeshBinaryHeader.m_meshletCount, l_meshletData.m_meshletList.data(), a_memoryWriteOffset);

            // Meshlet内LocalVertexIndexからModelVertexIndexへ変換するIndex配列を書き込む
            WriteBinaryData(l_modelMeshBinaryHeader.m_uniqueVertexIndexCount, l_meshletData.m_uniqueVertexIndexList.data(), a_memoryWriteOffset);

            // Meshlet内の三角形Index情報を書き込む
            WriteBinaryData(l_modelMeshBinaryHeader.m_primitiveIndexCount, l_meshletData.m_primitiveIndexList.data(), a_memoryWriteOffset);

            // Meshlet単位のカリング用Boundsを書き込む
            WriteBinaryData(l_modelMeshBinaryHeader.m_meshletBoundsCount, l_meshletData.m_meshletBoundsList.data(), a_memoryWriteOffset);
        }

        template <typename ModelMeshType>
        std::uint64_t CalculateModelMeshBinaryFileSizeCommon(const ModelMeshType& a_modelMesh) const
        {
            using ModelVertexType = typename std::remove_cvref_t<decltype(a_modelMesh.m_vertexList)>::value_type;

            const auto& l_materialAssetData = a_modelMesh.m_material.m_assetData;
            const auto& l_meshletData       = a_modelMesh.m_meshletData;

            // ヘッダーサイズの計算(indexSizeなどの入っているヘッダー)
            auto l_modelMeshBinaryFileSize = CalculateBinaryDataSize<Struct::ModelMeshBinaryHeader>(k_singleBinaryElementCount);

            // モデル頂点、インデックスリストのサイズ計算
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<ModelVertexType>(a_modelMesh.m_vertexList.size());
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<std::uint32_t>  (a_modelMesh.m_indexList.size());

            // PBR係数のサイズ計算
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<TypeAlias::Math::Color>(k_singleBinaryElementCount);
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<float>                 (k_singleBinaryElementCount);
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<float>                 (k_singleBinaryElementCount);

            // PBR用テクスチャファイルパスのサイズ計算
            l_modelMeshBinaryFileSize += CalculateWStringBinaryFileSize(l_materialAssetData.m_baseColorTextureFilePath);
            l_modelMeshBinaryFileSize += CalculateWStringBinaryFileSize(l_materialAssetData.m_normalTextureFilePath);
            l_modelMeshBinaryFileSize += CalculateWStringBinaryFileSize(l_materialAssetData.m_roughnessTextureFilePath);
            l_modelMeshBinaryFileSize += CalculateWStringBinaryFileSize(l_materialAssetData.m_metallicTextureFilePath);

            // メッシュレット、及びそのカリング用Boundの計算
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<Struct::ModelMeshlet>      (l_meshletData.m_meshletList.size());
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<std::uint32_t>             (l_meshletData.m_uniqueVertexIndexList.size());
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<std::uint32_t>             (l_meshletData.m_primitiveIndexList.size());
            l_modelMeshBinaryFileSize += CalculateBinaryDataSize<Struct::ModelMeshletBounds>(l_meshletData.m_meshletBoundsList.size());

            return l_modelMeshBinaryFileSize;
        }
    };
}