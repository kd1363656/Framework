#include "ModelMaterialBinaryConverter.h"

bool FWK::Converter::ModelMaterialBinaryConverter::Load(const std::filesystem::path& a_filePath, Graphics::ModelMaterialBase& a_material)
{
    return false;
}

bool FWK::Converter::ModelMaterialBinaryConverter::Save(const std::filesystem::path& a_filePath, const Graphics::ModelMaterialBase& a_material)
{
    return false;
}

bool FWK::Converter::ModelMaterialBinaryConverter::TryReadTypeName(const std::filesystem::path& a_filePath, std::string& a_typeName)
{
    return false;
}

bool FWK::Converter::ModelMaterialBinaryConverter::TryReadAssetFilePath(AssetFilePath& a_assetFilePath, std::uint64_t& a_memoryReadOffset) const
{
    return false;
}

void FWK::Converter::ModelMaterialBinaryConverter::WriteAssetFilePath(const AssetFilePath& a_assetFilePath, std::uint64_t& a_memoryWriteOffset) const
{}

std::uint64_t FWK::Converter::ModelMaterialBinaryConverter::CalculateAssetFilePathBinaryDataSize() const
{
    return k_emptyAssetFileSize;
}

bool FWK::Converter::ModelMaterialBinaryConverter::TryReadModelMaterialBinaryHeader(ModelMaterialBinaryHeader& a_header, std::uint64_t& a_memoryReadOffset) const
{
    return false;
}