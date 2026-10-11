#include "ModelMaterialFileCreator.h"
#include "../../../../../../Application/Application.h"

void FWK::Graphics::ModelMaterialFileCreator::CreateDefaultModelMaterialFile(const std::filesystem::path& a_modelFilePath, const std::wstring& a_subMeshName, const Struct::ModelMaterialAssetData& a_materialAssetData)
{}

FWK::AssetFilePath FWK::Graphics::ModelMaterialFileCreator::CreateTextureAssetFilePath(const std::filesystem::path& a_modelFilePath, const std::wstring& a_textureFilePath) const
{
    return {};
}

std::filesystem::path FWK::Graphics::ModelMaterialFileCreator::CreateDefaultModelMaterialFilePath(const std::filesystem::path& a_modelFilePath, const std::wstring& a_subMeshName)
{
    return {};
}

std::wstring FWK::Graphics::ModelMaterialFileCreator::ReplaceInvalidFileNameCharacter(const std::wstring& a_name)
{
    return {};
}