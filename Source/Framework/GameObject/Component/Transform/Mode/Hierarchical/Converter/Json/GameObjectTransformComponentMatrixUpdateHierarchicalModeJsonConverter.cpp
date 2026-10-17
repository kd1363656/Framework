#include "GameObjectTransformComponentMatrixUpdateHierarchicalModeJsonConverter.h"

void FWK::Converter::GameObjectTransformComponentMatrixUpdateHierarchicalModeJsonConverter::Deserialize(const nlohmann::json& a_rootJson, GameObjectTransformComponentMatrixUpdateHierarchicalMode&  a_gameObjectTransformComponentMatrixUpdateHierarchicalMode) const
{
    if (a_rootJson.is_null()) { return; }

     a_gameObjectTransformComponentMatrixUpdateHierarchicalMode.GameObjectTransformComponentMatrixUpdateModeBase::Deserialize(a_rootJson);

    auto& l_calculateParentWorldMatrixEnumBitShift =  a_gameObjectTransformComponentMatrixUpdateHierarchicalMode.GetMutableREFCalculateParentWorldMatrixEnumBitShift();

    if (const auto& l_json = a_rootJson.value(k_calculateParentWorldMatrixEnumBitShiftJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        l_calculateParentWorldMatrixEnumBitShift.Deserialize(l_json);
    }
}

nlohmann::json FWK::Converter::GameObjectTransformComponentMatrixUpdateHierarchicalModeJsonConverter::Serialize(const GameObjectTransformComponentMatrixUpdateHierarchicalMode&  a_gameObjectTransformComponentMatrixUpdateHierarchicalMode) const
{
    nlohmann::json l_rootJson =  a_gameObjectTransformComponentMatrixUpdateHierarchicalMode.GameObjectTransformComponentMatrixUpdateModeBase::Serialize();

    const auto& l_calculateParentWorldMatrixEnumBitShift =  a_gameObjectTransformComponentMatrixUpdateHierarchicalMode.GetREFCalculateParentWorldMatrixEnumBitShift();

    l_rootJson[k_calculateParentWorldMatrixEnumBitShiftJsonKey] = l_calculateParentWorldMatrixEnumBitShift.Serialize();

    return l_rootJson;
}