#include "StaticModelComponent.h"

void FWK::StaticModelComponent::DeserializePrefab(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	// アセットのファイルパスなどを読み込む
	ModelComponentBase::DeserializePrefab(a_rootJson);

	const auto& l_assetFilePath = GetREFAssetFilePathHelper().GetREFAssetFilePath();

	if (l_assetFilePath.empty() ||
		!m_model)
	{
		return;
	}
	
	m_model->Load(l_assetFilePath);
}

void FWK::StaticModelComponent::PostDeserialize()
{
	const auto& l_gameObject = GetREFOwner().lock();

	if (!l_gameObject) { return; }

	const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();

	if (!l_transformComponent) { return; }

	if (!m_model ||
		!m_drawRequestData) 
	{
		return; 
	}

	const auto& l_matrix = l_transformComponent->GetREFMatrix();

	// 描画に必要なデータを渡す
	m_drawRequestData->m_worldMatrix                 = l_matrix;
	m_drawRequestData->m_staticModelRecord           = m_model->GetREFStaticModelRecord();
	m_drawRequestData->m_worldInverseTransposeMatrix = l_matrix.Transpose();
	m_drawRequestData->m_worldMaxScale               = Utility::CalculateWorldMaxScale(l_matrix);
}

nlohmann::json FWK::StaticModelComponent::SerializePrefab()
{
	nlohmann::json l_rootJson = {};

	Utility::UpdateJson(l_rootJson, ModelComponentBase::SerializePrefab());

	return l_rootJson;
}