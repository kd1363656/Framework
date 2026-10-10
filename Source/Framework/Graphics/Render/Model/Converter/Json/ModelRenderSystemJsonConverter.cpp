#include "ModelRenderSystemJsonConverter.h"

void FWK::Converter::ModelRenderSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::ModelRenderSystem& a_modelRenderSystem) const
{}

nlohmann::json FWK::Converter::ModelRenderSystemJsonConverter::Serialize(const Graphics::ModelRenderSystem& a_modelRenderSystem) const
{
    return {};
}