#include "WorldOutlinerEditorWindowGameObjectSelection.h"

void FWK::Editor::WorldOutlinerEditorWindowGameObjectSelection::Synchronize()
{
	// Sceneから実際に削除され
	// weak_ptrが失効した選択要素を取り除く
	m_selectedGameObjectVectorArray.RemoveExpiredElements();

	const auto& l_editorManager                = EditorManager::GetInstance              ();
	const auto& l_editorSelectedGameObjectWeak = l_editorManager.GetREFSelectedGameObject();
	const auto& l_editorSelectedGameObject     = l_editorSelectedGameObjectWeak.lock     ();

	// Editormanager側の選択が解除されている場合は、
	// Outliner側の複数選択も解除する
	if (!l_editorSelectedGameObject)
	{
		m_selectedGameObjectVectorArray.Clear();

		return;
	}

	// EditorManagerの代表選択が削除申請済みの場合は選択リストから外し、
	// 残っている選択から新しい代表選択を設定する
	if (l_editorSelectedGameObject->GetVALIsDestroyed())
	{
		RemoveSelectedGameObject(l_editorSelectedGameObjectWeak);
		
		return;
	}

	// EditorManagerの代表選択がOutlinerの選択リストに含まれている場合は同期済み
	// Shift範囲選択やCtrl複数選択も、この条件によってそのまま維持される
	if (ContainsSelectedGameObject(l_editorSelectedGameObjectWeak)) { return; }

	// ViewportなどOutliner以外からGameObjectが選択された場合は、
	// 外部で選択されたGameObjectだけの単一選択へ切り替える
	m_selectedGameObjectVectorArray.Clear();

	AddSelectedGameObject(l_editorSelectedGameObjectWeak);
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectSelection::SelectSingleGameObject(const std::weak_ptr<GameObject>& a_gameObject)
{
	const auto& l_gameObject    = a_gameObject.lock         ();
	      auto& l_editorManager = EditorManager::GetInstance();

	if (!l_gameObject ||
		l_gameObject->GetVALIsDestroyed())
	{
		return;
	}

	m_selectedGameObjectVectorArray.Clear();

	AddSelectedGameObject(a_gameObject);

	l_editorManager.SetSelectedGameObject(a_gameObject);
}
void FWK::Editor::WorldOutlinerEditorWindowGameObjectSelection::SelectRangeGameObject(const std::weak_ptr<GameObject>& a_gameObject)
{
	const auto& l_rangeEndGameObject = a_gameObject.lock();

	if (!l_rangeEndGameObject ||
		l_rangeEndGameObject->GetVALIsDestroyed())
	{
		return;
	}

	const auto& l_rangeAnchorGameObjectWeak = EditorManager::GetInstance      ().GetREFSelectedGameObject();
	const auto& l_rangeAnchorGameObject     = l_rangeAnchorGameObjectWeak.lock();

	// Shiftセンタ奥の開始地点が存在しない場合は、
	// 通常の単一選択として扱う
	if (!l_rangeAnchorGameObject ||
		l_rangeAnchorGameObject->GetVALIsDestroyed())
	{
		SelectSingleGameObject(a_gameObject);

		return;
	}

	// 開始地点と終了地点が同じ場合は、
	// そのGameObjectだけを選択状態にする
	if (l_rangeAnchorGameObject == l_rangeEndGameObject)
	{
		m_selectedGameObjectVectorArray.Clear();

		AddSelectedGameObject(a_gameObject);

		return;
	}

	m_selectedGameObjectVectorArray.Clear();

	Enum::GameObjectRangeSelectionState l_rangeSelectionState = Enum::GameObjectRangeSelectionState::BeforeRange;

	const auto& l_sceneManager = SceneManager::GetInstance ();
	const auto& l_scene        = l_sceneManager.GetVALScene().lock();

	if (!l_scene) { return; }

	// Outlinerと同じ順番で、
	// RootGameObjectから再帰走査する
	for (const auto& l_rootGameObject : l_scene->GetREFGameObjectList())
	{
		if (!l_rootGameObject ||
			l_rootGameObject->GetVALIsDestroyed())
		{
			continue;
		}

		if (!l_rootGameObject->GetREFParent().expired()) { continue; }

		l_rangeSelectionState = AddGameObjectRangeSelectionRecursive(l_rootGameObject,  
			                                                         l_rangeAnchorGameObjectWeak,
			                                                         a_gameObject,
			                                                         l_rangeSelectionState);

		if (l_rangeSelectionState == Enum::GameObjectRangeSelectionState::Completed) { break; }
	}

	// 何らかの理由で範囲を完成できなかった場合は、
	// 最低開始地点と終了地点だけを選択する
	if (l_rangeSelectionState != Enum::GameObjectRangeSelectionState::Completed)
	{
		m_selectedGameObjectVectorArray.Clear();

		AddSelectedGameObject(l_rangeAnchorGameObjectWeak);
		AddSelectedGameObject(a_gameObject);
	}
}
void FWK::Editor::WorldOutlinerEditorWindowGameObjectSelection::ToggleGameObjectSelection(const std::weak_ptr<GameObject>& a_gameObject)
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject ||
		l_gameObject->GetVALIsDestroyed())
	{
		return;
	}

	auto& l_editorManager = EditorManager::GetInstance();

	// 既に選択済みの場合は解除する
	if (ContainsSelectedGameObject(a_gameObject))
	{
		const auto& l_editorSelectedGameObject = l_editorManager.GetREFSelectedGameObject().lock();

		RemoveSelectedGameObject(a_gameObject);

		// EditorManagerの代表選択ではないGameObjectを削除しただけなら、
		// Editormanager側は変更しない
		if (l_editorSelectedGameObject != l_gameObject) { return; }

		// EditorManagerの代表選択を解除した場合は、
		// 残っている選択GameObjectから新しい代表を設定する
		const auto& l_selectedGameObjectDataList = m_selectedGameObjectVectorArray.GetREFArrayElementDataList();

		for (const auto& l_selectedGameObjectData : l_selectedGameObjectDataList)
		{
			if (const auto& l_selectedGameObject = l_selectedGameObjectData.m_type.lock();
				!l_selectedGameObject ||
				l_selectedGameObject->GetVALIsDestroyed())
			{
				continue;
			}

			l_editorManager.SetSelectedGameObject(l_selectedGameObjectData.m_type);

			return;
		}

		// 一つも選択が残っていない場合は
		// ユーザー捜査によって選択がなくなったため解除する
		l_editorManager.SetSelectedGameObject({});

		return;
	}

	// 未選択GameObjectなら現在の複数選択へ追加する
	AddSelectedGameObject(a_gameObject);

	// Ctrlで新しく選択されたGameObjectを代表選択にする
	l_editorManager.SetSelectedGameObject(a_gameObject);
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectSelection::ClearSelectedGameObjects()
{
	m_selectedGameObjectVectorArray.Clear();

	auto& l_editorManager = EditorManager::GetInstance();

	l_editorManager.SetSelectedGameObject({});
}

bool FWK::Editor::WorldOutlinerEditorWindowGameObjectSelection::ContainsSelectedGameObject(const std::weak_ptr<GameObject>& a_gameObject) const
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject ||
		l_gameObject->GetVALIsDestroyed())
	{
		return false;
	}

	const auto& l_selectedGameObjectDataList = m_selectedGameObjectVectorArray.GetREFArrayElementDataList();

	for (const auto& l_selectedGameObjectData : l_selectedGameObjectDataList)
	{
		const auto& l_selectedGameObject = l_selectedGameObjectData.m_type.lock();

		if (!l_selectedGameObject ||
			l_selectedGameObject->GetVALIsDestroyed())
		{
			continue;
		}

		if (l_selectedGameObject == l_gameObject) { return true; }
	}

	return false;
}

std::size_t FWK::Editor::WorldOutlinerEditorWindowGameObjectSelection::FetchVALSelectedGameObjectCount() const
{
	auto l_selectedGameObjectCount = k_initialSelectedGameObjectCount;

	const auto& l_selectedGameObjectDataList = m_selectedGameObjectVectorArray.GetREFArrayElementDataList();

	for (const auto& l_selectedGameObjectData : l_selectedGameObjectDataList)
	{
		const auto& l_selectedGameObject = l_selectedGameObjectData.m_type.lock();

		if (!l_selectedGameObject ||
			l_selectedGameObject->GetVALIsDestroyed())
		{
			continue;
		}

		++l_selectedGameObjectCount;
	}

	return l_selectedGameObjectCount;
}

FWK::Enum::GameObjectRangeSelectionState FWK::Editor::WorldOutlinerEditorWindowGameObjectSelection::AddGameObjectRangeSelectionRecursive(const std::weak_ptr<GameObject>&          a_gameObject,
	                                                                                                                                     const std::weak_ptr<GameObject>&          a_rangeAnchorObject,
	                                                                                                                                     const std::weak_ptr<GameObject>&          a_rangeEndGameObject,
	                                                                                                                                           Enum::GameObjectRangeSelectionState a_rangeSelectionState)
{
	if (a_rangeSelectionState == Enum::GameObjectRangeSelectionState::Completed) { return a_rangeSelectionState; }

	const auto& l_gameObject            = a_gameObject.lock        ();
	const auto& l_rangeAnchorGameObject = a_rangeAnchorObject.lock ();
	const auto& l_rangeEndGameObject    = a_rangeEndGameObject.lock();

	// 範囲選択の両端が無効なら
	// 範囲選択そのものを成立させない
	if (!l_rangeAnchorGameObject ||
		!l_rangeEndGameObject)
	{
		return Enum::GameObjectRangeSelectionState::None;
	}

	// 現在確認しているGameObjectだけが無効の場合は
	// 現在の範囲選択状態を維持して走査を続ける
	if (!l_gameObject ||
		l_gameObject->GetVALIsDestroyed())
	{
		return a_rangeSelectionState;
	}

	if (l_gameObject == l_rangeAnchorGameObject ||
		l_gameObject == l_rangeEndGameObject)
	{
		// 一つ目の端点を発見
		if (a_rangeSelectionState == Enum::GameObjectRangeSelectionState::BeforeRange)
		{
			a_rangeSelectionState = Enum::GameObjectRangeSelectionState::Selecting;
		}
		// 二つ目の端点を発見
		else if(a_rangeSelectionState == Enum::GameObjectRangeSelectionState::Selecting)
		{
			a_rangeSelectionState = Enum::GameObjectRangeSelectionState::Completed;
		}
	}

	// 一つの端点から二つの端点までを選択する
	if (a_rangeSelectionState == Enum::GameObjectRangeSelectionState::Selecting ||
		a_rangeSelectionState == Enum::GameObjectRangeSelectionState::Completed)
	{
		AddSelectedGameObject(a_gameObject);
	}

	// 二つ目の端点まで到達したため
	// これ以上GameObjectを確認する必要はない
	if (a_rangeSelectionState == Enum::GameObjectRangeSelectionState::Completed)
	{
		return a_rangeSelectionState;
	}

	const auto& l_childSmartPointerVectorArray = l_gameObject->GetREFChildSmartPointerVectorArray         ();
	const auto& l_childGameObjectDataList      = l_childSmartPointerVectorArray.GetREFArrayElementDataList();

	for (const auto& l_childGameObjectData : l_childGameObjectDataList)
	{
		if (const auto& l_childGameObject = l_childGameObjectData.m_type.lock();
			!l_childGameObject ||
			l_childGameObject->GetVALIsDestroyed())
		{
			continue;
		}

		a_rangeSelectionState = AddGameObjectRangeSelectionRecursive(l_childGameObjectData.m_type,
			                                                         a_rangeAnchorObject,
			                                                         a_rangeEndGameObject,
			                                                         a_rangeSelectionState);

		if (a_rangeSelectionState == Enum::GameObjectRangeSelectionState::Completed ||
			a_rangeSelectionState == Enum::GameObjectRangeSelectionState::None)
		{
			return a_rangeSelectionState;
		}
	}

	return a_rangeSelectionState;
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectSelection::AddSelectedGameObject(const std::weak_ptr<GameObject>& a_gameObject)
{
	if (const auto& l_gameObejct = a_gameObject.lock();
		!l_gameObejct ||
		l_gameObejct->GetVALIsDestroyed()) 
	{
		return; 
	}

	// 同じGmaeObjectを二重登録しない
	if (ContainsSelectedGameObject(a_gameObject)) { return; }

	m_selectedGameObjectVectorArray.Add(a_gameObject);
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectSelection::RemoveSelectedGameObject(const std::weak_ptr<GameObject>&a_gameObject)
{
	if (const auto& l_gameObject = a_gameObject.lock();
		!l_gameObject)
	{
		// 失効済みweak_ptrはまとめて除去する
		m_selectedGameObjectVectorArray.RemoveExpiredElements();

		return;
	}

	m_selectedGameObjectVectorArray.RemoveSameElement(a_gameObject);
}