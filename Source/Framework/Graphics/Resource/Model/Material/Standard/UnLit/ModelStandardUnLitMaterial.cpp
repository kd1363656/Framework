#include "ModelStandardUnLitMaterial.h"

const FWK::Struct::ModelRenderTableINFO& FWK::Graphics::ModelStandardUnLitMaterial::FetchREFTableINFO() const
{
    return ModelStandardUnLitMaterialGPUData::GetREFModelRenderTableINFO();
}

void FWK::Graphics::ModelStandardUnLitMaterial::WriteGPUData(GPUElementTable& a_table, const std::uint32_t a_tableElementIndex) const
{}