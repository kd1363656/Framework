#include "FetchTransformComponentFromSelfGameObjectHelper.h"

void FWK::Utility::FetchTransformComponentFromSelfGameObjectHelper::PostDeserialize(const GameObject& a_self)
{
	const auto& l_transformComponent = a_self.GetVALTransformComponent();

	// 絶対にあるべきTransformComponentが存在しないためreturn
	FWK_ASSERT_RETURN_IF(l_transformComponent.expired(), "自身のゲームオブジェクトポインタが無効のため、自身のTransformComponent取得に失敗しました。");

	m_fetchedTransformComponent = l_transformComponent;
}