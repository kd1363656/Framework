#include "CameraComponent.h"
#include "../../../Application/Application.h"

void FWK::CameraComponent::DeserializePrefab(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.DeserializePrefab(a_rootJson, *this);
}

void FWK::CameraComponent::PostDeserialize()
{
	m_fetchTransformComponentFromSelfGameObjectHelper.PostDeserialize(GetREFOwner());

	// 早速取得したTransformComponentから行列を取得する
	const auto& l_transformComponent = m_fetchTransformComponentFromSelfGameObjectHelper.GetREFFetchedTransformComponent().lock();

	FWK_ASSERT_RETURN_IF(!l_transformComponent, "TransformComponentが無効になっています。TransformComponentは存在するべきコンポーネントです");

	// アスペクト比率及びカメラ行列をセット
	const auto& l_application = Application::GetInstance  ();
	const auto& l_window      = l_application.GetREFWindow();
	
	m_camera.SetAspectRatio (l_window.GetVALAspectRatio());
	m_camera.SetCameraMatrix(l_transformComponent->GetREFMatrix());
	
	// プロジェクション行列などを設定する
	m_camera.Setup();
}

void FWK::CameraComponent::EarlyUpdate()
{
	const auto& l_application = Application::GetInstance    ();
	const auto& l_window      = l_application.GetREFWindow  ();
	
	// リサイズ申請がされたときのみにアスペクト比率を更新する
	if (const auto& l_resizeRequest = l_window.GetREFResizeRequest();
		!l_resizeRequest.m_isRequested) 
	{
		return; 
	}

	m_camera.SetAspectRatio(l_window.GetVALAspectRatio());
}
void FWK::CameraComponent::PostLateUpdate()
{
	const auto& l_transformComponent = m_fetchTransformComponentFromSelfGameObjectHelper.GetREFFetchedTransformComponent().lock();

	FWK_ASSERT_RETURN_IF(!l_transformComponent, "TransformComponentが無効になっています。TransformComponentは存在するべきコンポーネントです");

	m_camera.ApplyCameraMatrix(l_transformComponent->GetREFMatrix());
}

void FWK::CameraComponent::EditInspector()
{
	m_inspector.EditInspector(*this);
}

nlohmann::json FWK::CameraComponent::SerializePrefab()
{
	return m_jsonConverter.SerializePrefab(*this);
}