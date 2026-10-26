#include "SkeletalAnimationModelSpringBoneSettingBinaryConverter.h"

bool FWK::Converter::SkeletalAnimationModelSpringBoneSettingBinaryConverter::Load(Graphics::SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting, const std::filesystem::path& a_modelFilePath)
{
    const auto& l_springBoneSettingFilePath = CreateAssetFilePath(a_modelFilePath, Constant::k_skeletalAnimationModelSpringBoneSettingExtension);

    if (!Utility::CanLoadFilePath(l_springBoneSettingFilePath, Constant::k_skeletalAnimationModelSpringBoneSettingExtension)) { return false; }

    if (!CreateReadMemoryMappedFile(l_springBoneSettingFilePath)) { return false; }

    auto l_memoryReadOffset = k_initialMemoryReadOffset;

    SpringBoneSettingBinaryHeader l_springBoneSettingBinaryHeader = {};

    // .springではない、保存形式が古い、ファイルサイズが合わないファイルは読み込まない
    if (!TryReadSingleBinaryData(l_springBoneSettingBinaryHeader, l_memoryReadOffset) ||
        l_springBoneSettingBinaryHeader.m_assetTypeID != k_springBoneSettingAssetTypeID ||
        l_springBoneSettingBinaryHeader.m_version     != k_springBoneSettingAssetVersion ||
        l_springBoneSettingBinaryHeader.m_fileSize    != GetVALMappedDataSize())
    {
        DestroyMemoryMappedFile();

        return false;
    }

    // 途中で失敗した場合に設定が中途半端にならないよう、全て読めてから設定へ渡す
    std::vector<Struct::ModelSpringBoneChainSetting>    l_chainSettingList    = {};
    std::vector<Struct::ModelSpringBoneColliderSetting> l_colliderSettingList = {};

    l_chainSettingList.resize   (l_springBoneSettingBinaryHeader.m_chainSettingCount);
    l_colliderSettingList.resize(l_springBoneSettingBinaryHeader.m_colliderSettingCount);

    // チェーンのグループ -> 当たり判定の順に読み込む(書き込みも同じ順で行う)
    const bool l_isChainSettingRead = std::ranges::all_of(l_chainSettingList, [this, &l_memoryReadOffset](Struct::ModelSpringBoneChainSetting& a_chainSetting)
    {
        return TryReadChainSetting(a_chainSetting, l_memoryReadOffset);
    });

    const bool l_isColliderSettingRead = l_isChainSettingRead &&
                                         std::ranges::all_of(l_colliderSettingList, [this, &l_memoryReadOffset](Struct::ModelSpringBoneColliderSetting& a_colliderSetting)
                                         {
                                             return TryReadColliderSetting(a_colliderSetting, l_memoryReadOffset);
                                         });

    // 最後まで読んだ位置がファイルサイズと一致しなければ、壊れたファイルとして扱う
    const bool l_isReadToEnd = l_isColliderSettingRead &&
                               l_memoryReadOffset == GetVALMappedDataSize();

    DestroyMemoryMappedFile();

    if (!l_isReadToEnd) { return false; }

    a_springBoneSetting.SetChainSettingList     (std::move(l_chainSettingList));
    a_springBoneSetting.SetColliderSettingList  (std::move(l_colliderSettingList));
    a_springBoneSetting.SetPrewarmIterationCount(l_springBoneSettingBinaryHeader.m_prewarmIterationCount);

    return true;
}

bool FWK::Converter::SkeletalAnimationModelSpringBoneSettingBinaryConverter::Save(const Graphics::SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting, const std::filesystem::path& a_modelFilePath)
{
    const auto& l_springBoneSettingFilePath = CreateAssetFilePath(a_modelFilePath, Constant::k_skeletalAnimationModelSpringBoneSettingExtension);
    const auto& l_chainSettingList          = a_springBoneSetting.GetREFChainSettingList                                   ();
    const auto& l_colliderSettingList       = a_springBoneSetting.GetREFColliderSettingList                                ();

    SpringBoneSettingBinaryHeader l_springBoneSettingBinaryHeader = {};

    l_springBoneSettingBinaryHeader.m_fileSize              = CalculateFileSize(a_springBoneSetting);
    l_springBoneSettingBinaryHeader.m_chainSettingCount     = l_chainSettingList.size();
    l_springBoneSettingBinaryHeader.m_colliderSettingCount  = l_colliderSettingList.size();
    l_springBoneSettingBinaryHeader.m_prewarmIterationCount = a_springBoneSetting.GetVALPrewarmIterationCount();

    FWK_ASSERT_RETURN_VALUE_IF(!CreateWriteMemoryMappedFile(l_springBoneSettingFilePath, l_springBoneSettingBinaryHeader.m_fileSize), ".springファイルの書き込み用MemoryMappedFileの作成に失敗しました。", false);

    auto l_memoryWriteOffset = k_initialMemoryWriteOffset;

    WriteBinaryData(k_singleBinaryElementCount, &l_springBoneSettingBinaryHeader, l_memoryWriteOffset);

    for (const auto& l_chainSetting : l_chainSettingList)
    {
        WriteChainSetting(l_chainSetting, l_memoryWriteOffset);
    }

    for (const auto& l_colliderSetting : l_colliderSettingList)
    {
        WriteColliderSetting(l_colliderSetting, l_memoryWriteOffset);
    }

    const bool l_isWrittenToEnd = l_memoryWriteOffset == l_springBoneSettingBinaryHeader.m_fileSize;

    DestroyMemoryMappedFile();

    // 一致しない場合は、書き込み順かサイズ計算のどこかが間違っている
    FWK_ASSERT_RETURN_VALUE_IF(!l_isWrittenToEnd, ".springファイルの書き込みサイズが計算したファイルサイズと一致しません。", false);

    return true;
}

bool FWK::Converter::SkeletalAnimationModelSpringBoneSettingBinaryConverter::TryReadChainSetting(Struct::ModelSpringBoneChainSetting& a_chainSetting, std::uint64_t& a_memoryReadOffset) const
{
    ChainSettingBinaryData l_chainSettingBinaryData = {};

    if (!TryReadSingleBinaryData(l_chainSettingBinaryData, a_memoryReadOffset)) { return false; }

    a_chainSetting.m_gravityDirection         = l_chainSettingBinaryData.m_gravityDirection;
    a_chainSetting.m_stiffness                = l_chainSettingBinaryData.m_stiffness;
    a_chainSetting.m_drag                     = l_chainSettingBinaryData.m_drag;
    a_chainSetting.m_gravityPower             = l_chainSettingBinaryData.m_gravityPower;
    a_chainSetting.m_hitRadius                = l_chainSettingBinaryData.m_hitRadius;
    a_chainSetting.m_maxAngularVelocityDegree = l_chainSettingBinaryData.m_maxAngularVelocityDegree;
    a_chainSetting.m_endLength                = l_chainSettingBinaryData.m_endLength;
    a_chainSetting.m_adjacentLinkStiffness    = l_chainSettingBinaryData.m_adjacentLinkStiffness;
    a_chainSetting.m_isEnabled                = l_chainSettingBinaryData.m_isEnabled             != k_falseValue;
    a_chainSetting.m_isAdjacentChainLinked    = l_chainSettingBinaryData.m_isAdjacentChainLinked != k_falseValue;
    a_chainSetting.m_isAdjacentChainLooped    = l_chainSettingBinaryData.m_isAdjacentChainLooped != k_falseValue;

    a_chainSetting.m_rootBoneNameList.resize(l_chainSettingBinaryData.m_rootBoneNameCount);

    // 根元のボーンの名前は、1つごとに「文字数 -> 文字」の順に並んでいる
    for (auto& l_rootBoneName : a_chainSetting.m_rootBoneNameList)
    {
        auto l_rootBoneNameSize = k_emptyNameSize;

        if (!TryReadSingleBinaryData (l_rootBoneNameSize, a_memoryReadOffset)                     ||
            !TryReadWStringBinaryData(l_rootBoneNameSize, l_rootBoneName, a_memoryReadOffset))
        {
            return false;
        }
    }

    return true;
}
bool FWK::Converter::SkeletalAnimationModelSpringBoneSettingBinaryConverter::TryReadColliderSetting(Struct::ModelSpringBoneColliderSetting& a_colliderSetting, std::uint64_t& a_memoryReadOffset) const
{
    ColliderSettingBinaryData l_colliderSettingBinaryData = {};

    // 名前以外の設定 -> 名前の順に読み込む
    if (!TryReadSingleBinaryData (l_colliderSettingBinaryData, a_memoryReadOffset)                                                  ||
        !TryReadWStringBinaryData(l_colliderSettingBinaryData.m_boneNameSize, a_colliderSetting.m_boneName, a_memoryReadOffset))
    {
        return false;
    }

    a_colliderSetting.m_offset     = l_colliderSettingBinaryData.m_offset;
    a_colliderSetting.m_tailOffset = l_colliderSettingBinaryData.m_tailOffset;
    a_colliderSetting.m_shape      = l_colliderSettingBinaryData.m_shape;
    a_colliderSetting.m_radius     = l_colliderSettingBinaryData.m_radius;

    return true;
}

void FWK::Converter::SkeletalAnimationModelSpringBoneSettingBinaryConverter::WriteChainSetting(const Struct::ModelSpringBoneChainSetting& a_chainSetting, std::uint64_t& a_memoryWriteOffset) const
{
    ChainSettingBinaryData l_chainSettingBinaryData = {};

    l_chainSettingBinaryData.m_rootBoneNameCount        = a_chainSetting.m_rootBoneNameList.size();
    l_chainSettingBinaryData.m_gravityDirection         = a_chainSetting.m_gravityDirection;
    l_chainSettingBinaryData.m_stiffness                = a_chainSetting.m_stiffness;
    l_chainSettingBinaryData.m_drag                     = a_chainSetting.m_drag;
    l_chainSettingBinaryData.m_gravityPower             = a_chainSetting.m_gravityPower;
    l_chainSettingBinaryData.m_hitRadius                = a_chainSetting.m_hitRadius;
    l_chainSettingBinaryData.m_maxAngularVelocityDegree = a_chainSetting.m_maxAngularVelocityDegree;
    l_chainSettingBinaryData.m_endLength                = a_chainSetting.m_endLength;
    l_chainSettingBinaryData.m_adjacentLinkStiffness    = a_chainSetting.m_adjacentLinkStiffness;
    l_chainSettingBinaryData.m_isEnabled                = a_chainSetting.m_isEnabled             ? k_trueValue : k_falseValue;
    l_chainSettingBinaryData.m_isAdjacentChainLinked    = a_chainSetting.m_isAdjacentChainLinked ? k_trueValue : k_falseValue;
    l_chainSettingBinaryData.m_isAdjacentChainLooped    = a_chainSetting.m_isAdjacentChainLooped ? k_trueValue : k_falseValue;

    WriteBinaryData(k_singleBinaryElementCount, &l_chainSettingBinaryData, a_memoryWriteOffset);

    for (const auto& l_rootBoneName : a_chainSetting.m_rootBoneNameList)
    {
        const auto l_rootBoneNameSize = CalculateWStringBinaryFileSize(l_rootBoneName);

        WriteBinaryData       (k_singleBinaryElementCount, &l_rootBoneNameSize, a_memoryWriteOffset);
        WriteWStringBinaryData(l_rootBoneName,                                  a_memoryWriteOffset);
    }
}
void FWK::Converter::SkeletalAnimationModelSpringBoneSettingBinaryConverter::WriteColliderSetting(const Struct::ModelSpringBoneColliderSetting& a_colliderSetting, std::uint64_t& a_memoryWriteOffset) const
{
    ColliderSettingBinaryData l_colliderSettingBinaryData = {};

    l_colliderSettingBinaryData.m_boneNameSize = CalculateWStringBinaryFileSize(a_colliderSetting.m_boneName);
    l_colliderSettingBinaryData.m_offset       = a_colliderSetting.m_offset;
    l_colliderSettingBinaryData.m_tailOffset   = a_colliderSetting.m_tailOffset;
    l_colliderSettingBinaryData.m_shape        = a_colliderSetting.m_shape;
    l_colliderSettingBinaryData.m_radius       = a_colliderSetting.m_radius;

    // 名前以外の設定 -> 名前の順に書き込む(読み込みも同じ順で行う)
    WriteBinaryData       (k_singleBinaryElementCount, &l_colliderSettingBinaryData, a_memoryWriteOffset);
    WriteWStringBinaryData(a_colliderSetting.m_boneName,                             a_memoryWriteOffset);
}

std::uint64_t FWK::Converter::SkeletalAnimationModelSpringBoneSettingBinaryConverter::CalculateFileSize(const Graphics::SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting) const
{
    const auto& l_chainSettingList    = a_springBoneSetting.GetREFChainSettingList   ();
    const auto& l_colliderSettingList = a_springBoneSetting.GetREFColliderSettingList();

    auto l_fileSize = CalculateBinaryDataSize<SpringBoneSettingBinaryHeader>(k_singleBinaryElementCount);

    for (const auto& l_chainSetting : l_chainSettingList)
    {
        l_fileSize += CalculateBinaryDataSize<ChainSettingBinaryData>(k_singleBinaryElementCount);

        for (const auto& l_rootBoneName : l_chainSetting.m_rootBoneNameList)
        {
            l_fileSize += CalculateBinaryDataSize<std::uint64_t>(k_singleBinaryElementCount);
            l_fileSize += CalculateWStringBinaryFileSize        (l_rootBoneName);
        }
    }

    for (const auto& l_colliderSetting : l_colliderSettingList)
    {
        l_fileSize += CalculateBinaryDataSize<ColliderSettingBinaryData>(k_singleBinaryElementCount);
        l_fileSize += CalculateWStringBinaryFileSize                     (l_colliderSetting.m_boneName);
    }

    return l_fileSize;
}