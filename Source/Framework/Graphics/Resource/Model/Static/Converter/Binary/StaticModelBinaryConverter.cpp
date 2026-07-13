#include "StaticModelBinaryConverter.h"

bool FWK::Converter::StaticModelBinaryConverter::LoadAsset(const std::filesystem::path& a_filePath, Graphics::StaticModelRecord& a_staticModelRecord)
{
    auto& l_staticModelData = a_staticModelRecord.GetMutableREFModelData();

    l_staticModelData.m_modelMeshList.clear();

    // .assetが存在しない/FBXより古い/元FBXが存在しない場合は、
    // ここでは失敗扱いにして、呼び出し側でFBX読み込みへ進ませる
    if (!CanLoadAsset(a_filePath)) { return false; }

    // 読み込み用MemoryMappedFileの作成
    if (const auto& l_staticModelAssetFilePath = CreateAssetFilePath(a_filePath);
        !CreateReadMemoryMappedFile(l_staticModelAssetFilePath))
    {
        return false; 
    }

    auto l_memoryReadOffset = k_initialMemoryReadOffset;

    ModelBinaryHeader l_modelBinaryHeader = {};

    // バージョン、メッシュ数などを含むStaticModel全体Headerを読み込む
    if (!TryReadSingleBinaryData(l_modelBinaryHeader, l_memoryReadOffset)) 
    {
        FailLoadAsset(l_staticModelData);

        return false;
    }

    // Model用.assetではないなら読まない。
    if (l_modelBinaryHeader.m_assetTypeID != k_modelAssetTypeID)
    {
        FailLoadAsset(l_staticModelData);

        return false;
    }

    // 保存形式が古いなら、FBXから再生成する
    if (l_modelBinaryHeader.m_version != k_modelAssetVersion)
    {
        FailLoadAsset(l_staticModelData);

        return false;
    }

    // Headerに保存されたファイルサイズと実際の.assetサイズが違う場合は、壊れた.asssetの可能性が高いため読み込まない。
    if (l_modelBinaryHeader.m_fileSize != GetREFMappedDataSize())
    {
        FailLoadAsset(l_staticModelData);

        return false;
    }

    // メッシュ数が0ならreturn
    if (l_modelBinaryHeader.m_modelMeshCount == Constant::k_emptyModelMeshCount)
    {
        FailLoadAsset(l_staticModelData);

        return false;
    }

    // ModelMeshの読み込み
    l_staticModelData.m_modelMeshList.resize(l_modelBinaryHeader.m_modelMeshCount);

    for (auto& l_staticModelMesh : l_staticModelData.m_modelMeshList)
    {
        if (!TryReadModelMeshBinaryDataCommon(l_staticModelMesh, l_memoryReadOffset))
        {
            FailLoadAsset(l_staticModelData);

            return false;
        }
    }

    // 現在のオフセット位置がファイルサイズと一致しなければ.assetファイルが壊れている可能性があるのでreturn
    if (l_memoryReadOffset != l_modelBinaryHeader.m_fileSize)
    {
        FailLoadAsset(l_staticModelData);

        return false;
    }

    DestroyMemoryMappedFile();

    return true;
}

bool FWK::Converter::StaticModelBinaryConverter::SaveAsset(const Graphics::StaticModelRecord& a_staticModelRecord, const std::filesystem::path& a_filePath)
{
    auto& l_staticModelData = a_staticModelRecord.GetREFModelData();

    // StaticModelAssetの元になるFBXが存在しない場合、
    // 何から生成された.assetなのか判断できないため保存しない
    FWK_ASSERT_RETURN_VALUE_IF(!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerFBXExtension), "Assetの元になるFBXファイルが無効となっており、バイナリーファイルの保存に失敗しました。", false);

    // FBXと同じ場所・同じ名前で拡張子だけ.assetにした保存先を作る
    const auto& l_staticModelAssetFilePath = CreateAssetFilePath(a_filePath);

    // StaticModelData全体を保存したときのファイルサイズを計算する
    const auto& l_modelAssetFileSize = CalculateAssetFileSize(l_staticModelData);

    // Meshが一つもない、または保存できるデータがない場合は.asset化しない
    FWK_ASSERT_RETURN_VALUE_IF(l_modelAssetFileSize == BinaryConverterBase::k_emptyAssetFileSize, "Assetへ保存するStaticModelDataが空のため、バイナリーファイルの保存に失敗しました。", false);

    // 書き込み用MemoryMappedFileを作成する
    FWK_ASSERT_RETURN_VALUE_IF(!CreateWriteMemoryMappedFile(l_staticModelAssetFilePath, l_modelAssetFileSize), "Assetの書き込み用MemoryMappedfile作成に失敗しました。", false);

    // 現在の書き込み位置
    // ファイル先頭から順番にHeader -> MeshData...と書き込む
    auto l_memoryWriteOffset = k_initialMemoryWriteOffset;

    // StaticModel全体のHeaderを作成して書き込む
    const auto& l_modelBinaryHeader = CreateModelBinaryHeader(l_staticModelData, l_modelAssetFileSize);

    WriteBinaryData(k_singleBinaryElementCount, &l_modelBinaryHeader, l_memoryWriteOffset);

    // ModelMeshListを書き込む
    for (const auto& l_staticModelMesh : l_staticModelData.m_modelMeshList)
    {
        WriteModelMeshBinaryDataCommon(l_staticModelMesh, l_memoryWriteOffset);
    }

    // 計算したファイルサイズと実際に書き込んだサイズが一致するか確認する
    // ここが一致しない場合、保存順かサイズ計算のどこかが間違っている
    if (l_memoryWriteOffset != l_modelAssetFileSize)
    {
        DestroyMemoryMappedFile();

        FWK_ASSERT_RETURN_VALUE("Assetの書き込みサイズが計算したファイルサイズと一致せず、バイナリーファイルの保存に失敗しました。", false);
    }

    DestroyMemoryMappedFile();

    return true;
}

void FWK::Converter::StaticModelBinaryConverter::FailLoadAsset(Graphics::StaticModelRecord::ModelData& a_modelData)
{
    // 中途半端に読み込んだModelMeshが残らないように空にする。
    a_modelData.m_modelMeshList.clear();

    // 読み込み途中で失敗した場合も、MemoryMappedFileは必ず閉じる
    DestroyMemoryMappedFile();
}

FWK::Converter::StaticModelBinaryConverter::ModelBinaryHeader FWK::Converter::StaticModelBinaryConverter::CreateModelBinaryHeader(const Graphics::StaticModelRecord::ModelData& a_modelData, const std::uint64_t& a_fileSize) const
{
    ModelBinaryHeader l_modelBinaryHeader = {};

    l_modelBinaryHeader.m_fileSize       = a_fileSize;
    l_modelBinaryHeader.m_version        = k_modelAssetVersion;
    l_modelBinaryHeader.m_assetTypeID    = k_modelAssetTypeID;
    l_modelBinaryHeader.m_modelMeshCount = a_modelData.m_modelMeshList.size();

    return l_modelBinaryHeader;
}

std::uint64_t FWK::Converter::StaticModelBinaryConverter::CalculateAssetFileSize(const Graphics::StaticModelRecord::ModelData& a_modelData) const
{
    // Meshが一つもないStaticModelDataは.asset化しない
    if (a_modelData.m_modelMeshList.empty()) { return BinaryConverterBase::k_emptyAssetFileSize; }

    // ファイル先頭に置くStaticModel全体Header
    auto l_modelAssetFileSize = CalculateBinaryDataSize<ModelBinaryHeader>(k_singleBinaryElementCount);

    for (const auto& l_staticModelMesh : a_modelData.m_modelMeshList)
    {
        l_modelAssetFileSize += CalculateModelMeshBinaryFileSizeCommon(l_staticModelMesh);
    }

    return l_modelAssetFileSize;
}