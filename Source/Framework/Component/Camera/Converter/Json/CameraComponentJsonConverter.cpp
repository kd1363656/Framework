#include "CameraComponentJsonConverter.h"

void FWK::Converter::CameraComponentIJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, CameraComponent& a_cameraComponent) const
{
	if (a_rootJson.is_null()) { return; }

	auto& l_camera = a_cameraComponent.GetMutableREFCamera();

	l_camera.SetFovYDegree (a_rootJson.value(k_fovYDegreeJsonKey,  Constant::k_cameraDefaultFOVYDegree));
	l_camera.SetFarClip    (a_rootJson.value(k_farClipJsonKey,     Constant::k_cameraDefaultFarClip));
	l_camera.SetNearClip   (a_rootJson.value(k_nearClipJsonKey,    Constant::k_cameraDefaultNearClip));
}

nlohmann::json FWK::Converter::CameraComponentIJsonConverter::SerializePrefab(CameraComponent& a_cameraComponent) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_camera = a_cameraComponent.GetREFCamera();

	l_rootJson[k_fovYDegreeJsonKey] = l_camera.GetVALFovYDegree ();
	l_rootJson[k_farClipJsonKey]    = l_camera.GetVALFarClip    ();
	l_rootJson[k_nearClipJsonKey]   = l_camera.GetVALNearClip   ();

	return l_rootJson;
}