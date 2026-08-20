#include "StaticModelComponent.h"

void FWK::StaticModelComponent::INIT()
{
	ModelComponentBase::INIT();
}

void FWK::StaticModelComponent::DeserializePrefab(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	// アセットのファイルパスなどを読み込む
	ModelComponentBase::DeserializePrefab(a_rootJson);

	const auto& l_assetFilePathHelper = GetVALAssetFilePathHelper().lock();

	if (!l_assetFilePathHelper) { return; }

	const auto& l_assetFilePath = l_assetFilePathHelper->GetREFAssetFilePath();

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

void FWK::StaticModelComponent::PostLateUpdate()
{
	if (!m_drawRequestData) { return; }

	// 当たり判定などを行って確定したTransformComponentの現在の行列を取得し適用

}

nlohmann::json FWK::StaticModelComponent::SerializePrefab()
{
	nlohmann::json l_rootJson = {};

	Utility::UpdateJson(l_rootJson, ModelComponentBase::SerializePrefab());

	return l_rootJson;
}

void FWK::StaticModelComponent::AddRegisterDrawRequestStrategy(std::unique_ptr<StaticModelRegisterDrawRequestStrategyBase>&& a_registerDrawRequestStrategy)
{
	if (!a_registerDrawRequestStrategy) { return; }

	const auto l_staticTypeID = a_registerDrawRequestStrategy->GetREFRuntimeTypeINFO().k_staticTypeID;

	m_registerDrawRequestStrategyMap.try_emplace(l_staticTypeID, a_registerDrawRequestStrategy);
}