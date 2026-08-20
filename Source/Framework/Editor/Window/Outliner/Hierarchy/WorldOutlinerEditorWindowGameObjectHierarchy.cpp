#include "WorldOutlinerEditorWindowGameObjectHierarchy.h"

void FWK::Editor::WorldOutlinerEditorWindowGameObjectHierarchy::RequestApplyParent(const std::weak_ptr<GameObject>& a_parentGameObject, const std::weak_ptr<GameObject>& a_childGameObject)
{
	// 1フレーム内に複数の親子関係変更要求を処理しない
	if (m_gameObjectNodeHierarchyChangeRequest.m_type != Enum::GameObjectHierarchyChangeRequestType::None) { return; }

	const auto& l_parentGameObject = a_parentGameObject.lock();
	const auto& l_childGameObject  = a_childGameObject.lock ();

	if (!l_parentGameObject                      ||
		!l_childGameObject                       ||
		 l_parentGameObject->GetVALIsDestroyed() ||
		l_childGameObject->GetVALIsDestroyed())
	{
		return;
	}

	// 自分自身を子GameObject二はできな
	if (l_parentGameObject == l_childGameObject) { return; }

	m_gameObjectNodeHierarchyChangeRequest.m_parentGameObject = a_parentGameObject;
	m_gameObjectNodeHierarchyChangeRequest.m_childGameObject  = a_childGameObject;
	m_gameObjectNodeHierarchyChangeRequest.m_type             = Enum::GameObjectHierarchyChangeRequestType::ApplyParent;
}
void FWK::Editor::WorldOutlinerEditorWindowGameObjectHierarchy::RequestUnparent(const std::weak_ptr<GameObject>& a_childGameObject)
{
	// 1フレーム中に複数の親子関係を変更要求を処理しない
	if (m_gameObjectNodeHierarchyChangeRequest.m_type != Enum::GameObjectHierarchyChangeRequestType::None)
	{
		return;
	}

	const auto& l_childGameObject = a_childGameObject.lock();

	if (!l_childGameObject ||
		l_childGameObject->GetVALIsDestroyed())
	{
		return;
	}

	const auto& l_parentGameObject = l_childGameObject->GetREFParent().lock();

	// 既にRootGameObjectなら解除する親が存在しない
	if (!l_parentGameObject ||
		l_parentGameObject->GetVALIsDestroyed())
	{
		return;
	}

	// Unparent要求時点の親も保存する
	// ApplyRequest()までに親が変更された場合に
	// 別の親GameObjectを誤って解除しないため
	m_gameObjectNodeHierarchyChangeRequest.m_parentGameObject = l_parentGameObject;
	m_gameObjectNodeHierarchyChangeRequest.m_childGameObject  = l_childGameObject;
	m_gameObjectNodeHierarchyChangeRequest.m_type             = Enum::GameObjectHierarchyChangeRequestType::Unparent;
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectHierarchy::ApplyRequest()
{
	if (m_gameObjectNodeHierarchyChangeRequest.m_type == Enum::GameObjectHierarchyChangeRequestType::None)
	{
		return;
	}

	const auto& l_sceneManager = SceneManager::GetInstance ();
	const auto& l_scene        = l_sceneManager.GetVALScene().lock();

	if (!l_scene) 
	{
		ClearRequest();

		return; 
	}

	const auto& l_childGameObject = m_gameObjectNodeHierarchyChangeRequest.m_childGameObject.lock();

	if (!l_childGameObject ||
		l_childGameObject->GetVALIsDestroyed())
	{
		ClearRequest();

		return;
	}

	switch(m_gameObjectNodeHierarchyChangeRequest.m_type)
	{
		case Enum::GameObjectHierarchyChangeRequestType::ApplyParent:
		{
			const auto& l_parentGameObject = m_gameObjectNodeHierarchyChangeRequest.m_parentGameObject.lock();

			if (!l_parentGameObject ||
				l_parentGameObject->GetVALIsDestroyed())
			{
				break; 
			}

			// GameObject側で
			// GameObject事態の循環とPrefab循環を確認してから
			// 実際の親子関係を構築する
			if (l_parentGameObject->ApplyParent(m_gameObjectNodeHierarchyChangeRequest.m_childGameObject))
			{
				l_scene->SetIsGameObjectExecutionLevelListDirty(true);

				// Prefabの親子構造が変更されたので
				// Scene上の現在の構造から全Prefabを更新する
				l_scene->GetMutableREFPrefabSystem().RefreshAllPrefab();
			}
		}
		break;

		case Enum::GameObjectHierarchyChangeRequestType::Unparent:
		{
			const auto& l_requestedParentGameObject = m_gameObjectNodeHierarchyChangeRequest.m_parentGameObject.lock();

			if (!l_requestedParentGameObject ||
				 l_requestedParentGameObject->GetVALIsDestroyed())
			{
				break; 
			}

			const auto& l_currentParentGameObject = l_childGameObject->GetREFParent().lock();

			// RequestUnparent()を行った時点から
			// 親GameObjetが変化している場合は解除しない
			if (l_currentParentGameObject != l_requestedParentGameObject)
			{
				break;
			}

			l_requestedParentGameObject->Unparent(m_gameObjectNodeHierarchyChangeRequest.m_childGameObject);

			// 実際にRootへ戻った場合だけ
			// Sceneの実行階層Listを再構築する
			if (l_childGameObject->GetREFParent().expired())
			{
				l_scene->SetIsGameObjectExecutionLevelListDirty(true);

				// Prefabの親子構造からChildが外れたため
				// 全Prefabの保存内容も現在のHierarchyへ同期する
				l_scene->GetMutableREFPrefabSystem().RefreshAllPrefab();
			}
		}
		break;

		case Enum::GameObjectHierarchyChangeRequestType::None:
		default:
		break;
	}

	ClearRequest();
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectHierarchy::ClearRequest()
{
	m_gameObjectNodeHierarchyChangeRequest.m_parentGameObject.reset();
	m_gameObjectNodeHierarchyChangeRequest.m_childGameObject.reset ();

	m_gameObjectNodeHierarchyChangeRequest.m_type = Enum::GameObjectHierarchyChangeRequestType::None;
}