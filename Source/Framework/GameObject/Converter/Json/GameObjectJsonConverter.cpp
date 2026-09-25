#include "GameObjectJsonConverter.h"

void FWK::Converter::GameObjectJsonConverter::Deserialize(const nlohmann::json& a_rootJson, const std::weak_ptr<GameObject>& a_gameObject)
{
    if (a_rootJson.is_null()) { return; }
}
 
nlohmann::json FWK::Converter::GameObjectJsonConverter::Serialize(const GameObject& a_gameObject) const
{
    nlohmann::json l_rootJson = {};

    return l_rootJson;
}