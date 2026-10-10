#include "ModelMaterialSystem.h"

void FWK::Graphics::ModelMaterialSystem::Deserialize(const nlohmann::json& a_rootJson)
{}
bool FWK::Graphics::ModelMaterialSystem::Create()
{
    return false;
}

nlohmann::json FWK::Graphics::ModelMaterialSystem::Serialize() const
{
    return {};
}

bool FWK::Graphics::ModelMaterialSystem::CreateErrorMaterial()
{
    return false;
}

std::weak_ptr<FWK::Graphics::ModelMaterialRecord> FWK::Graphics::ModelMaterialSystem::LoadModelMaterial(const std::filesystem::path& a_filePath)
{
    return {};
}

bool FWK::Graphics::ModelMaterialSystem::AddModelMaterialReferenceCount(const std::weak_ptr<ModelMaterialRecord>& a_materialRecord)
{
    return false;
}
bool FWK::Graphics::ModelMaterialSystem::SubtractModelMaterialReferenceCount(const std::weak_ptr<ModelMaterialRecord>& a_materialRecord)
{
    return false;
}

std::shared_ptr<FWK::Graphics::ModelMaterialBase> FWK::Graphics::ModelMaterialSystem::CreateModelMaterial(const std::filesystem::path& a_filePath)
{
    return nullptr;
}