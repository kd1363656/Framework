#include "SkeletalAnimationModelMorphSettingBinaryConverter.h"

bool FWK::Converter::SkeletalAnimationModelMorphSettingBinaryConverter::Load(Graphics::SkeletalAnimationModelMorphSetting& a_morphSetting, const std::filesystem::path& a_modelFilePath)
{
    const auto& l_morphSettingFilePath = CreateAssetFilePath(a_modelFilePath, Constant::k_skeletalAnimationModelMorphSettingExtension);

    if (!Utility::CanLoadFilePath(l_morphSettingFilePath, Constant::k_skeletalAnimationModelMorphSettingExtension)) { return false; }

    if (!CreateReadMemoryMappedFile(l_morphSettingFilePath)) { return false; }

    auto l_memoryReadOffset = k_initialMemoryReadOffset;

    MorphSettingBinaryHeader l_morphSettingBinaryHeader = {};

    // .morphではない、保存形式が古い、ファイルサイズが合わないファイルは読み込まない
    if (!TryReadSingleBinaryData(l_morphSettingBinaryHeader, l_memoryReadOffset)        ||
        l_morphSettingBinaryHeader.m_assetTypeID != k_morphSettingAssetTypeID           ||
        l_morphSettingBinaryHeader.m_version     != k_morphSettingAssetVersion          ||
        l_morphSettingBinaryHeader.m_fileSize    != GetVALMappedDataSize())
    {
        DestroyMemoryMappedFile();

        return false;
    }

    // 途中で失敗した場合に設定が中途半端にならないよう、全て読めてから設定へ渡す
    std::vector<Struct::ModelMorphTargetSetting> l_targetSettingList = {};

    l_targetSettingList.resize(l_morphSettingBinaryHeader.m_targetSettingCount);

    for (auto& l_targetSetting : l_targetSettingList)
    {
        TargetSettingBinaryData l_targetSettingBinaryData = {};

        // 名前以外の設定 -> 名前の順に読み込む
        if (!TryReadSingleBinaryData (l_targetSettingBinaryData, l_memoryReadOffset)                                                  ||
            !TryReadWStringBinaryData(l_targetSettingBinaryData.m_targetNameSize, l_targetSetting.m_targetName, l_memoryReadOffset))
        {
            DestroyMemoryMappedFile();

            return false;
        }

        l_targetSetting.m_lodProtection   = l_targetSettingBinaryData.m_lodProtection;
        l_targetSetting.m_defaultWeight   = l_targetSettingBinaryData.m_defaultWeight;
        l_targetSetting.m_minWeight       = l_targetSettingBinaryData.m_minWeight;
        l_targetSetting.m_maxWeight       = l_targetSettingBinaryData.m_maxWeight;
        l_targetSetting.m_isShadowEnabled = l_targetSettingBinaryData.m_isShadowEnabled != k_falseValue;
    }

    // 最後まで読んだ位置がファイルサイズと一致しなければ、壊れたファイルとして扱う
    const bool l_isReadToEnd = l_memoryReadOffset == GetVALMappedDataSize();

    DestroyMemoryMappedFile();

    if (!l_isReadToEnd) { return false; }

    a_morphSetting.SetTargetSettingList(std::move(l_targetSettingList));

    return true;
}

bool FWK::Converter::SkeletalAnimationModelMorphSettingBinaryConverter::Save(const Graphics::SkeletalAnimationModelMorphSetting& a_morphSetting, const std::filesystem::path& a_modelFilePath)
{
    const auto& l_morphSettingFilePath = CreateAssetFilePath(a_modelFilePath, Constant::k_skeletalAnimationModelMorphSettingExtension);
    const auto& l_targetSettingList    = a_morphSetting.GetREFTargetSettingList                     ();

    MorphSettingBinaryHeader l_morphSettingBinaryHeader = {};

    l_morphSettingBinaryHeader.m_fileSize           = CalculateFileSize(l_targetSettingList);
    l_morphSettingBinaryHeader.m_targetSettingCount = l_targetSettingList.size();

    FWK_ASSERT_RETURN_VALUE_IF(!CreateWriteMemoryMappedFile(l_morphSettingFilePath, l_morphSettingBinaryHeader.m_fileSize), ".morphファイルの書き込み用MemoryMappedFileの作成に失敗しました。", false);

    auto l_memoryWriteOffset = k_initialMemoryWriteOffset;

    WriteBinaryData(k_singleBinaryElementCount, &l_morphSettingBinaryHeader, l_memoryWriteOffset);

    for (const auto& l_targetSetting : l_targetSettingList)
    {
        TargetSettingBinaryData l_targetSettingBinaryData = {};

        l_targetSettingBinaryData.m_targetNameSize  = CalculateWStringBinaryFileSize(l_targetSetting.m_targetName);
        l_targetSettingBinaryData.m_lodProtection   = l_targetSetting.m_lodProtection;
        l_targetSettingBinaryData.m_defaultWeight   = l_targetSetting.m_defaultWeight;
        l_targetSettingBinaryData.m_minWeight       = l_targetSetting.m_minWeight;
        l_targetSettingBinaryData.m_maxWeight       = l_targetSetting.m_maxWeight;
        l_targetSettingBinaryData.m_isShadowEnabled = l_targetSetting.m_isShadowEnabled ? k_trueValue : k_falseValue;

        // 名前以外の設定 -> 名前の順に書き込む(読み込みも同じ順で行う)
        WriteBinaryData       (k_singleBinaryElementCount, &l_targetSettingBinaryData, l_memoryWriteOffset);
        WriteWStringBinaryData(l_targetSetting.m_targetName,                           l_memoryWriteOffset);
    }

    const bool l_isWrittenToEnd = l_memoryWriteOffset == l_morphSettingBinaryHeader.m_fileSize;

    DestroyMemoryMappedFile();

    // 一致しない場合は、書き込み順かサイズ計算のどこかが間違っている
    FWK_ASSERT_RETURN_VALUE_IF(!l_isWrittenToEnd, ".morphファイルの書き込みサイズが計算したファイルサイズと一致しません。", false);

    return true;
}

bool FWK::Converter::SkeletalAnimationModelMorphSettingBinaryConverter::Exists(const std::filesystem::path& a_modelFilePath) const
{
    const auto& l_morphSettingFilePath = CreateAssetFilePath(a_modelFilePath, Constant::k_skeletalAnimationModelMorphSettingExtension);

    return Utility::CanLoadFilePath(l_morphSettingFilePath, Constant::k_skeletalAnimationModelMorphSettingExtension);
}

std::uint64_t FWK::Converter::SkeletalAnimationModelMorphSettingBinaryConverter::CalculateFileSize(const std::vector<Struct::ModelMorphTargetSetting>& a_targetSettingList) const
{
    auto l_fileSize = CalculateBinaryDataSize<MorphSettingBinaryHeader>(k_singleBinaryElementCount);

    for (const auto& l_targetSetting : a_targetSettingList)
    {
        l_fileSize += CalculateBinaryDataSize<TargetSettingBinaryData>(k_singleBinaryElementCount);
        l_fileSize += CalculateWStringBinaryFileSize                  (l_targetSetting.m_targetName);
    }

    return l_fileSize;
}