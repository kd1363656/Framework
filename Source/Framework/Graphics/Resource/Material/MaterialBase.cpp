#include "MaterialBase.h"

bool FWK::Graphics::MaterialBase::SaveBinaryData(const std::filesystem::path& a_filePath) const
{
    Converter::MaterialBinaryConverter l_materialBinaryConverter = {};

    const auto& l_typeINFO = GetREFRuntimeTypeINFO();

    const auto l_payloadSize = CalculateBinaryPayloadSize(l_materialBinaryConverter);

    auto l_memoryWriteOffset = k_initialMemoryOffset;

    // ヘッダーと型名 -> Payloadの順に書き込む
    FWK_ASSERT_RETURN_VALUE_IF(!l_materialBinaryConverter.BeginWrite(a_filePath,
                                                                     l_typeINFO.k_name,
                                                                     l_payloadSize,
                                                                     l_memoryWriteOffset),
                                                                     ".materialファイルの書き込みを開始できず、Materialの保存に失敗しました。",
                                                                     false);

    WriteBinaryPayload(l_materialBinaryConverter, l_memoryWriteOffset);

    return l_materialBinaryConverter.EndWrite(l_memoryWriteOffset);
}
bool FWK::Graphics::MaterialBase::LoadBinaryData(const std::filesystem::path& a_filePath)
{
    Converter::MaterialBinaryConverter l_materialBinaryConverter = {};

    const auto& l_typeINFO = GetREFRuntimeTypeINFO();

    std::string l_typeName = {};

    auto l_memoryReadOffset = k_initialMemoryOffset;

    if (!l_materialBinaryConverter.BeginRead(a_filePath, l_typeName, l_memoryReadOffset)) { return false; }

    // 別の型のMaterialとして保存されたファイルは読み込まない
    if (l_typeName != l_typeINFO.k_name ||
        !ReadBinaryPayload(l_materialBinaryConverter, l_memoryReadOffset))
    {
        l_materialBinaryConverter.EndRead(l_memoryReadOffset);

        return false;
    }

    return l_materialBinaryConverter.EndRead(l_memoryReadOffset);
}

void FWK::Graphics::MaterialBase::EditInspector()
{
    const auto& l_typeINFO = GetREFRuntimeTypeINFO();
    const auto& l_filePath = GetREFFilePath       ();

    ImGui::Text(k_typeNameLabel.data(), l_typeINFO.k_name.data());
    ImGui::Text(k_filePathLabel.data(), Utility::WStringToString(l_filePath).c_str());

    // 値が変わったら、次のフレームでGPUDataBufferを作り直す
    if (EditInspectorParameter())
    {
        m_isGPUDataDirty = true;
    }

    // ファイルを持たないMaterial(デフォルトマテリアル)は保存しない
    if (l_filePath.empty()) { return; }

    if (ImGui::Button(k_saveButtonLabel.data()))
    {
        FWK_ASSERT_RETURN_IF(!SaveBinaryData(l_filePath), "Materialの保存に失敗しました。");
    }
}

bool FWK::Graphics::MaterialBase::ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
    return m_gpuDataBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext);
}

std::string_view FWK::Graphics::MaterialBase::FetchVALFilePrefix() const
{
    const auto& l_typeINFO = GetREFRuntimeTypeINFO();

    auto l_filePrefix = l_typeINFO.k_name;

    if (l_filePrefix.ends_with(k_classNameSuffix))
    {
        l_filePrefix.remove_suffix(k_classNameSuffix.size());
    }

    return l_filePrefix;
}
std::wstring FWK::Graphics::MaterialBase::FetchVALFileName(const std::wstring& a_name) const
{
    const auto l_filePrefix = FetchVALFilePrefix();

    // 型名はASCII文字だけで構成されているため、1文字ずつwchar_tへ変換してよい
    std::wstring l_fileName = { l_filePrefix.begin(), l_filePrefix.end() };

    l_fileName += a_name;
    l_fileName += Constant::k_materialExtension.wstring();

    return l_fileName;
}

std::filesystem::path FWK::Graphics::MaterialBase::FetchVALResolvedFilePath(const std::wstring& a_relativeFilePath) const
{
    if (a_relativeFilePath.empty()) { return {}; }

    const std::filesystem::path l_filePath = a_relativeFilePath;

    if (!l_filePath.is_relative()) { return l_filePath; }

    const std::filesystem::path l_materialFilePath = GetREFFilePath();

    return l_materialFilePath.parent_path() / l_filePath;
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::MaterialBase::FetchVALTextureSRVDescriptorIndex(const Texture& a_texture) const
{
    const auto& l_textureRecord = a_texture.GetREFTextureRecord().lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_textureRecord, "TextureRecordが無効なため、MaterialのTextureSRVDescriptorIndexの取得に失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);

    return l_textureRecord->GetVALSRVDescriptorIndex();
}