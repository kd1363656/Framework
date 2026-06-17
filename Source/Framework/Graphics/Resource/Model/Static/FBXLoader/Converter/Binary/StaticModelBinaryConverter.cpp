#include "StaticModelBinaryConverter.h"

bool FWK::Converter::StaticModelBinaryConverter::LoadStaticModelAsset(const std::filesystem::path& a_filePath, Struct::StaticModelData& a_staticModelData)
{
    a_staticModelData.m_modelMeshList.clear();

    // .assetが存在しない/FBXより古い/音FBXが存在しない場合は、
    // ここでは失敗扱いにして、呼び出し側でFBX読み込みへ進ませる
    if (!CanLoadStaticModelAsset(a_filePath)) { return false; }

    // 読み込み用MemoryMappedFileの作成
    if (const auto& l_staticModelAssetFilePath = CreateAssetFilePath(a_filePath);
        !CreateReadMemoryMappedFile(l_staticModelAssetFilePath))
    {
        return false; 
    }

    auto l_memoryReadOffset = GetREFInitialMemoryReadOffset();

    StaticModelBinaryHeader l_staticModelBinaryHeader = {};

    // バージョン、メッシュ数などを含むStaticModel全体Headerを読み込む
    if (!TryReadSingleBinaryData(l_staticModelBinaryHeader, l_memoryReadOffset)) 
    {
        FailLoadStaticModelAsset(a_staticModelData);

        return false;
    }

    // StaticModel用.assetではないなら読まない。
    if (l_staticModelBinaryHeader.m_assetTypeID != k_staticModelAssetTypeID)
    {
        FailLoadStaticModelAsset(a_staticModelData);

        return false;
    }

    // 保存形式が古いなら、FBXから再生成する
    if (l_staticModelBinaryHeader.m_version != k_staticModelAssetVersion)
    {
        FailLoadStaticModelAsset(a_staticModelData);

        return false;
    }

    // Headerに保存されたファイルサイズと実際の.assetサイズが違う場合は、壊れた.asssetの可能性が高いため読み込まない。
    if (l_staticModelBinaryHeader.m_fileSize != GetREFMappedDataSize())
    {
        FailLoadStaticModelAsset(a_staticModelData);

        return false;
    }

    // メッシュ数が0ならreturn
    if (l_staticModelBinaryHeader.m_modelMeshCount == Constant::k_emptyModelMeshCount)
    {
        FailLoadStaticModelAsset(a_staticModelData);

        return false;
    }

    // メッシュ数分リストをリサイズ
    a_staticModelData.m_modelMeshList.resize(l_staticModelBinaryHeader.m_modelMeshCount);

    for (auto& l_staticModelMesh : a_staticModelData.m_modelMeshList)
    {
        StaticModelMeshBinaryHeader l_staticModelMeshBinaryHeader = {};

        // メッシュ単位Headerを読み込む
        if (!TryReadSingleBinaryData(l_staticModelMeshBinaryHeader, l_memoryReadOffset))
        {
            FailLoadStaticModelAsset(a_staticModelData);

            return false;
        }

        // 頂点配列を読み込む
        if (!TryReadBinaryDataList(l_staticModelMeshBinaryHeader.m_vertexCount, l_memoryReadOffset, l_staticModelMesh.m_staticModelVertexList))
        {
            FailLoadStaticModelAsset(a_staticModelData);

            return false;
        }
        
        // インデックス配列を読み込む
        if (!TryReadBinaryDataList(l_staticModelMeshBinaryHeader.m_indexCount, l_memoryReadOffset, l_staticModelMesh.m_indexList))
        {
            FailLoadStaticModelAsset(a_staticModelData);

            return false;
        }

        auto& l_modelMaterialAssetData = l_staticModelMesh.m_modelMaterial.m_modelMaterialAssetData;
       
        // ベースカラー係数を読み取る
        if (!TryReadSingleBinaryData(l_modelMaterialAssetData.m_baseColorFactor, l_memoryReadOffset))
        {
            FailLoadStaticModelAsset(a_staticModelData);

            return false;
        }

        // ラフネス係数を読み取る
        if (!TryReadSingleBinaryData(l_modelMaterialAssetData.m_roughnessFactor, l_memoryReadOffset))
        {
            FailLoadStaticModelAsset(a_staticModelData);

            return false;
        }

        // メタリック係数を読み取る
        if (!TryReadSingleBinaryData(l_modelMaterialAssetData.m_metallicFactor, l_memoryReadOffset))
        {
            FailLoadStaticModelAsset(a_staticModelData);

            return false;
        }

        // マテリアル用テクスチャファイルパスを読み込む
        // ベースカラーテクスチャ用ファイルパスの文字列を読み取る
        if (!TryReadWStringBinaryData(l_staticModelMeshBinaryHeader.m_baseColorTextureFilePathSize, l_memoryReadOffset, l_modelMaterialAssetData.m_baseColorTextureFilePath))
        {
            FailLoadStaticModelAsset(a_staticModelData);

            return false;
        }

         // ノーマルマップテクスチャ用ファイルパスの文字列を読み取る
        if (!TryReadWStringBinaryData(l_staticModelMeshBinaryHeader.m_normalTextureFilePathSize, l_memoryReadOffset, l_modelMaterialAssetData.m_normalTextureFilePath))
        {
            FailLoadStaticModelAsset(a_staticModelData);

            return false;
        }

         // ラフネステクスチャ用ファイルパスの文字列を読み取る
        if (!TryReadWStringBinaryData(l_staticModelMeshBinaryHeader.m_roughnessTextureFilePathSize, l_memoryReadOffset, l_modelMaterialAssetData.m_roughnessTextureFilePath))
        {
            FailLoadStaticModelAsset(a_staticModelData);

            return false;
        }

         // ラフネステクスチャ用ファイルパスの文字列を読み取る
        if (!TryReadWStringBinaryData(l_staticModelMeshBinaryHeader.m_metallicTextureFilePathSize, l_memoryReadOffset, l_modelMaterialAssetData.m_metallicTextureFilePath))
        {
            FailLoadStaticModelAsset(a_staticModelData);

            return false;
        }

        // Texture本体は.assetに保存しない。
		// TextureSystem登録時にRuntimeDataへ入れる
        l_staticModelMesh.m_modelMaterial.m_modelMaterialRuntimeData = {};

        auto& l_modelMeshletData = l_staticModelMesh.m_modelMeshletData;

        // Meshlet関連データ読み込む
        // メッシュレット情報を読み込む
        if (!TryReadBinaryDataList(l_staticModelMeshBinaryHeader.m_meshletCount, l_memoryReadOffset, l_modelMeshletData.m_meshletList))
        {
            FailLoadStaticModelAsset(a_staticModelData);

            return false;
        }

        // 頂点インデックス情報を読み込む
        if (!TryReadBinaryDataList(l_staticModelMeshBinaryHeader.m_uniqueVertexIndexCount, l_memoryReadOffset, l_modelMeshletData.m_uniqueVertexIndexList))
        {
            FailLoadStaticModelAsset(a_staticModelData);

            return false;
        }

        // プリミティブインデックス情報を読み込む
        if (!TryReadBinaryDataList(l_staticModelMeshBinaryHeader.m_primitiveIndexCount, l_memoryReadOffset, l_modelMeshletData.m_primitiveIndexList))
        {
            FailLoadStaticModelAsset(a_staticModelData);

            return false;
        }

        // メッシュレットカリング情報を読み込む
        if (!TryReadBinaryDataList(l_staticModelMeshBinaryHeader.m_meshletBoundsCount, l_memoryReadOffset, l_modelMeshletData.m_meshletBoundsList))
        {
            FailLoadStaticModelAsset(a_staticModelData);

            return false;
        }
    }

    // 現在のオフセット位置がファイルサイズと一致しなければ.assetファイルが壊れている可能性があるのでreturn
    if (l_memoryReadOffset != l_staticModelBinaryHeader.m_fileSize)
    {
        FailLoadStaticModelAsset(a_staticModelData);

        return false;
    }

    DestroyMemoryMappedFile();

    return true;
}

bool FWK::Converter::StaticModelBinaryConverter::SaveStaticModelAsset(const std::filesystem::path& a_filePath, const Struct::StaticModelData& a_staticModelData)
{
    // StaticModelAssetの元になるFBXが存在しない場合、
    // 何から生成された.assetなのか判断できないため保存しない
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerFBXExtension), "StaticModelAssetの元になるFBXファイルが無効となっており、バイナリーファイルの保存に失敗しました。", false);

    // FBXと同じ場所・同じ名前で拡張子だけ.assetにした保存先を作る
    const auto& l_staticModelAssetFilePath = CreateAssetFilePath(a_filePath);

    // StaticModelData全体を保存したときのファイルサイズを計算する
    const auto& l_staticModelAssetFileSize = CalculateStaticModelAssetFileSize(a_staticModelData);

    // Meshが一つもない、または保存できるデータがない場合は.asset化しない
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_staticModelAssetFileSize == Constant::k_emptyAssetFileSize, "StaticModelAssetへ保存するStaticModelDataが空のため、バイナリーファイルの保存に失敗しました。", false);

    // 書き込み用MemoryMappedFileを作成する
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateWriteMemoryMappedFile(l_staticModelAssetFilePath, l_staticModelAssetFileSize), "StaticModelAssetの書き込み用MemoryMappedfile作成に失敗しました。", false);

    // 現在の書き込み位置
    // ファイル先頭から順番にHeader -> MeshData...と書き込む
    auto l_memoryWriteOffset = GetREFInitialMemoryWriteOffset();

    // StaticModel全体のHeaderを作成して書き込む
    const auto& l_staticModelBinaryHeader = CreateStaticModelBinaryHeader(a_staticModelData, l_staticModelAssetFileSize);

    WriteBinaryData(GetREFSingleBinaryElementCount(), &l_staticModelBinaryHeader, l_memoryWriteOffset);

    for (const auto& l_staticModelMesh : a_staticModelData.m_modelMeshList)
    {
        // Mesh単位のHeaderを作成して書き込む
        // このHeaderにはVertex数/Index数/TexturePathサイズ/Meshlet数などが入る
        const auto& l_staticModelMeshBinaryHeader = CreateStaticModelMeshBinaryHeader(l_staticModelMesh);

        // StaticModelMeshHeaderを書き込む
        WriteBinaryData(GetREFSingleBinaryElementCount(), &l_staticModelMeshBinaryHeader, l_memoryWriteOffset);

        // StaticModel配列を書き込む
        WriteBinaryData(l_staticModelMeshBinaryHeader.m_vertexCount, l_staticModelMesh.m_staticModelVertexList.data(), l_memoryWriteOffset);

        // 通常Index配列を書き込む
        WriteBinaryData(l_staticModelMeshBinaryHeader.m_indexCount, l_staticModelMesh.m_indexList.data(), l_memoryWriteOffset);

        const auto& l_modelMaterialAssetData = l_staticModelMesh.m_modelMaterial.m_modelMaterialAssetData;

        // PBRMaterial値を書き込む
        // Texture本体ではなく、.assetに保存できる係数だけを保存する
        // ベースカラー係数
        WriteBinaryData(GetREFSingleBinaryElementCount(), &l_modelMaterialAssetData.m_baseColorFactor, l_memoryWriteOffset);

        // ラフネス係数
        WriteBinaryData(GetREFSingleBinaryElementCount(), &l_modelMaterialAssetData.m_roughnessFactor, l_memoryWriteOffset);

        // メタリック係数
        WriteBinaryData(GetREFSingleBinaryElementCount(), &l_modelMaterialAssetData.m_metallicFactor, l_memoryWriteOffset);

        // MaterialTexturePathを書き込む
        // 文字列サイズはStaticModelMeshBinaryHeader側に保存済みなので、
        // ここでは文字列本体だけ書き込む
        WriteWStringBinaryData(l_modelMaterialAssetData.m_baseColorTextureFilePath, l_memoryWriteOffset);
        WriteWStringBinaryData(l_modelMaterialAssetData.m_normalTextureFilePath,    l_memoryWriteOffset);
        WriteWStringBinaryData(l_modelMaterialAssetData.m_roughnessTextureFilePath, l_memoryWriteOffset);
        WriteWStringBinaryData(l_modelMaterialAssetData.m_metallicTextureFilePath,  l_memoryWriteOffset);

        const auto& l_modelMeshletData = l_staticModelMesh.m_modelMeshletData;

        // Meshlet本体を書き込む
        WriteBinaryData(l_staticModelMeshBinaryHeader.m_meshletCount, l_modelMeshletData.m_meshletList.data(), l_memoryWriteOffset);

        // Meshlet無いLocalVErtexIndexからStaticModelVertexUIndexへ変換するIndex配列を書き込む
        WriteBinaryData(l_staticModelMeshBinaryHeader.m_uniqueVertexIndexCount, l_modelMeshletData.m_uniqueVertexIndexList.data(), l_memoryWriteOffset);

        // Meshlet内の三角形情報を書き込む
        WriteBinaryData(l_staticModelMeshBinaryHeader.m_primitiveIndexCount, l_modelMeshletData.m_primitiveIndexList.data(), l_memoryWriteOffset);

        // Meshletカリング用Boundsを書き込む
        WriteBinaryData(l_staticModelMeshBinaryHeader.m_meshletBoundsCount, l_modelMeshletData.m_meshletBoundsList.data(), l_memoryWriteOffset);
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

    const auto& l_staticModelAssetFilePath = CreateAssetFilePath(a_filePath);

    // .assetが存在しないなら、FBXから読み込んで生成する
    if (!Utility::CanLoadFilePath(l_staticModelAssetFilePath, Constant::k_lowerAssetExtension)) { return false; }

    // FBXと同じ場所・同じ名前で拡張子だけ.assetにしたパスを作る。
    if (IsUpdatedSourceFile(a_filePath, l_staticModelAssetFilePath)) { return false; }

    return true;
}

void FWK::Converter::StaticModelBinaryConverter::FailLoadStaticModelAsset(Struct::StaticModelData& a_staticModelData)
{
    // 中途半端に読み込んだModelMeshが残らないように空にする。
    a_staticModelData.m_modelMeshList.clear();

    // 読み込み途中で失敗した場合も、MemoryMappedFileは必ず閉じる
    DestroyMemoryMappedFile();
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