#include "WorldOutlinerEditorWindow.h"

void FWK::Editor::WorldOutlinerEditorWindow::Draw()
{
	// Hierarchy用のImGuiウィンドウを開始する
	if (!ImGui::Begin(k_editorName.data()))
	{
		ImGui::End();
		return;
	}

	auto& l_scene = SceneManager::GetInstance().GetMutableREFScene();

	bool l_isHierarchyChanged = false;

	// SceneのGameObjectListにはRootと子の両方が存在するため
	// 親を持たないGameObjectだけを再帰描画の開始点にする
	for (const auto& l_gameObject : l_scene.GetREFGameObjectList())
	{
		if (!l_gameObject ||
			l_gameObject->GetVALIsDestroyed())
		{
			continue;
		}

		// 親を持っているGameObjectは、
		// 親GameObjectのNodeから描画されるため、continue
		if (!l_gameObject->GetREFParent().expired()) { continue; }

		// 親子変更によって子Listが変更された場合は、
		// そのフレームのHierarchy操作を終了する
		if (DrawGameObjectNode(l_gameObject)) 
		{
			l_isHierarchyChanged = true;

			break;
		}
	}

	// 親子変更が起きていてもSceneの所有List自体は変更されていないため、
	// RootへのDrop領域は描画できる
	DrawRootDropArea(l_scene);

	ImGui::End();

	if (l_isHierarchyChanged)
	{
		// 登録済みGameObjectの実行階層が変化したため、
		// 次のEarlyUpdateで実行階層Listを再構築する
		l_scene.SetIsGameObjectExecutionLevelListDirty(true);
	}
}

bool FWK::Editor::WorldOutlinerEditorWindow::DrawGameObjectNode(const std::shared_ptr<GameObject>& a_gameObject) const
{
	if (!a_gameObject ||
		a_gameObject->GetVALIsDestroyed())
	{
		return false;
	}

	auto& l_editorManager = EditorManager::GetInstance();

	// GameObject表示名
    auto l_gameObjectName = a_gameObject->GetREFContainsNumberPrefabName();

	// 番号を含んだゲームオブジェクトのプレハブ名、プレハブ名すら文字列が空ならゲームオブジェクトを名前とする
	if (l_gameObjectName.empty()) 
	{
		l_gameObjectName = a_gameObject->GetREFPrefabName().empty() ? Constant::k_gameObjectString : a_gameObject->GetREFPrefabName();
	}

	// 同じ名前のGameObjectが複数存在しても、
	// UUIDによってImGui内部では別のItemとして扱う
	const auto& l_gameObjectNodeLabel     = std::format                                     ("{}##{}", l_gameObjectName, Utility::UUIDToString(a_gameObject->GetREFUUID()));
	const auto& l_childGameObjectDataList = a_gameObject->GetREFChildSmartPointerVectorArray().GetREFArrayElementDataList();

	bool l_hasChildGameObject = false;
	
	for (const auto& l_childGameObjectData : l_childGameObjectDataList)
	{
		if (const auto l_childGameObject = l_childGameObjectData.m_type.lock();
			!l_childGameObject ||
			l_childGameObject->GetVALIsDestroyed()) 
		{
			continue; 
		}

		l_hasChildGameObject = true;

		break;
	}

	ImGuiTreeNodeFlags l_treeNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow |
		                                 ImGuiTreeNodeFlags_SpanAvailWidth;

	if (const auto& l_selectedGameObject = l_editorManager.GetREFSelectedGameObject().lock();
		l_selectedGameObject == a_gameObject)
	{
		l_treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	// 有効な子が存在しない場合は展開用の矢印を表示しない
	if (!l_hasChildGameObject)
	{
		l_treeNodeFlags |= ImGuiTreeNodeFlags_Leaf | 
			               ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	const bool l_isNodeOpen = ImGui::TreeNodeEx(l_gameObjectNodeLabel.c_str(), l_treeNodeFlags);

	// 左クリックされたGameObjectを選択する
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		l_editorManager.SetSelectedGameObject(a_gameObject);
	}

	// Drawを開始しようとしているGameObjectも選択対象にする
	if (ImGui::IsItemActive() &&
		ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		l_editorManager.SetSelectedGameObject(a_gameObject);
	}

	// EditorManagerが所有するstd::shared_ptrをPayloadとして使用する
	// EditorManagerのメンバなのでDrag中もアドレスが変化しない
	Utility::DragDropSource(k_gameObjectDragDropPayloadLabel, l_editorManager.GetREFSelectedGameObject());

	if (std::shared_ptr<GameObject> l_droppedGameObject = nullptr;
		Utility::DragDropTarget(k_gameObjectDragDropPayloadLabel, l_droppedGameObject))
	{
		// 無効または削除予定のGameObjectは受け付けない
		// ジフン自信を自分の子にはできない
		if (!l_droppedGameObject                     ||
			l_droppedGameObject->GetVALIsDestroyed() ||
			l_droppedGameObject == a_gameObject)
		{
			return false; 
		}

		// すでに同じ親なら何もしない
		if (const auto& l_currentParentGameObject = l_droppedGameObject->GetREFParent().lock();
			l_currentParentGameObject == a_gameObject) 
		{
			return false; 
		}

		// すでに同じ親なら親子関係を変更しない
		// 描画しているGameObjectを親として、
		// DropされたGameObjectを子にする
		a_gameObject->ApplyParent(l_droppedGameObject);

		// ApplyParent()が成功した場合だけ
		// 実行階層Listの再構築を要求する
		// 追加順序の違いで子が親よりも先に行列の確定などを行わいないようにするため
		if (const auto& l_appliedParentGameObject = l_droppedGameObject->GetREFParent().lock();
			l_appliedParentGameObject != a_gameObject) 
		{
			return false; 
		}
		
		l_editorManager.SetSelectedGameObject(l_droppedGameObject);

		// 子を持つ開かれたNodeは、
		// TreeNodeEX()によってTreePushされている
		if (l_hasChildGameObject && 
			l_isNodeOpen)
		{
			ImGui::TreePop();
		}

		// ApplyParent()によって子Listが変更されたため
		// 現在のHierarchy操作を終了する
		return true;
	}

	// 右クリックメニュー
	bool l_isDestroyRequested = false;

	if (ImGui::BeginPopupContextItem())
	{
		// 右クリックしたGameObjectを選択対象にする
		l_editorManager.SetSelectedGameObject(a_gameObject);

		l_isDestroyRequested = ImGui::MenuItem(k_destroyGameObjectMenuItemName.data());

		ImGui::EndPopup();
	}

	if (l_isDestroyRequested)
	{
		// Sceneからその場で削除するのではなく、
		// 自身とすべての詞損へ削除フラグを立てる
		a_gameObject->Destroy();

		const auto& l_selectedGameObject = l_editorManager.GetREFSelectedGameObject().lock();

		// 選択中GameObjectも削除対象になった場合は、
		// EditorManagerの選択状態を解除する
		if (l_selectedGameObject && 
			l_selectedGameObject->GetVALIsDestroyed())
		{
			// 空のゲームオブジェクトを渡し保持しているゲームオブジェクトをリセット
			l_editorManager.SetSelectedGameObject(std::shared_ptr<GameObject>());
		}

	}

	// 削除申請されたGameObject以下は、
	// このフレームでは描画しない
	if (a_gameObject->GetVALIsDestroyed())
	{
		// 子を持つ開かれたNodeだけ
		// TreeNodeEx(9によってTreePushされている
		if (l_hasChildGameObject && 
			l_isNodeOpen)
		{
			ImGui::TreePop();
		}

		// Destroy()では子List事態は変更されないため、
		// 他のRootGameObjectの描画は続行できる
		return false;
	}

	// 子が存在しない場合は再帰描画しない
	if (!l_hasChildGameObject) { return false; }

	// Nodeが閉じている場合も子を描画しない
	if (!l_isNodeOpen) { return false; }

	// 子GameObjectを再帰描画
	for (const auto& l_childGameObjectData : l_childGameObjectDataList)
	{
		if (const auto l_childGameObject = l_childGameObjectData.m_type.lock();
			!l_childGameObject                     ||
			l_childGameObject->GetVALIsDestroyed() ||
			!DrawGameObjectNode(l_childGameObject))
		{
			continue; 
		}
		
		// 子以下でApplyParent()が行われた場合、
		// 現在捜査している子Listも変更されている可能性があるため、
		// それ以上捜査せずtrueを伝える
		ImGui::TreePop();

		return true;
	}

	ImGui::TreePop();

	return false;
}

void FWK::Editor::WorldOutlinerEditorWindow::DrawRootDropArea(Scene& a_scene) const
{
	ImVec2 l_rootDropAreaSize = ImGui::GetContentRegionAvail();

	if (l_rootDropAreaSize.x < k_rootDropAreaMINWidth) 
	{
		l_rootDropAreaSize.x = k_rootDropAreaMINWidth;
	}

	if (l_rootDropAreaSize.y < k_rootDropAreaMINHeight)
	{
		l_rootDropAreaSize.y = k_rootDropAreaMINHeight;
	}

	// Outlinerの残りの空白部分を一つのImGuiItemにする
	// この領域をRootへのDrop先
	// 空白右クリックメニュー
	// 選択解除領域として使用する
	ImGui::InvisibleButton(k_rootDropAreaLabel.data(), l_rootDropAreaSize);

	const bool l_isRootDropAreaClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
	const bool l_isGameObjectDropped   = TryUnparentDroppedGameObject(a_scene);

	// Hoverではなく、本当に空白領域がクリックされた場合だけ
	// 選択を解除する
	// Drag&Dropではない通常の空白クリック時だけ選択を解除する
	if (l_isRootDropAreaClicked &&
		!l_isGameObjectDropped)
	{
		EditorManager::GetInstance().SetSelectedGameObject(std::shared_ptr<GameObject>());
	}

	// Outliner空白右クリックメニュー
	if (ImGui::BeginPopupContextItem(k_rootContextMenuLabel.data()))
	{
		if (ImGui::MenuItem(k_addRootGameObjectMenuItemName.data()))
		{
			RequestAddGameObject(a_scene);
		}

		ImGui::EndPopup();
	}
}

bool FWK::Editor::WorldOutlinerEditorWindow::TryUnparentDroppedGameObject(Scene& a_scene) const
{
	// RootへのDrop
	std::shared_ptr<GameObject> l_droppedGameObject = {};

	if (!Utility::DragDropTarget(k_gameObjectDragDropPayloadLabel, l_droppedGameObject)) { return false; }

	// Drop操作自体は成立している
	// 以下の失敗時にも空白クリックとしては処理しない
	if (!l_droppedGameObject)                     { return true; }
	if (l_droppedGameObject->GetVALIsDestroyed()) { return true; }

	const auto& l_currentParentGameObject = l_droppedGameObject->GetREFParent().lock();

	if (!l_currentParentGameObject) { return true; }

	// すでにrootなら解除する親がいない
	l_currentParentGameObject->Unparent(l_droppedGameObject);

	// Unparent後にも親が残っている場合は解除失敗
	if (!l_droppedGameObject->GetREFParent().expired()) { return true; }

	EditorManager::GetInstance().SetSelectedGameObject(l_droppedGameObject);


	// 親解除によって実行階層が変化したため、
	// 次のEarlyUpdateで実行階層Listを再構築する
	a_scene.SetIsGameObjectExecutionLevelListDirty(true);

	return true;
}

void FWK::Editor::WorldOutlinerEditorWindow::RequestAddGameObject(Scene & a_scene) const
{
	const auto& l_gameObject = std::make_shared<GameObject>();

	// TransformComponentを含む各Componentへ、
	// OwnerGameObjectなどの必要な情報を設定する
	l_gameObject->PostDeserialize();

	const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_IF(!l_transformComponent, "新規GameObjectにTransformComponentが存在しないため、Outlinerから空のGameObjectを追加できませんでした。");

	//Outlinerから作成するGameObjectは、親を持たないStandalone状態で開始する
	l_transformComponent->ApplyStandalone();

	// 親を持たないため
	// Scene::AddGameObjectによってExecutionLevelZeroへ追加される
	a_scene.AddGameObject(l_gameObject);

	// 作成したGameObjectを選択状態にする
	EditorManager::GetInstance().SetSelectedGameObject(l_gameObject);
}