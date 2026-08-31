#include "WorldOutlinerEditorWindow.h"

void FWK::Editor::WorldOutlinerEditorWindow::Draw()
{
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

	const auto& l_sceneManager = SceneManager::GetInstance ();
	const auto& l_sceneWeak    = l_sceneManager.GetVALScene();
	
	if (const auto& l_scene = l_sceneWeak.lock();
		!l_scene)
	{
		m_isSceneSelected = false;

		m_sceneRename.CancelRename();

		ImGui::End();

		return; 
	}

	// ViewportなどのOutliner以外からのEditorManagerの選択状態が変更された
	// EditorManagerの選択状態をOutliner側へ同期する
	m_gameObjectSelection.Synchronize();

	// ViewportなどからGameObjectが選択された場合は、
	// SceneNodeの選択を解除する
	if (m_gameObjectSelection.FetchVALSelectedGameObjectCount() != k_emptySelectionCount)
	{
		m_isSceneSelected = false;
	}

	// SceneNodeが選択されている場合のF2Renameを更新する
	m_sceneRename.Update(l_sceneWeak, m_isSceneSelected);

	// Rename状態の確認とF2によるRename開始を行う
	m_gameObjectRename.Update(m_gameObjectSelection);

	// Deleteキーによる選択中GameObjectの削除要求を確認
	ApplySelectedGameObjectDestroyShortcut();

	// SceneをOutlinerの最上位Nodeとして描画し、
	// その配下にScene内のRootGameObjectと子Hierarchyを描画する
	DrawSceneNode(l_sceneWeak);

	// Outlinerの残りの領域を、
	// RootへのDrop先、選択解除領域
	// 空白右クリック領域として描画する
	DrawRootDropArea();

	ImGui::End();

	// OutlinerのImGui描画が完全に終了してから
	// 選択中GameObjectへ削除フラグを設定する
	// アウトライナーの全体の描画を止めないようにするため
	ApplySelectedGameObjectDestroyRequest();

	// 親子関係の変更もImGui描画終了後に反映する
	m_gameObjectHierarchy.ApplyRequest();
}

bool FWK::Editor::WorldOutlinerEditorWindow::CreateDroppedPrefabInstance(const ContentBrowserEditorWindow& a_contentBrowserEditorWindow)
{
	std::filesystem::path l_assetFilePath = {};

	// ContentBrowserのFileEntryから送られている
	// AssetFilePathPayloadを受け取る
	if (auto& l_dragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();
		!l_dragDropPayloadStorage.DragDropTarget(Constant::k_assetFilePathDragAndDropPayloadLabel, l_assetFilePath)) 
	{
		return false; 
	}

	if (l_assetFilePath.empty()) { return false; }

	const auto& l_sceneManager = SceneManager::GetInstance ();
	const auto& l_scene        = l_sceneManager.GetVALScene().lock();

	if (!l_scene)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "Sceneが無効のためPrefabInstanceを生成できませんでした。");

		return false;
	}

	const auto* l_prefabAssetRegistry = a_contentBrowserEditorWindow.FetchPTRAssetRegistry(Enum::ContentBrowserAssetType::Prefab);

	if (!l_prefabAssetRegistry) { return false; }

	const auto& l_prefabInstanceCreator = a_contentBrowserEditorWindow.GetREFPrefabInstanceCreator();
	const auto& l_createdGameObject     = l_prefabInstanceCreator.CreatePrefabInstance            (*l_prefabAssetRegistry, l_assetFilePath, *l_scene);

	if (l_createdGameObject.expired()) { return false; }

	// Dropして生成したrootGameObjectをそのまま選択する
	m_gameObjectSelection.SelectSingleGameObject(l_createdGameObject);

	// GameObjectが選択されたためSceneNode選択は解除する
	m_isSceneSelected = false;

	return true;
}

void FWK::Editor::WorldOutlinerEditorWindow::DrawSceneNode(const std::weak_ptr<Scene>& a_scene)
{
	const auto& l_scene = a_scene.lock();

	if (!l_scene) { return; }

	const bool l_hasGameObject       = !l_scene->GetREFGameObjectList().empty();
	const bool l_isSceneRenameTarget = m_sceneRename.IsTarget(a_scene);

	std::string l_sceneNodeLabel = {};

	if (l_isSceneRenameTarget)
	{
		// Rename中はScene名をTreeNode側へ表示せず
		// 固定内部IDだけを使用する
		l_sceneNodeLabel = std::string{ k_sceneNodeInternalLabel };
	}
	else
	{
		l_sceneNodeLabel = std::format("{}{}", l_scene->GetREFSceneName(), k_sceneNodeInternalLabel);
	}

	ImGuiTreeNodeFlags l_treeNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow    |
		                                 ImGuiTreeNodeFlags_SpanAvailWidth |
		                                 ImGuiTreeNodeFlags_DefaultOpen;

	if (m_isSceneSelected)
	{
		l_treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	// GameObjectが一つもないSceneは
	// 開く必要のないLeafNodeとして描画する
	if (!l_hasGameObject)
	{
		l_treeNodeFlags |= ImGuiTreeNodeFlags_Leaf |
			               ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	const bool l_isSceneNodeOpen = ImGui::TreeNodeEx(l_sceneNodeLabel.c_str(), l_treeNodeFlags);

	// 展開矢印をクリックしただけの場合は、
	// Scene選択状態を変更しない
	if (!ImGui::IsItemToggledOpen())
	{
		ApplySceneNodeSelection();
	}

	// Renameしていない通常状態だけ、
	// 現在SceneをContentBrowserへ変身できるようにする
	if (!l_isSceneRenameTarget)
	{
		auto& l_dragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();

		l_dragDropPayloadStorage.DragDropSource(Constant::k_sceneDragDropPayloadLabel, a_scene);

		// ContentBrowserからSceneファイルがSceneNodeはDropされた場合は、
		// 現在Sceneの遷移先として登場する
		std::filesystem::path l_assetFilePath = {};

		const auto& l_editorManager              = EditorManager::GetInstance                                  ();
		const auto& l_contentBrowserEditorWindow = l_editorManager.FindWindowEditor<ContentBrowserEditorWindow>().lock();

		if (const auto* l_sceneAssetRegistry = l_contentBrowserEditorWindow->FetchPTRAssetRegistry(Enum::ContentBrowserAssetType::Scene);
			l_contentBrowserEditorWindow &&
			l_sceneAssetRegistry         &&
			l_dragDropPayloadStorage.DragDropTarget(Constant::k_assetFilePathDragAndDropPayloadLabel, l_assetFilePath))
		{
			// SceneFilePath -> SceneUUID
			// のマップからUUIDを取得する
			const auto l_sceneUUID = l_sceneAssetRegistry->FindVALAssetUUID(l_assetFilePath);

			// SceneRegistryに登録されているFileだけをScene遷移先として扱う
			if (!l_sceneUUID.is_nil())
			{
				auto& l_sceneManager = SceneManager::GetInstance();

				l_sceneManager.AddNextSceneLoadFilePath(l_sceneUUID, l_assetFilePath);
			}
		}
	}

	// SceneがRename対象なら
	// TreeNodeと同じ行へInputTextを描画する
	if (l_isSceneRenameTarget)
	{
		ImGui::SameLine();

		m_sceneRename.DrawRenameInput();
	}

	if (!l_hasGameObject ||
		!l_isSceneNodeOpen)
	{
		return;
	}

	// SceneのGameObject所有リストを
	// Outlinerの描画元としてそのまま使用する
	for (const auto& l_gameObject : l_scene->GetREFGameObjectList())
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
		// Sceneの所有Listと各GameObjectの子Listを最後まで完全に走査できる
		DrawGameObjectNode(l_gameObject);
	}
	
	// SceneNodeはTreePushされているため必ず戻す
	ImGui::TreePop();	
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

	// 現在のGameObjectの子を順番に再帰描画する
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
	// ImGui内部IDには変更されないUUIDを使用する
	const auto& l_gameObjectUUIDString     = boost::uuids::to_string    (l_gameObject->GetREFSceneInstanceUUID());
	const bool  l_isGameObjectRenameTarget = m_gameObjectRename.IsTarget(a_gameObject);
	
	const bool  l_isPrefabGameObject  = !l_gameObject->GetREFPrefabUUID().is_nil();
	const auto& l_gameObjectTextColor = l_isPrefabGameObject ? k_prefabGameObjectTextColor : k_nonPrefabGameObjectTextColor;

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

	// TreeNodeの文字色だけをPrefabかどうかに応じて変更する
	ImGui::PushStyleColor(ImGuiCol_Text, l_gameObjectTextColor);

	l_gameObjectNodeDrawResult.m_isNodeOpen = ImGui::TreeNodeEx(l_gameObjectNodeLabel.c_str(), l_treeNodeFlags);

	ImGui::PopStyleColor();

	// 展開矢印をクリックしただけの場合は、
	// GameObjectの選択状態を変更しない
	if (!ImGui::IsItemToggledOpen())
	{
		ApplyGameObjectNodeSelection(a_gameObject);
	}

	// TreeNode自体を対象とする処理は
	// InputTextを描画する前に行う
	DrawGameObjectNodeContextMenu(a_gameObject);
	DrawGameObjectNodeDragDrop   (a_gameObject);

	// Rename対象ならTreeNodeの右側へ
	// 名前編集用InputTextを描画する
	if (l_isGameObjectRenameTarget)
	{
		ImGui::SameLine();

		m_gameObjectRename.DrawRenameInput();
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

	// SceneRename中にGameObjectへ操作を移す場合は、
	// 先にSceneRenameを確定する
	if (m_sceneRename.GetVALIsRenameActive())
	{
		m_sceneRename.ConfirmRename();
	}

	// Rename中に別のGameObjectを右クリックした場合も
	// 選択状態を変更する前に現在のRenameを確定する
	if (m_gameObjectRename.GetVALIsRenameActive())
	{
		m_gameObjectRename.ConfirmRename();
	}

	m_isSceneSelected = false;

	// 未選択のGameObjectを右クリックした場合は
	// そのGameObjectだけを選択対象にする
	// すでに複数選択に含まれているGameObjectなら
	// 現在の複数選択状態を維持する
	if (!m_gameObjectSelection.ContainsSelectedGameObject(a_gameObject))
	{
		m_gameObjectSelection.SelectSingleGameObject(a_gameObject);
	}

	if (ImGui::MenuItem(k_destroyGameObjectMenuItemText.data(),
		                k_destroyGameObjectMenuItemShortcutText.data()))
	{
		// 実際のDestroy()はNode描画終了後に行う
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
	l_imguiDragDropPayloadStorage.DragDropSource(Constant::k_gameObjectDragDropPayloadLabel, a_gameObject);
	
	std::weak_ptr<GameObject> l_droppedGameObject = {};

	// 現在描画しているGameObject上へ
	// 別のGameObjectがDropされたか確認する
	if (!l_imguiDragDropPayloadStorage.DragDropTarget(Constant::k_gameObjectDragDropPayloadLabel, l_droppedGameObject)) { return false; }

	const auto& l_childGameObject = l_droppedGameObject.lock();

	if (!l_childGameObject ||
		l_childGameObject->GetVALIsDestroyed())
	{
		return false;
	}

	// 自分自身を自分の子には出来ない
	if (l_parentGameObject == l_childGameObject) { return false; }

	// すでに同じ親なら変更する必要がない
	if (const auto& l_currentParrentGameObject = l_childGameObject->GetREFParent().lock();
		l_currentParrentGameObject == l_parentGameObject)
	{
		return false;
	}

	// 実際のApplyParent()は行わず
	// ImGui::End()後に処理する要求だけ保存する
	m_gameObjectHierarchy.RequestApplyParent(a_gameObject, l_droppedGameObject);

	return true;
}
void FWK::Editor::WorldOutlinerEditorWindow::DrawRootDropArea()
{
	// GameObjectNodeを描画した後に残っている
	// Outliner領域全体をRoot用DropArea領域として利用する
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

	// Outliner上のGameObjectを空白領域へDropした場合は、
	// 既存の親子関係解除要求として処理する
	const bool  l_isGameObjectUnparentRequested = UnparentDroppedGameObject                                   ();
	const auto& l_editorManager                 = EditorManager::GetInstance                                  ();
	const auto& l_contentBrowserEditorWindow    = l_editorManager.FindWindowEditor<ContentBrowserEditorWindow>().lock();
	      bool  l_isPrefabInstanceCreated       = false;

	if (l_contentBrowserEditorWindow)
	{
		// ContentBrowserからAssetFilePathがDropされた場合、
		// Registry -> PrefabUUID -> Prefab生成処理へつなぐ
		l_isPrefabInstanceCreated = CreateDroppedPrefabInstance(*l_contentBrowserEditorWindow);
	}

	// Drop操作を処理したFrameでは、
	// 空白クリックとしてSelectionを解除しない
	if (!l_isGameObjectUnparentRequested &&
		!l_isPrefabInstanceCreated       &&
		ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		// SceneRename中は空白クリックをRename確定操作として扱う
		if (m_sceneRename.GetVALIsRenameActive())
		{
			m_sceneRename.ConfirmRename();

			return;
		}

		// Rename中の空白左クリックは、
		// Renameを確定するだけにする
		// 現在選択しているGameObjectは選択解除しない
		if (m_gameObjectRename.GetVALIsRenameActive())
		{
			m_gameObjectRename.ConfirmRename();

			return;
		}

		// 通常時の空白左クリックだけ
		// Outlinerの選択をすべて解除する
		m_gameObjectSelection.ClearSelectedGameObjects();

		m_isSceneSelected = false;
	}

	// Outlinerの空白部分を右クリックした場合のメニュー
	if (!ImGui::BeginPopupContextItem(k_rootContextMenuLabel.data())) { return; }

	// 空白右クリックでSceneRename中なら先に確定する
	if (m_sceneRename.GetVALIsRenameActive())
	{
		m_sceneRename.ConfirmRename();
	}

	// 空白右クリックでもRename中なら先に確定する
	if (m_gameObjectRename.GetVALIsRenameActive())
	{
		m_gameObjectRename.ConfirmRename();
	}

	if (const auto& l_selectedGameObjectCount = m_gameObjectSelection.FetchVALSelectedGameObjectCount();
		l_selectedGameObjectCount != k_emptySelectionCount)
	{
		// GameObjectが一つでも選択されている場合は、
		// 「GameObject追加」は表示しない
		// 複数選択中でも、このMenuItemから
		// 選択されているGameObjectをまとめて削除できる
		if (ImGui::MenuItem(k_destroyGameObjectMenuItemText.data(), k_destroyGameObjectMenuItemShortcutText.data()))
		{
			// 実際のDestroy()はOutliner描画終了後に行う
			m_isSelectedGameObjectDestroyRequested = true;
		}
	}
	else
	{
		// 何も選択されていない場合だけRootGameObject追加を表示する
		if (ImGui::MenuItem(k_addRootGameObjectMenuItemText.data()))
		{
			RequestAddGameObject();
		}
	}

	ImGui::EndPopup();
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

		// 一つでも有効な子GameObjectが存在すればtrue
		return true;
	}

	return false;
}

void FWK::Editor::WorldOutlinerEditorWindow::ApplySceneNodeSelection()
{
	if (!ImGui::IsItemClicked(ImGuiMouseButton_Left)) { return; }

	// GameObjectRename中にSceneNodeをクリックした場合は、
	// 先にGameObject名の変更を確定する
	if (m_gameObjectRename.GetVALIsRenameActive())
	{
		m_gameObjectRename.ConfirmRename();
	}

	// SceneとGameObjectを同時選択状態にはしない
	m_gameObjectSelection.ClearSelectedGameObjects();

	m_isSceneSelected = true;
}

void FWK::Editor::WorldOutlinerEditorWindow::ApplyGameObjectNodeSelection(const std::weak_ptr<GameObject>& a_gameObject)
{
	if (!ImGui::IsItemClicked(ImGuiMouseButton_Left)) { return; }

	// SceneRename中にGameObjectをクリックした場合は
	// Scene名変更を先に確定する
	if (m_sceneRename.GetVALIsRenameActive())
	{
		m_sceneRename.ConfirmRename();
	}

	// リネーム中に別のNode操作が行われた場合は
	// 先に現在の名前変更を確定する
	if (m_gameObjectRename.GetVALIsRenameActive())
	{
		m_gameObjectRename.ConfirmRename();
	}

	// GameObjectを選択した時点でSceneNode選択を解除する
	// Shift/Ctrlで早期returnする前に必ず設定する
	m_isSceneSelected = false;

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

void FWK::Editor::WorldOutlinerEditorWindow::ApplySelectedGameObjectDestroyShortcut()
{
	// 既に削除要求が出ている場合は
	// 同じFrame内で重複して要求しない
	if (m_isSelectedGameObjectDestroyRequested) { return; }

	// Rename用InputText内でDeleteを押した場合は、
	// GameObject削除Shortcutとして処理しない
	if (m_gameObjectRename.GetVALIsRenameActive() ||
		m_sceneRename.GetVALIsRenameActive()) 
	{
		return; 
	}

	// OutlinerまたはOutliner配下のChildWindowにFocusがある場合だけ
	// Deleteショートカットを有効にする
	if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) { return; }

	// 今後Outlinerに検索欄などのInputTextを追加した場合も、
	// Text入力中のDeleteキーをGameObject削除として扱わない
	if (ImGui::GetIO().WantTextInput) { return; }

	if (!ImGui::IsKeyPressed(ImGuiKey_Delete, false)) { return; }

	// 選択中GameObjectが存在しないなら何もしない
	if (m_gameObjectSelection.FetchVALSelectedGameObjectCount() == k_emptySelectionCount) { return; }

	// ここではDestroy()しない
	// OutlinerのScene/GameObject走査が全て終了してから
	// ApplySelectedGameObjectDestroyRequest()で実際に削除要求を出す
	m_isSelectedGameObjectDestroyRequested = true;
}
void FWK::Editor::WorldOutlinerEditorWindow::ApplySelectedGameObjectDestroyRequest()
{
	if (!m_isSelectedGameObjectDestroyRequested) { return; }

	// このFrameの要求はここで消費する
	m_isSelectedGameObjectDestroyRequested = false;

	const auto& l_sceneManager = SceneManager::GetInstance ();
	const auto& l_scene        = l_sceneManager.GetVALScene().lock();

	if (!l_scene) 
	{
		// ActiveなSceneがない状態では
		// GameObject選択自体も有効状態として扱わない
		m_gameObjectSelection.ClearSelectedGameObjects();

		return; 
	}

	bool l_isPrefabDestroyed = false;

	for (const auto& l_gameObject : l_scene->GetREFGameObjectList())
	{
		if (!l_gameObject ||
			 l_gameObject->GetVALIsDestroyed())
		{
			continue;
		}

		// Outlinerで現在選択されているGameObjectだけ削除要求を出す
		if (!m_gameObjectSelection.ContainsSelectedGameObject(l_gameObject)) { continue; }

		// PrefabInstanceが一つでも削除対象に含まれていた場合は、
		// Destroy要求後にPrefab全体を更新する
		// 親PrefabがこのGameObjectをChildとして保持している場合
		// 親PrefabのJsonからも削除する必要があるため
		if (!l_gameObject->GetREFPrefabUUID().is_nil())
		{
			l_isPrefabDestroyed = true;
		}

		l_gameObject->Destroy();
	}

	if (l_isPrefabDestroyed)
	{
		auto& l_prefabSystem = l_scene->GetMutableREFPrefabSystem();

		l_prefabSystem.RefreshAllPrefab();
	}

	// Destroy要求を出したGameObjectを
	// EditorManagerやOutlinerの選択肢として残さない
	m_gameObjectSelection.ClearSelectedGameObjects();
}

bool FWK::Editor::WorldOutlinerEditorWindow::UnparentDroppedGameObject()
{
	std::weak_ptr<GameObject> l_droppedGameObject = {};

	// 直前に描画したRootDropAreaへ
	// GameObjectがDropされたか確認する
	if (auto& l_imguiDragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();
		!l_imguiDragDropPayloadStorage.DragDropTarget(Constant::k_gameObjectDragDropPayloadLabel, l_droppedGameObject))
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

	const auto& l_sceneManager = SceneManager::GetInstance ();
	const auto& l_scene        = l_sceneManager.GetVALScene().lock();

	if (!l_scene) { return; }

	const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();

	if (!l_transformComponent) { return; }

	// 一つの行列で機能する行列合成方法を設定
	l_transformComponent->ApplyStandalone();

	// Outlinerから作成できるGameObjectはRootのみなので
	// ApplyParent()などは行わず、そのままSceneへ追加する
	l_scene->AddGameObject(l_gameObject);

	// 作成したGameObjectをOutlinerの単一選択にする
	m_gameObjectSelection.SelectSingleGameObject(l_gameObject);

	m_isSceneSelected = false;
}