#include "MaterialBase.h"

bool FWK::Graphics::MaterialBase::LoadBinaryData(const std::filesystem::path& a_filePath)
{
    return m_materialBaseBinaryConverter.Load(*this, a_filePath);
}

bool FWK::Graphics::MaterialBase::Deserialize(const nlohmann::json& a_rootJson)
{
    return m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Graphics::MaterialBase::EditInspector()
{
    m_editInspector.EditInspector(*this);
}

bool FWK::Graphics::MaterialBase::ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
    return m_gpuDataBuffer.ReserveRelease(a_retiredFenceValue, a_resourceReleaseContext);
}

bool FWK::Graphics::MaterialBase::SaveBinaryData(const std::filesystem::path& a_filePath)
{
    return m_materialBaseBinaryConverter.Save(*this, a_filePath);
}

nlohmann::json FWK::Graphics::MaterialBase::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
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