#include "GameObjectInputComponent.h"

void FWK::GameObjectInputComponent::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }
}

nlohmann::json FWK::GameObjectInputComponent::Serialize() const
{
    auto l_rootJson = nlohmann::json{};

    return l_rootJson;
}

std::shared_ptr<FWK::GameObjectComponentBase> FWK::GameObjectInputComponent::Clone() const
{
    auto l_clone = std::make_shared<GameObjectInputComponent>();

    // jsonに乗る部分はDeserialize/Serializeで一括生成
    l_clone->Deserialize(Serialize());

    return l_clone;
}