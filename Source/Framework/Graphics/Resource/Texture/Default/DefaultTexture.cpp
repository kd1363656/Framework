#include "DefaultTexture.h"

void FWK::Graphics::DefaultTexture::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }
}

nlohmann::json FWK::Graphics::DefaultTexture::Serialize() const
{
    return nlohmann::json();
}
