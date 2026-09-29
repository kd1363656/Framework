#include "GameObjectTransformComponentHierarchicalMatrixUpdateModeJsonConverter.h"

void FWK::Converter::GameObjectTransformComponentMatrixUpdateHierarchicalModeJsonConverter::Deserialize(const nlohmann::json& a_rootJson, GameObjectTransformComponentMatrixUpdateHierarchicalMode& a_matrixUpdateHierarchicalMode) const
{
    if (a_rootJson.is_null()) { return; }

    auto& l_calculateParentWorldMatrixEnumBitShift = a_matrixUpdateHierarchicalMode.GetMutableREFCalculateParentWorldMatrixEnumBitShift();

    if (const auto& l_json = a_rootJson.value(k_calculateParentWorldMatrixEnumBitShiftJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        l_calculateParentWorldMatrixEnumBitShift.Deserialize(a_rootJson);
    }
}

nlohmann::json FWK::Converter::GameObjectTransformComponentMatrixUpdateHierarchicalModeJsonConverter::Serialize(const GameObjectTransformComponentMatrixUpdateHierarchicalMode& a_matrixUpdateHierarchicalMode) const
{
    nlohmann::json l_rootJson = {};

    const auto& l_calculateParentWorldMatrixEnumBitShift = a_matrixUpdateHierarchicalMode.GetREFCalculateParentWorldMatrixEnumBitShift();

    l_rootJson[k_calculateParentWorldMatrixEnumBitShiftJsonKey] = l_calculateParentWorldMatrixEnumBitShift.Serialize();

    return l_rootJson;
}