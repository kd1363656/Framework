#include "StaticModelBinaryConverter.h"

bool FWK::Converter::StaticModelBinaryConverter::LoadStaticModelAsset(const std::filesystem::path& a_filePath, Struct::StaticModelData& a_staticModelData)
{
    return false;
}

bool FWK::Converter::StaticModelBinaryConverter::SaveStaticModelAsset(const std::filesystem::path& a_filePath, const Struct::StaticModelData& a_staticModelData)
{
    // StaticModelAssetの元になるFBXが存在しない場合、
    // 何から生成された.assetなのか判断できないため保存しない
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerFBXExtension), "StaticModelAssetの元になるFBXファイルが無効となっており、バイナリーファイルの保存に失敗しました。", false);

    // FBXと同じ場所・同じ名前で拡張子だけ.assetにした保存先を作る
    const auto& l_staticModelAsssetFilePath = CreateAssetFilePath(a_filePath);

    // StaticModelData全体を保存したときのファイルサイズを計算する
    const auto& l_staticModelAssetFileSize = CalculateStaticModelAssetFileSize(a_staticModelData);

    // Meshが一つもない、または保存できるデータがない場合は.asset化しない
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_staticModelAssetFileSize == Constant::k_emptyAssetFileSize, "StaticModelAssetへ保存するStaticModelDataが空のため、バイナリーファイルの保存に失敗しました。", false);

    // 書き込み用MemoryMappedFileを作成する
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateWriteMemoryMappedFile(l_staticModelAsssetFilePath, l_staticModelAssetFileSize), "StaticModelAssetの書き込み用MemoryMappedfile作成に失敗しました。", false);

    // メモリマップされた書き込み先の先頭アドレスを取得する
    auto* l_mappedData = GetMutablePTRMappedData();

    if (!l_mappedData)
    {
        DestroyMemoryMappedFile();

        FWK_ASSERT_RETURN_VALUE("StaticModelAssetの書き込み先メモリが無効となっており、バイナリーファイルの保存に失敗しました。", false);
    }

    // 現在の書き込み位置
    // ファイル先頭から順番にHeader -> MeshData...と書き込む
    auto l_memoryWriteOffset = GetREFInitialMemoryWriteOffset();

    // StaticModel全体のHeaderを作成して書き込む
    const auto& l_staticModelBinaryHeader = CreateStaticModelBinaryHeader(a_staticModelData, l_staticModelAssetFileSize);

    WriteBinaryData(GetREFSingleBinaryElementCount(),
                    &l_staticModelBinaryHeader,
                    l_memoryWriteOffset,
                    l_mappedData);

    for (const auto& l_staticModelMesh : a_staticModelData.m_modelMeshList)
    {
        // Mesh単位のHeaderを作成して書き込む
        // このHeaderにはVertex数/Index数/TexturePathサイズ/Meshlet数などが入る
        const auto& l_staticModelMeshBinaryHeader = CreateStaticModelMeshBinaryHeader(l_staticModelMesh);

        // StaticModelMeshHeaderを書き込む
        WriteBinaryData(GetREFSingleBinaryElementCount(), 
                        &l_staticModelMeshBinaryHeader,
                        l_memoryWriteOffset,
                        l_mappedData);

        // StaticModel配列を書き込む
        WriteBinaryData(l_staticModelMeshBinaryHeader.m_vertexCount,
                        l_staticModelMesh.m_staticModelVertexList.data(),
                        l_memoryWriteOffset,
                        l_mappedData);

        // 通常Index配列を書き込む
        WriteBinaryData(l_staticModelMeshBinaryHeader.m_indexCount,
                        l_staticModelMesh.m_indexList.data(),
                        l_memoryWriteOffset,
                        l_mappedData);

        const auto& l_modelMaterialAssetData = l_staticModelMesh.m_modelMaterial.m_modelMaterialAssetData;

        // PBRMaterial値を書き込む
        // Texture本体ではなく、.assetに保存できる係数だけを保存する
        // ベースカラー係数
        WriteBinaryData(GetREFSingleBinaryElementCount(),
                        &l_modelMaterialAssetData.m_baseColorFactor,
                        l_memoryWriteOffset,
                        l_mappedData);

        // ラフネス係数
        WriteBinaryData(GetREFSingleBinaryElementCount(),
                        &l_modelMaterialAssetData.m_roughnessFactor,
                        l_memoryWriteOffset,
                        l_mappedData);

        // メタリック係数
        WriteBinaryData(GetREFSingleBinaryElementCount(),
                        &l_modelMaterialAssetData.m_metallicFactor,
                        l_memoryWriteOffset,
                        l_mappedData);

        // MaterialTexturePathを書き込む
        // 文字列サイズはStaticModelMeshBinaryHeader側に保存済みなので、
        // ここでは文字列本体だけ書き込む
        WriteWStringBinaryData(l_modelMaterialAssetData.m_baseColorTextureFilePath, l_memoryWriteOffset, l_mappedData);
        WriteWStringBinaryData(l_modelMaterialAssetData.m_normalTextureFilePath,    l_memoryWriteOffset, l_mappedData);
        WriteWStringBinaryData(l_modelMaterialAssetData.m_roughnessTextureFilePath, l_memoryWriteOffset, l_mappedData);
        WriteWStringBinaryData(l_modelMaterialAssetData.m_metallicTextureFilePath,  l_memoryWriteOffset, l_mappedData);

        const auto& l_modelMeshletData = l_staticModelMesh.m_modelMeshletData;

        // Meshlet本体を書き込む
        WriteBinaryData(l_staticModelMeshBinaryHeader.m_meshletCount, 
                        l_modelMeshletData.m_meshletList.data(), 
                        l_memoryWriteOffset,
                        l_mappedData);

        // Meshlet無いLocalVErtexIndexからStaticModelVertexUIndexへ変換するIndex配列を書き込む
        WriteBinaryData(l_staticModelMeshBinaryHeader.m_uniqueVertexIndexCount,
                        l_modelMeshletData.m_uniqueVertexIndexList.data(),
                        l_memoryWriteOffset,
                        l_mappedData);

        // Meshlet内の三角形情報を書き込む
        WriteBinaryData(l_staticModelMeshBinaryHeader.m_primitiveIndexCount,
                        l_modelMeshletData.m_primitiveIndexList.data(),
                        l_memoryWriteOffset,
                        l_mappedData);

        // Meshletカリング用Boundsを書き込む
        WriteBinaryData(l_staticModelMeshBinaryHeader.m_meshletBoundsCount,
                        l_modelMeshletData.m_meshletBoundsList.data(),
                        l_memoryWriteOffset,    
                        l_mappedData);
    }

    // 計算したファイルサイズと実際に書き込んだサイズが一致するか確認する。
    // ここが一致しない場合、保存純化サイズ計算のどこかが間違っている。
    if (l_memoryWriteOffset != l_staticModelAssetFileSize)
    {
        DestroyMemoryMappedFile();

        FWK_ASSERT_RETURN_VALUE("StaticModelAssetの書き込みサイズが計算したファイルサイズと一致せず、バイナリーファイルの保存に失敗しました。", false);
    }

    DestroyMemoryMappedFile();

    return true;
}

bool FWK::Converter::StaticModelBinaryConverter::CanLoadStaticModelAsset(const std::filesystem::path& a_filePath) const
{
    // 元となるFBXが存在しない場合は、.assetの正当性を判断できないので読み込まない。
    if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerFBXExtension)) { return false; }

    // FBXと同じ場所・同じ名前で拡張子だけ.assetにしたパスを作る。
    if (const auto& l_staticModelAssetFilePath = CreateAssetFilePath(a_filePath);
        IsUpdatedSourceFile(a_filePath, l_staticModelAssetFilePath)) 
    {
        return false; 
    }

    return true;
}

FWK::Converter::StaticModelBinaryConverter::StaticModelBinaryHeader FWK::Converter::StaticModelBinaryConverter::CreateStaticModelBinaryHeader(const Struct::StaticModelData& a_staticModelData, const std::uint64_t& a_fileSize) const
{
    StaticModelBinaryHeader l_staticModelBinaryHeader = {};

    l_staticModelBinaryHeader.m_fileSize       = a_fileSize;
    l_staticModelBinaryHeader.m_version        = k_staticModelAssetVersion;
    l_staticModelBinaryHeader.m_assetTypeID    = k_staticModelAssetTypeID;
    l_staticModelBinaryHeader.m_modelMeshCount = a_staticModelData.m_modelMeshList.size();

    return l_staticModelBinaryHeader;
}

FWK::Converter::StaticModelBinaryConverter::StaticModelMeshBinaryHeader FWK::Converter::StaticModelBinaryConverter::CreateStaticModelMeshBinaryHeader(const Struct::StaticModelMesh& a_staticModelMesh) const
{
    StaticModelMeshBinaryHeader l_staticModelMeshBinaryHeader = {};

    const auto& l_modelMaterialAssetData = a_staticModelMesh.m_modelMaterial.m_modelMaterialAssetData;
    const auto& l_modelMeshletData       = a_staticModelMesh.m_modelMeshletData;

    // 頂点・Index数。
    l_staticModelMeshBinaryHeader.m_vertexCount = a_staticModelMesh.m_staticModelVertexList.size();
    l_staticModelMeshBinaryHeader.m_indexCount  = a_staticModelMesh.m_indexList.size            ();

    // Materialが参照しているTexturePathのバイナリ保存サイズ。
    // std::wstringは可変長なので、Headerに保存サイズを持たせておく。
    l_staticModelMeshBinaryHeader.m_baseColorTextureFilePathSize = CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_baseColorTextureFilePath);
    l_staticModelMeshBinaryHeader.m_normalTextureFilePathSize    = CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_normalTextureFilePath);
    l_staticModelMeshBinaryHeader.m_roughnessTextureFilePathSize = CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_roughnessTextureFilePath);
    l_staticModelMeshBinaryHeader.m_metallicTextureFilePathSize  = CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_metallicTextureFilePath);

    // Meshlet関連データ数。
    l_staticModelMeshBinaryHeader.m_meshletCount           = l_modelMeshletData.m_meshletList.size          ();
    l_staticModelMeshBinaryHeader.m_uniqueVertexIndexCount = l_modelMeshletData.m_uniqueVertexIndexList.size();
    l_staticModelMeshBinaryHeader.m_primitiveIndexCount    = l_modelMeshletData.m_primitiveIndexList.size   ();
    l_staticModelMeshBinaryHeader.m_meshletBoundsCount     = l_modelMeshletData.m_meshletBoundsList.size    ();

    return l_staticModelMeshBinaryHeader;
}

std::uint64_t FWK::Converter::StaticModelBinaryConverter::CalculateStaticModelAssetFileSize(const Struct::StaticModelData& a_staticModelData) const
{
    // Meshが一つもないStaticModelDataは.asset化しない
    if (a_staticModelData.m_modelMeshList.empty()) { return Constant::k_emptyAssetFileSize; }

    // ファイル先頭に置くStaticModel全体Header
    auto l_staticModelAssetFileSize = CalculateBinaryDataSize<StaticModelBinaryHeader>(GetREFSingleBinaryElementCount());

    for (const auto& l_staticModelMesh : a_staticModelData.m_modelMeshList)
    {
        const auto& l_modelMaterialAssetData = l_staticModelMesh.m_modelMaterial.m_modelMaterialAssetData;
        const auto& l_modelMeshletData       = l_staticModelMesh.m_modelMeshletData;

        // Mesh単位Header
        l_staticModelAssetFileSize += CalculateBinaryDataSize<StaticModelMeshBinaryHeader>(GetREFSingleBinaryElementCount());

        // StaticModel用頂点配列
        l_staticModelAssetFileSize += CalculateBinaryDataSize<Struct::StaticModelVertex>(l_staticModelMesh.m_staticModelVertexList.size());

        // Index配列
        l_staticModelAssetFileSize += CalculateBinaryDataSize<std::uint32_t>(l_staticModelMesh.m_indexList.size());

        // PBRMaterial値
        // Textureではなく、Materialそのものが持つ係数
        l_staticModelAssetFileSize += CalculateBinaryDataSize<TypeAlias::Math::Color>(GetREFSingleBinaryElementCount());
        l_staticModelAssetFileSize += CalculateBinaryDataSize<float>                 (GetREFSingleBinaryElementCount());
        l_staticModelAssetFileSize += CalculateBinaryDataSize<float>                 (GetREFSingleBinaryElementCount());

        // MaterialTexturePath
        l_staticModelAssetFileSize += CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_baseColorTextureFilePath);
        l_staticModelAssetFileSize += CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_normalTextureFilePath);
        l_staticModelAssetFileSize += CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_roughnessTextureFilePath);
        l_staticModelAssetFileSize += CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_metallicTextureFilePath);

        // Meshlet本体
        l_staticModelAssetFileSize += CalculateBinaryDataSize<Struct::StaticModelMeshlet>(l_modelMeshletData.m_meshletList.size());

        // Meshlet内のLocalVertexIndexからStaticModelVertexIndexへ変換するIndex配列
        l_staticModelAssetFileSize += CalculateBinaryDataSize<std::uint32_t>(l_modelMeshletData.m_uniqueVertexIndexList.size());

        // Meshlet内Triangle情報
        l_staticModelAssetFileSize += CalculateBinaryDataSize<std::uint32_t>(l_modelMeshletData.m_primitiveIndexList.size());

        // Meshletカリング用Bounds
        l_staticModelAssetFileSize += CalculateBinaryDataSize<Struct::StaticModelMeshletBounds>(l_modelMeshletData.m_meshletBoundsList.size());
    }

    return l_staticModelAssetFileSize;
}