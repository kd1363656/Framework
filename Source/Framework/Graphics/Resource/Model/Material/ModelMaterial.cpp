#include "ModelMaterial.h"

FWK::Graphics::ModelMaterial::ModelMaterial() :
    m_materialRecord(),

    m_storageID(Constant::k_invalidStorageID)
{}
FWK::Graphics::ModelMaterial::ModelMaterial(const ModelMaterial& a_other) :
    m_materialRecord(a_other.m_materialRecord),

    m_storageID(a_other.m_storageID)
{
    AddReferenceCount();
}
FWK::Graphics::ModelMaterial::ModelMaterial(ModelMaterial&& a_other) noexcept :
    m_materialRecord(std::move(a_other.m_materialRecord)),

    m_storageID(a_other.m_storageID)
{
    a_other.m_storageID = Constant::k_invalidStorageID;

    a_other.m_materialRecord.reset();
}
FWK::Graphics::ModelMaterial::~ModelMaterial()
{
    SubtractReferenceCount();
}

FWK::Graphics::ModelMaterial& FWK::Graphics::ModelMaterial::operator=(const ModelMaterial& a_other)
{
    return *this;
}
FWK::Graphics::ModelMaterial& FWK::Graphics::ModelMaterial::operator=(ModelMaterial&& a_other) noexcept
{
    return *this;
}

bool FWK::Graphics::ModelMaterial::Load(const std::filesystem::path& a_filePath)
{
    return false;
}

bool FWK::Graphics::ModelMaterial::IsValid() const
{
    return false;
}

std::shared_ptr<FWK::Graphics::ModelMaterialBase> FWK::Graphics::ModelMaterial::FetchVALMaterial() const
{
    return nullptr;
}

void FWK::Graphics::ModelMaterial::AddReferenceCount() const
{}

void FWK::Graphics::ModelMaterial::SubtractReferenceCount()
{}