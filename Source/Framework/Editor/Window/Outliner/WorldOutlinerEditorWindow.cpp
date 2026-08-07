#include "WorldOutlinerEditorWindow.h"

void FWK::Editor::WorldOutlinerEditorWindow::Draw()
{
	const auto& l_editorManager = EditorManager::GetInstance();
	const auto& l_scene         = SceneManager::GetInstance ().GetMutableREFScene();

	// Outliner用ImGuiウィンドウを開始
	if (!ImGui::Begin(k_editorName.data()))
	{
		ImGui::End();

		// 前フレームから削除要求が残っていた場合に備えて
		// Outlinerが折りたたまれていても削除要求を処理する
		ApplySelectedGameObjectDestroyRequest    ();
		ApplyGameObjectNodeHierarchyChangeRequest();

		return;
	}

	// ViewportなどのOutliner以外からのEditorManagerの選択状態が変更された場合にOutliner側の選択状態を同期する
	SynchronizeSelectedGameObject();

	// リネーム中に選択対象が無効になった場合や、
	// 複数選択へ変化した場合はリネームをキャンセルする
	if (m_isGameObjectRenameActive)
	{
		const auto& l_selectedGameObject = l_editorManager.GetREFSelectedGameObject().lock();

		if (!l_selectedGameObject ||
			l_selectedGameObject->GetVALIsDestroyed() ||
			Utility::IsPrefabInstance(*l_selectedGameObject) ||
			FetchVALSelectedGameObjectCount() != k_singleSelectionCount)
		{
			CancelGameObjectRename();
		}
	}

	// Outlinerにフォーカスがある場合だけF2を受け付け
	// 名前変更要求を行う
	if (!m_isGameObjectRenameActive                                   &&
		ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
		ImGui::IsKeyPressed(ImGuiKey_F2, false))
	{
		RequestGameObjectRename(l_editorManager.GetREFSelectedGameObject());
	}

	// SceneのGameObject所有リストを
	// Outlinerの描画元としてそのまま使用する
	for (const auto& l_gameObject : l_scene.GetREFGameObjectList())
	{
		if (!l_gameObject ||
			l_gameObject->GetVALIsDestroyed())
		{
			continue;
		}

		// 親を持つGameObjectは、
		// 親GameObjectのNodeから再帰描画される
		if (!l_gameObject->GetREFParent().expired()) { continue; }

		// 親子関係変更はImGui::End()後まで反映しないため
		// Sceneの所有Listと各GameObejctの子Listを最後まで完全に走査できる
		DrawGameObjectNode(l_gameObject);
	}

	// Outlinerの残りの領域を、
	// RootへのDrop先、選択解除領域
	// 空白右クリック領域として描画する
	DrawRootDropArea();

	ImGui::End();

	// OutlinerのImGui描画が完全に終了してから
	// 選択中GameOBjectへ削除フラグを設定する
	// アウトライナーの全体の描画を止めないようにするため
	ApplySelectedGameObjectDestroyRequest();

	// 親子関係の変更もImGui病が終了後に反映する
	ApplyGameObjectNodeHierarchyChangeRequest();
}

void FWK::Editor::WorldOutlinerEditorWindow::SynchronizeSelectedGameObject()
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

void FWK::Editor::WorldOutlinerEditorWindow::DrawGameObjectNode(const std::weak_ptr<GameObject>& a_gameObject)
{
	
}
FWK::Editor::WorldOutlinerEditorWindow::GameObjectNodeDrawResult FWK::Editor::WorldOutlinerEditorWindow::DrawGameObjectNodeHeader(const std::weak_ptr<GameObject>& a_gameObject)
{
	return GameObjectNodeDrawResult();
}
void FWK::Editor::WorldOutlinerEditorWindow::DrawGameObjectNodeContextMenu(const std::weak_ptr<GameObject>& a_gameObject)
{

}
bool FWK::Editor::WorldOutlinerEditorWindow::DrawGameObjectNodeDragDrop(const std::weak_ptr<GameObject>& a_gameObject)
{
	return false;
}
void FWK::Editor::WorldOutlinerEditorWindow::DrawGameObjectRenameInput()
{
}
void FWK::Editor::WorldOutlinerEditorWindow::DrawRootDropArea()
{
}

bool FWK::Editor::WorldOutlinerEditorWindow::HasValidChildGameObject(const std::weak_ptr<GameObject>& a_gameObject) const
{
	return false;
}

void FWK::Editor::WorldOutlinerEditorWindow::ApplyGameObjectNodeSelection(const std::weak_ptr<GameObject>& a_gameObject)
{
	if (!ImGui::IsItemClicked(ImGuiMouseButton_Left)) { return; }

	// リネーム中に別のNode操作が行われた場合は
	// 先に現在の名前変更を確定する
	if (m_isGameObjectRenameActive)
	{
		ConfirmGameObjectRename();
	}

	const auto& l_imGuiIO = ImGui::GetIO();

	// ShiftをCtrlより優先する
	if (l_imGuiIO.KeyShift)
	{
		SelectGameObjectRange(a_gameObject);

		return;
	}

	if (l_imGuiIO.KeyCtrl)
	{
		ToggleGameObjectSelection(a_gameObject);

		return;
	}

	SelectSingleGameObject(a_gameObject);
}

void FWK::Editor::WorldOutlinerEditorWindow::SelectSingleGameObject(const std::weak_ptr<GameObject>& a_gameObject)
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
void FWK::Editor::WorldOutlinerEditorWindow::SelectGameObjectRange(const std::weak_ptr<GameObject>& a_gameObject)
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

	GameObjectRangeSelectionState l_rangeSelectionState = GameObjectRangeSelectionState::BeforeRange;
}

void FWK::Editor::WorldOutlinerEditorWindow::ToggleGameObjectSelection(const std::weak_ptr<GameObject>& a_gameObject)
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

FWK::Editor::WorldOutlinerEditorWindow::GameObjectRangeSelectionState FWK::Editor::WorldOutlinerEditorWindow::AddGameObjectRangeSelectionRecursive(const std::weak_ptr<GameObject>&    a_gameObject, 
	                                                                                                                                               const std::weak_ptr<GameObject>&    a_rangeAnchorObject, 
	                                                                                                                                               const std::weak_ptr<GameObject>&    a_rangeEndGameObject, 
	                                                                                                                                                     GameObjectRangeSelectionState a_rangeSelectionState)
{
	return GameObjectRangeSelectionState();
}
void FWK::Editor::WorldOutlinerEditorWindow::AddSelectedGameObject(const std::weak_ptr<GameObject>& a_gameObject)
{
	const auto& l_gameObejct = a_gameObject.lock();

	if (!l_gameObejct->GetVALIsDestroyed()) { return; }

	// 同じGmaeObjectを二重登録しない
	if (ContainsSelectedGameObject(a_gameObject)) { return; }

	m_selectedGameObjectVectorArray.Add(a_gameObject);
}

void FWK::Editor::WorldOutlinerEditorWindow::RemoveSelectedGameObject(const std::weak_ptr<GameObject>& a_gameObject)
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject)
	{
		// 失効済みweak_ptrはまとめて除去する
		m_selectedGameObjectVectorArray.RemoveExpiredElements();

		return;
	}

	m_selectedGameObjectVectorArray.RemoveSameElement(a_gameObject);
}

void FWK::Editor::WorldOutlinerEditorWindow::ClearSelectedGameObjects()
{
	m_selectedGameObjectVectorArray.Clear();

	auto& l_editorManager = EditorManager::GetInstance();

	l_editorManager.SetSelectedGameObject({});
}
void FWK::Editor::WorldOutlinerEditorWindow::ClearGameObjectRenameState()
{

}
void FWK::Editor::WorldOutlinerEditorWindow::ClearGameObjectNodeHierarchyChangeRequest()
{

}

bool FWK::Editor::WorldOutlinerEditorWindow::ContainsSelectedGameObject(const std::weak_ptr<GameObject>& a_gameObject) const
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

void FWK::Editor::WorldOutlinerEditorWindow::RequestApplyParent(const std::weak_ptr<GameObject>& a_parentGameObject, const std::weak_ptr<GameObject>& a_childGameObject)
{
	
}

void FWK::Editor::WorldOutlinerEditorWindow::RequestUnparent(const std::weak_ptr<GameObject>& a_childGameObject)
{

}

void FWK::Editor::WorldOutlinerEditorWindow::ApplyGameObjectNodeHierarchyChangeRequest()
{

}
void FWK::Editor::WorldOutlinerEditorWindow::ApplySelectedGameObjectDestroyRequest()
{

}

void FWK::Editor::WorldOutlinerEditorWindow::RequestGameObjectRename(const std::weak_ptr<GameObject>& a_gameObject)
{

}

void FWK::Editor::WorldOutlinerEditorWindow::ConfirmGameObjectRename()
{

}

void FWK::Editor::WorldOutlinerEditorWindow::CancelGameObjectRename()
{

}

bool FWK::Editor::WorldOutlinerEditorWindow::IsGameObjectRenameTarget(const std::weak_ptr<GameObject>& a_gameObject) const
{

}

bool FWK::Editor::WorldOutlinerEditorWindow::UnparentDroppedGameObject()
{
	return false;
}

void FWK::Editor::WorldOutlinerEditorWindow::RequestAddGameObject()
{
}

std::size_t FWK::Editor::WorldOutlinerEditorWindow::FetchVALSelectedGameObjectCount() const
{
	std::size_t l_selectedGameObjectCount = k_initialSelectedGameObjectCount;

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