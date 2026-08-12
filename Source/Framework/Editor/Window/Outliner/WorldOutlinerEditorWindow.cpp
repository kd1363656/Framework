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
		ApplySelectedGameObjectDestroyRequest();
		m_gameObjectHierarchy.ApplyRequest   ();

		return;
	}

	// ViewportなどのOutliner以外からのEditorManagerの選択状態が変更された
	// EditorManagerの選択状態をOutliner側へ同期する
	m_gameObjectSelection.Synchronize();

	// リネーム中に選択対象が無効になった場合や、
	// 複数選択へ変化した場合はリネームをキャンセルする
	if (m_isGameObjectRenameActive)
	{
		const auto& l_selectedGameObject = l_editorManager.GetREFSelectedGameObject().lock();

		// PrefabInstanceNUMが発行済みのGameObjectは、
        // PrefabName + InstanceNUMから名前が決まるため
        // Outlinerから直接リネームしない
		if (!l_selectedGameObject                                                                        ||
			l_selectedGameObject->GetVALIsDestroyed()                                                    ||
			l_selectedGameObject->GetVALPrefabSceneInstanceNUM() == Constant::k_invalidPrefabInstanceNUM ||
			m_gameObjectSelection.FetchVALSelectedGameObjectCount() != k_singleSelectionCount)
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
	m_gameObjectHierarchy.ApplyRequest();
}

void FWK::Editor::WorldOutlinerEditorWindow::DrawGameObjectNode(const std::weak_ptr<GameObject>& a_gameObject)
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject ||
		l_gameObject->GetVALIsDestroyed())
	{
		return;
	}

	// 現在のGameObject一つ分のNodeを描画する
	// 有効な子GameObjectが存在しない場合は
	// 子を再帰描画する必要がない
	if (const auto l_gameObjectNodeDrawResult = DrawGameObjectNodeHeader(a_gameObject);
		!l_gameObjectNodeDrawResult.m_hasChildGameObject ||
		!l_gameObjectNodeDrawResult.m_isNodeOpen) 
	{
		return; 
	}

	// TreeNodeが閉じられている場合も、
	// 子GameObjectは描画しない
	const auto& l_childSmartPointerVectorArray = l_gameObject->GetREFChildSmartPointerVectorArray         ();
	const auto& l_childGameObjectDataList      = l_childSmartPointerVectorArray.GetREFArrayElementDataList();

	// 現在のGameOBjectの子を順番に再帰描画する
	for (const auto& l_childGameObjectData : l_childGameObjectDataList)
	{
		if (const auto& l_childGameObject = l_childGameObjectData.m_type.lock();
			!l_childGameObject ||
			l_childGameObject->GetVALIsDestroyed())
		{
			continue;
		}

		DrawGameObjectNode(l_childGameObjectData.m_type);
	}

	// 子GameObjectを持つTreeNodeは、
	// TreeNodeEx()によってTreePushされているため必ず戻す
	ImGui::TreePop();
}
FWK::Struct::GameObjectNodeDrawResult FWK::Editor::WorldOutlinerEditorWindow::DrawGameObjectNodeHeader(const std::weak_ptr<GameObject>& a_gameObject)
{
	Struct::GameObjectNodeDrawResult l_gameObjectNodeDrawResult = {};

	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject ||
		l_gameObject->GetVALIsDestroyed())
	{
		return l_gameObjectNodeDrawResult;
	}

	// 有効な子GameObjectが存在するか確認する
	l_gameObjectNodeDrawResult.m_hasChildGameObject = HasValidChildGameObject(a_gameObject);

	// Outlinerへ表示するGameObject名
	const auto& l_gameObjectName = l_gameObject->FetchVALGameObjectName();

	// GameObject名は変更される可能性があるため
	// ImGui内部IDにあｈ変更されないUUIDを使用する
	const auto& l_gameObjectUUIDString     = boost::uuids::to_string (l_gameObject->GetREFSceneInstanceUUID());
	const bool  l_isGameObjectRenameTarget = IsGameObjectRenameTarget(a_gameObject);
	
	std::string l_gameObjectNodeLabel = {};

	if (l_isGameObjectRenameTarget)
	{
		// Rename中はTreeNode側には名前を表示せず
		// UUIDだけをImGui内部IDとして使用する
		l_gameObjectNodeLabel = std::format("###{}", l_gameObjectUUIDString);
	}
	else
	{
		l_gameObjectNodeLabel = std::format("{}###{}", l_gameObjectName, l_gameObjectUUIDString);
	}

	ImGuiTreeNodeFlags l_treeNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow |
		                                 ImGuiTreeNodeFlags_SpanAvailWidth;

	// Outlinerの選択Listに登録されているGameObjectを
	// 選択状態として表示する
	if (m_gameObjectSelection.ContainsSelectedGameObject(a_gameObject))
	{
		l_treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	// 子GameObjectが存在しない場合は
	// 展開できないLeafNodeとして描画する
	if (!l_gameObjectNodeDrawResult.m_hasChildGameObject)
	{
		l_treeNodeFlags |= ImGuiTreeNodeFlags_Leaf |
			               ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	l_gameObjectNodeDrawResult.m_isNodeOpen = ImGui::TreeNodeEx(l_gameObjectNodeLabel.c_str(), l_treeNodeFlags);

	// 展開矢印をクリックしただけの場合は、
	// GameObjectの選択状態を変更しない
	if (!ImGui::IsItemToggledOpen())
	{
		ApplyGameObjectNodeSelection(a_gameObject);
	}

	// TreeNode事態を対象とする処理は
	// InputTextを描画する前に行う
	DrawGameObjectNodeContextMenu(a_gameObject);
	DrawGameObjectNodeDragDrop   (a_gameObject);

	// Rename対象ならTreeNodeの右側へ
	// 名前編集用InputTextを描画する
	if (l_isGameObjectRenameTarget)
	{
		ImGui::SameLine();

		DrawGameObjectRenameInput();
	}

	return l_gameObjectNodeDrawResult;
}
void FWK::Editor::WorldOutlinerEditorWindow::DrawGameObjectNodeContextMenu(const std::weak_ptr<GameObject>& a_gameObject)
{
	if (const auto& l_gameObject = a_gameObject.lock();
		!l_gameObject ||
		l_gameObject->GetVALIsDestroyed())
	{
		return;
	}

	if (!ImGui::BeginPopupContextItem()) { return; }

	// Renameh食うに別のGameObjectを右クリックした場合も
	// 選択状態を変更する前に現在のRenameを確定する
	if (m_isGameObjectRenameActive)
	{
		ConfirmGameObjectRename();
	}

	// 未選択のGameObjectを右クリックした場合は
	// そのGameObjectだけを選択対象にする
	// すでに複数選択に含まれているGameObjectなら
	// 現在の複数選択状態を維持する
	if (!m_gameObjectSelection.ContainsSelectedGameObject(a_gameObject))
	{
		m_gameObjectSelection.SelectSingleGameObject(a_gameObject);
	}

	if (ImGui::MenuItem(k_destroyGameObjectMenuItemName.data()))
	{
		// 実際のDestroy()はNode病が終了後に行う
		m_isSelectedGameObjectDestroyRequested = true;
	}

	ImGui::EndPopup();
}
bool FWK::Editor::WorldOutlinerEditorWindow::DrawGameObjectNodeDragDrop(const std::weak_ptr<GameObject>& a_gameObject)
{
	const auto& l_parentGameObject = a_gameObject.lock();

	if (!l_parentGameObject ||
		l_parentGameObject->GetVALIsDestroyed())
	{
		return false;
	}

	auto& l_imguiDragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();

	// 現在描画しているGameObjectをDrag元として使用する
	l_imguiDragDropPayloadStorage.DragDropSource(k_gameObjectDragDropPayloadLabel, a_gameObject);
	
	std::weak_ptr<GameObject> l_droppedGameObject = {};

	// 現在描画しているGameObject上へ
	// 別のGameObjectがDropされたが確認する
	if (!l_imguiDragDropPayloadStorage.DragDropTarget(k_gameObjectDragDropPayloadLabel, l_droppedGameObject)) { return false; }

	const auto& l_childGameObject = l_droppedGameObject.lock();

	if (!l_childGameObject ||
		l_childGameObject->GetVALIsDestroyed())
	{
		return false;
	}

	// 自分自身を自分の子には出来な
	if (l_parentGameObject == l_childGameObject) { return false; }

	// すでに同じ親なら変更する必要がない
	if (const auto& l_currentParrentGameObject = l_childGameObject->GetREFParent().lock();
		l_currentParrentGameObject == l_parentGameObject)
	{
		return false;
	}

	// 実際のApplyParent()は行わず
	// ImGui::End(9後に処理する要求だけ保存する
	m_gameObjectHierarchy.RequestApplyParent(a_gameObject, l_droppedGameObject);

	return true;
}
void FWK::Editor::WorldOutlinerEditorWindow::DrawGameObjectRenameInput()
{
	if (!m_isGameObjectRenameActive) { return; }

	// F2を押した直後の1フレームだけ
	// InputTextへKeyboardFocusを移動する
	if (m_isGameObjectRenameInputFocusRequested)
	{
		ImGui::SetKeyboardFocusHere();

		m_isGameObjectRenameInputFocusRequested = false;
	}

	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

	const bool l_isRenameConfirmedByEnter = ImGui::InputText(k_gameObjectRenameInputLabel.data(), 
		                                                     &m_gameObjectRenameBuffer,
		                                                     ImGuiInputTextFlags_EnterReturnsTrue |
	                                                         ImGuiInputTextFlags_AutoSelectAll);

	const bool l_isRenameInputHovered = ImGui::IsItemHovered();

	// Escapeなら変更を破棄する
	if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
	{
		CancelGameObjectRename();

		return;
	}

	if (l_isRenameConfirmedByEnter) 
	{
		ConfirmGameObjectRename();

		return;
	}

	// InputText事態をクリックした場合は
	// カーソル移動などの編集捜査なので確定しない
	if (l_isRenameInputHovered) { return; }

	// InputText以外を左クリック、または右クリックした場合は
	// 現在の名前を確定する
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) ||
		ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		ConfirmGameObjectRename();
	}
}
void FWK::Editor::WorldOutlinerEditorWindow::DrawRootDropArea()
{
	// GameObjectNodeを描画した後に残っている
	//Outliner領域全体をRoot用DropArea領域として利用する
	ImVec2 l_rootDropAreaSize = ImGui::GetContentRegionAvail();

	// ImGuiの残り領域が小さすぎても
	// 最低限Dropできる大きさを確保する
	if (l_rootDropAreaSize.x < k_rootDropAreaMINWidth)
	{
		l_rootDropAreaSize.x = k_rootDropAreaMINWidth;
	}

	if (l_rootDropAreaSize.y < k_rootDropAreaMINHeight)
	{
		l_rootDropAreaSize.y = k_rootDropAreaMINHeight;
	}

	// 見た目を描画せず
	// Outlinerの残り領域を操作可能なItemとして登録する
	ImGui::InvisibleButton(k_rootDropAreaLabel.data(), l_rootDropAreaSize);

	// InvisibleButtonが直前のItemなので、
	// ここでDragDropTargetを判定する
	if (!UnparentDroppedGameObject() &&
		ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		// Rename中の空白左クリックは、
		// Renameを確定するだけにする
		// 現在選択しているGameObjectは選択解除しない
		if (m_isGameObjectRenameActive)
		{
			ConfirmGameObjectRename();

			return;
		}

		// 通常時の空白左クリックだけ
		// Outlinerの選択をすべて解除する
		m_gameObjectSelection.ClearSelectedGameObjects();
	}

	// Outlinerの空白部分を右クリックした場合のメニュー
	if (ImGui::BeginPopupContextItem(k_rootContextMenuLabel.data()))
	{
		// 空白右クリックでもRenameを確定する
		if (m_isGameObjectRenameActive)
		{
			ConfirmGameObjectRename();
		}

		if (ImGui::MenuItem(k_addRootGameObjectMenuItemName.data()))
		{
			RequestAddGameObject();
		}

		ImGui::EndPopup();
	}
}

bool FWK::Editor::WorldOutlinerEditorWindow::HasValidChildGameObject(const std::weak_ptr<GameObject>& a_gameObject) const
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject ||
		l_gameObject->GetVALIsDestroyed())
	{
		return false;
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

		// 一つでも有効な子GameObjetが存在すればtrue
		return true;
	}

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
		m_gameObjectSelection.SelectRangeGameObject(a_gameObject);

		return;
	}

	if (l_imGuiIO.KeyCtrl)
	{
		m_gameObjectSelection.ToggleGameObjectSelection(a_gameObject);
		
		return;
	}

	m_gameObjectSelection.SelectSingleGameObject(a_gameObject);
}

void FWK::Editor::WorldOutlinerEditorWindow::ClearGameObjectRenameState()
{
	m_gameObjectRenameBuffer.clear();

	m_isGameObjectRenameActive              = false;
	m_isGameObjectRenameInputFocusRequested = false;
}

void FWK::Editor::WorldOutlinerEditorWindow::ApplySelectedGameObjectDestroyRequest()
{
	if (!m_isSelectedGameObjectDestroyRequested) { return; }

	const auto& l_scene = SceneManager::GetInstance().GetREFScene();

	for (const auto& l_gameObject : l_scene.GetREFGameObjectList())
	{
		if (!l_gameObject ||
			 l_gameObject->GetVALIsDestroyed())
		{
			continue;
		}

		// Outlinerで現在選択されているGameObjectだけ削除要求を出す
		if (!m_gameObjectSelection.ContainsSelectedGameObject(l_gameObject)) { continue; }

		l_gameObject->Destroy();
	}

	m_isSelectedGameObjectDestroyRequested = false;
}

void FWK::Editor::WorldOutlinerEditorWindow::RequestGameObjectRename(const std::weak_ptr<GameObject>& a_gameObject)
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject ||
		l_gameObject->GetVALIsDestroyed())
	{
		return;
	}

	// PrefabSceneInstanceの名前は
	// PrefabName + InstanceNUMから決定されているため
	// Outlinerから直接リネームしない
	if (l_gameObject->GetVALPrefabSceneInstanceNUM() != Constant::k_invalidPrefabInstanceNUM) 
	{
		return; 
	}

	// F2リネームは単一選択時のみ行う
	if (m_gameObjectSelection.FetchVALSelectedGameObjectCount() != k_singleSelectionCount) { return; }

	// 現在Outlinerへ表示されている名前を
	// 編集用Bufferへコピーする
	m_gameObjectRenameBuffer = l_gameObject->FetchVALGameObjectName();

	m_isGameObjectRenameActive              = true;
	m_isGameObjectRenameInputFocusRequested = true;
}

void FWK::Editor::WorldOutlinerEditorWindow::ConfirmGameObjectRename()
{
	if (!m_isGameObjectRenameActive) { return; }

	const auto& l_editorManager      = EditorManager::GetInstance              ();
	const auto& l_selectedGameObject = l_editorManager.GetREFSelectedGameObject().lock();

	if (!l_selectedGameObject                     ||
		l_selectedGameObject->GetVALIsDestroyed() ||
		l_selectedGameObject->GetVALPrefabSceneInstanceNUM() != Constant::k_invalidPrefabInstanceNUM)
	{
		ClearGameObjectRenameState();

		return;
	}

	// 非PrefabGameObjectではPrefasbNameを
	// Outliner上のGameObject名とし使用しているため
	// 編集Bufferの内容をそのまま設定する
	l_selectedGameObject->SetSceneInstanceName(m_gameObjectRenameBuffer);

	ClearGameObjectRenameState();
}

void FWK::Editor::WorldOutlinerEditorWindow::CancelGameObjectRename()
{
	if (!m_isGameObjectRenameActive) { return; }

	ClearGameObjectRenameState();
}

bool FWK::Editor::WorldOutlinerEditorWindow::IsGameObjectRenameTarget(const std::weak_ptr<GameObject>& a_gameObject) const
{
	if (!m_isGameObjectRenameActive) { return false; }

	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject ||
		l_gameObject->GetVALIsDestroyed())
	{
		return false;
	}

    const auto& l_editorManager      = EditorManager::GetInstance              ();
	const auto& l_selectedGameObject = l_editorManager.GetREFSelectedGameObject().lock();

	if (!l_selectedGameObject ||
		l_selectedGameObject->GetVALIsDestroyed())
	{
		return false;
	}

	return l_gameObject == l_selectedGameObject;
}

bool FWK::Editor::WorldOutlinerEditorWindow::UnparentDroppedGameObject()
{
	std::weak_ptr<GameObject> l_droppedGameObject = {};

	// 直前に描画しとRootDropAreaへ
	// GameObjectがDropされたか確認する
	if (auto& l_imguiDragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();
		!l_imguiDragDropPayloadStorage.DragDropTarget(k_gameObjectDragDropPayloadLabel, l_droppedGameObject)) 
	{
		return false; 
	}

	const auto& l_gameObject = l_droppedGameObject.lock();

	// Drop操作自体は成立しているため
	// GameObjectが無効でもtrueを返す
	if (!l_gameObject ||
		l_gameObject->GetVALIsDestroyed())
	{
		return true;
	}

	// 既にRootGameObjectなら
	// 親解除要求を出す必要はない
	if (l_gameObject->GetREFParent().expired()) { return true; }

	// 描画中にはUnparent()せず、
	// ImGui::End()後に処理する要求だけ保存する
	m_gameObjectHierarchy.RequestUnparent(l_droppedGameObject);

	return true;
}

void FWK::Editor::WorldOutlinerEditorWindow::RequestAddGameObject()
{
	auto l_gameObject = std::make_shared<GameObject>();

	// 新規GameObjectが持っているTransformComponentへ
	// Ownerなどの初期状態を適用する
	l_gameObject->INIT           ();
	l_gameObject->PostDeserialize();

	auto& l_sceneManager = SceneManager::GetInstance        ();
	auto& l_scene        = l_sceneManager.GetMutableREFScene();

	// Outlinerから作成できるGameObjectはRootのみなので
	// ApplyParent(9などは行わず、そのままSceneへ変化する
	l_scene.AddGameObject(l_gameObject);

	// 作成したGameObjectをOutlinerの単一選択にする
	m_gameObjectSelection.SelectSingleGameObject(l_gameObject);
}