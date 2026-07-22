#include "PhysicsCharacterVirtualJsonConverter.h"

void FWK::Converter::PhysicsCharacterVirtualBaseJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Physics::PhysicsCharacterVirtualBase& a_physicsCharacterVirtualBase) const
{
	if (a_rootJson.is_null()) { return; }

	a_physicsCharacterVirtualBase.SetCapsuleHalfHeightOfCylinder(a_rootJson.value(k_capsuleHalfHeightOfCylinderJsonKey, Constant::k_defaultCharacterVirtualCapsuleHalfHeightOfCylinder));
	a_physicsCharacterVirtualBase.SetCapsuleRadius              (a_rootJson.value(k_capsuleRadiusJsonKey,               Constant::k_defaultCharacterVirtualCapsuleRadius));
	a_physicsCharacterVirtualBase.SetMaxSlopeAngleRadians       (a_rootJson.value(k_maxSlopeRadiansJsonKey,             Constant::k_defaultCharacterVirtualMaxSlopeAngleRadians));

	a_physicsCharacterVirtualBase.SetIsEnhancedInternalEdgeRemovalDisabled(a_rootJson.value(k_isEnhancedInternalEdgeRemovealDisabledeJsonKey, false));
}

nlohmann::json FWK::Converter::PhysicsCharacterVirtualBaseJsonConverter::Serialize(const Physics::PhysicsCharacterVirtualBase& a_physicsCharacterVirtualBase) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_capsuleHalfHeightOfCylinderJsonKey] = a_physicsCharacterVirtualBase.GetVALCapsuleHalfHeightOfCylinder();
	l_rootJson[k_capsuleRadiusJsonKey]               = a_physicsCharacterVirtualBase.GetVALCapsuleRadius              ();
	l_rootJson[k_maxSlopeRadiansJsonKey]             = a_physicsCharacterVirtualBase.GetVALMaxSlopeAngleRadius        ();

	l_rootJson[k_isEnhancedInternalEdgeRemovealDisabledeJsonKey] = a_physicsCharacterVirtualBase.GetIsEnhancedInternalEdgeRemovalDisabled();

	return l_rootJson;
}