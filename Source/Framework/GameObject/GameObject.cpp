#include "GameObject.h"

void FWK::GameObject::ConfirmMatrix()
{
	FWK_ASSERT_RETURN_IF(!m_transformComponent, "TransformComponentが存在しません、TransformComponentは必ず存在するべきComponentです。");

	m_transformComponent->ConfrimMatrix();
}