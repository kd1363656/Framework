#include "MaterialBaseBinaryConverter.h"

bool FWK::Converter::MaterialBaseBinaryConverter::Save(const Graphics::MaterialBase& a_materialBase, const std::filesystem::path& a_filePath)
{
    const auto& l_typeINFO = a_materialBase.GetREFRuntimeTypeINFO();

    // Payloadの中身は派生クラスごとに違うため、サイズの計算は派生クラス(のBinaryConverter)に任せる
    const std::uint64_t l_payloadSize = a_materialBase.CalculateBinaryPayloadSize(*this);

    auto l_memoryWriteOffset = k_initialMemoryWriteOffset;

    // ヘッダーと型名 -> Payloadの順に書き込む
    FWK_ASSERT_RETURN_VALUE_IF(!BeginWrite(a_filePath,
                                           l_typeINFO.k_name,
                                           l_payloadSize,
                                           l_memoryWriteOffset),
                                           ".materialファイルの書き込みを開始できず、Materialの保存に失敗しました。",
                                           false);

    a_materialBase.WriteBinaryPayload(*this, l_memoryWriteOffset);

    return EndWrite(l_memoryWriteOffset);
}

bool FWK::Converter::MaterialBaseBinaryConverter::Load(Graphics::MaterialBase& a_materialBase, const std::filesystem::path& a_filePath)
{
    const auto& l_typeINFO = a_materialBase.GetREFRuntimeTypeINFO();

    std::string l_typeName = {};

    auto l_memoryReadOffset = k_initialMemoryReadOffset;

    if (!BeginRead(a_filePath, l_typeName, l_memoryReadOffset)) { return false; }

    // 別の型のMaterialとして保存されたファイルは読み込まない
    if (l_typeName != l_typeINFO.k_name ||
        !a_materialBase.ReadBinaryPayload(*this, l_memoryReadOffset))
    {
        EndRead(l_memoryReadOffset);

        return false;
    }

    return EndRead(l_memoryReadOffset);
}

bool FWK::Converter::MaterialBaseBinaryConverter::TryReadTypeName(const std::filesystem::path& a_filePath, std::string& a_typeName)
{
    auto l_memoryReadOffset = k_initialMemoryReadOffset;

    if (!BeginRead(a_filePath, a_typeName, l_memoryReadOffset)) { return false; }

    // Payloadは読まないため、ここでファイルを閉じる
    DestroyMemoryMappedFile();

    return true;
}

bool FWK::Converter::MaterialBaseBinaryConverter::TryReadWString(std::wstring& a_destinationString, std::uint64_t& a_memoryReadOffset) const
{
    auto l_wStringBinaryFileSize = k_emptyWStringBinaryFileSize;

    if (!TryReadSingleBinaryData(l_wStringBinaryFileSize, a_memoryReadOffset)) { return false; }

    return TryReadWStringBinaryData(l_wStringBinaryFileSize, a_destinationString, a_memoryReadOffset);
}

void FWK::Converter::MaterialBaseBinaryConverter::WriteWString(const std::wstring& a_wString, std::uint64_t& a_memoryWriteOffset) const
{
    const auto l_wStringBinaryFileSize = CalculateWStringBinaryFileSize(a_wString);

    WriteBinaryData       (k_singleBinaryElementCount, &l_wStringBinaryFileSize, a_memoryWriteOffset);
    WriteWStringBinaryData(a_wString,                                            a_memoryWriteOffset);
}

std::uint64_t FWK::Converter::MaterialBaseBinaryConverter::CalculateWStringSize(const std::wstring& a_wString) const
{
    return CalculateBinaryDataSize<std::uint64_t>(k_singleBinaryElementCount) + CalculateWStringBinaryFileSize(a_wString);
}

bool FWK::Converter::MaterialBaseBinaryConverter::BeginRead(const std::filesystem::path& a_filePath, std::string& a_typeName, std::uint64_t& a_memoryReadOffset)
{
    a_typeName.clear();

    a_memoryReadOffset = k_initialMemoryReadOffset;

    if (!Utility::CanLoadFilePath(a_filePath, Constant::k_materialExtension)) { return false; }

    if (!CreateReadMemoryMappedFile(a_filePath)) { return false; }

    MaterialBinaryHeader l_materialBinaryHeader = {};

    // .materialではない、保存形式が古い、ファイルサイズが合わないファイルは読み込まない
    if (!TryReadSingleBinaryData(l_materialBinaryHeader, a_memoryReadOffset)           ||
        l_materialBinaryHeader.m_assetTypeID != k_materialAssetTypeID                  ||
        l_materialBinaryHeader.m_version     != k_materialAssetVersion                 ||
        l_materialBinaryHeader.m_fileSize    != GetVALMappedDataSize()                 ||
        !TryReadStringBinaryData(l_materialBinaryHeader.m_typeNameSize, a_typeName, a_memoryReadOffset))
    {
        a_typeName.clear();

        DestroyMemoryMappedFile();

        return false;
    }

    return true;
}

bool FWK::Converter::MaterialBaseBinaryConverter::BeginWrite(const std::filesystem::path& a_filePath,
                                                             const std::string_view&      a_typeName,
                                                             const std::uint64_t&         a_payloadSize,
                                                                   std::uint64_t&         a_memoryWriteOffset)
{
    a_memoryWriteOffset = k_initialMemoryWriteOffset;

    FWK_ASSERT_RETURN_VALUE_IF(a_typeName.empty(), "Materialの型名が空のため、.materialファイルの書き込みを開始できません。", false);

    const auto& l_typeName = std::string{ a_typeName };

    MaterialBinaryHeader l_materialBinaryHeader = {};

    l_materialBinaryHeader.m_typeNameSize = CalculateStringBinaryFileSize(l_typeName);
    l_materialBinaryHeader.m_fileSize     = CalculateBinaryDataSize<MaterialBinaryHeader>(k_singleBinaryElementCount) + l_materialBinaryHeader.m_typeNameSize + a_payloadSize;

    FWK_ASSERT_RETURN_VALUE_IF(!CreateWriteMemoryMappedFile(a_filePath, l_materialBinaryHeader.m_fileSize), ".materialファイルの書き込み用MemoryMappedFileの作成に失敗しました。", false);

    // ヘッダー -> 型名の順に書き込む(この後に派生クラスのBinaryConverterがPayloadを書き込む)
    WriteBinaryData      (k_singleBinaryElementCount, &l_materialBinaryHeader, a_memoryWriteOffset);
    WriteStringBinaryData(l_typeName,                                          a_memoryWriteOffset);

    return true;
}

bool FWK::Converter::MaterialBaseBinaryConverter::EndRead(const std::uint64_t& a_memoryReadOffset)
{
    const bool l_isReadToEnd = a_memoryReadOffset == GetVALMappedDataSize();

    DestroyMemoryMappedFile();

    return l_isReadToEnd;
}
bool FWK::Converter::MaterialBaseBinaryConverter::EndWrite(const std::uint64_t& a_memoryWriteOffset)
{
    const bool l_isWrittenToEnd = a_memoryWriteOffset == GetVALMappedDataSize();

    DestroyMemoryMappedFile();

    // 一致しない場合は、Materialの保存順かサイズ計算のどこかが間違っている
    FWK_ASSERT_RETURN_VALUE_IF(!l_isWrittenToEnd, ".materialファイルの書き込みサイズが計算したファイルサイズと一致しません。", false);

    return true;
}