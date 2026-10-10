#include "ModelMaterialSystemJsonConverter.h"

void FWK::Converter::ModelMaterialSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::ModelMaterialSystem& a_modelMaterialSystem) const
{}

nlohmann::json FWK::Converter::ModelMaterialSystemJsonConverter::Serialize(const Graphics::ModelMaterialSystem& a_modelMaterialSystem) const
{
    return {};
}