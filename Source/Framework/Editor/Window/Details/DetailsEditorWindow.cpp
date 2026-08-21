#include "DetailsEditorWindow.h"

void FWK::Editor::DetailsEditorWindow::Draw()
{
	if (!ImGui::Begin(k_editorName.data()))
	{
		ImGui::End();

		return;
	}

	      auto& l_editorManager      = EditorManager::GetInstance              ();
	const auto& l_selectedGameObject = l_editorManager.GetREFSelectedGameObject().lock();

	if (!l_selectedGameObject) 
	{
		ImGui::TextDisabled(k_noSelectedGameObjectMessage.data());
		ImGui::End         ();

		return;
	}

	if (l_selectedGameObject->GetVALIsDestroyed())
	{
		l_editorManager.SetSelectedGameObject({});

		ImGui::TextDisabled(k_destroyedGameObjectMessage.data());
		ImGui::End         ();

		return;
	}

	DrawGameObjectDetails(l_selectedGameObject);

	ImGui::End();
}

void FWK::Editor::DetailsEditorWindow::DrawGameObjectDetails(const std::weak_ptr<GameObject>& a_gameObject) const
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject) { return; }

	const auto& l_gameObjectDisplayName = l_gameObject->FetchVALGameObjectName();
	const auto& l_uuidString            = boost::uuids::to_string             (l_gameObject->GetREFSceneInstanceUUID());

	ImGui::SeparatorText(k_defaultINFOSeparatorLabel.data());

	ImGui::Text("%s : %.*s", k_gameObjectNameLabel.data(), static_cast<int>(l_gameObjectDisplayName.size()), l_gameObjectDisplayName.data());
	ImGui::Text("%s : %s",   k_uuidLabel.data(),                            l_uuidString.c_str());

	DrawGameObjectComponentObserverDetails(*l_gameObject);
	ImGui::SeparatorText                   (k_componentSeparatorLabel.data());
	DrawGameObjectTransformComponentDetails(*l_gameObject);
	DrawGameObjectComponentDetails         (*l_gameObject);

	ImGui::Separator();

	DrawAddComponentMenu(l_gameObject);
}
void FWK::Editor::DetailsEditorWindow::DrawGameObjectComponentObserverDetails(GameObject& a_gameObject) const
{
	auto& l_componentEventObserver = a_gameObject.GetMutableREFComponentEventObserver();

	if (!ImGui::CollapsingHeader(k_componentEventObserverLabel.data(), ImGuiTreeNodeFlags_None)) { return; }

	// コンポーネントオブザーバーの生成、破棄を管理
	if (bool l_isChecked = l_componentEventObserver ? true : false;
		ImGui::Checkbox(k_componentEventObserverUseLabel.data(), &l_isChecked))
	{
		if (l_componentEventObserver)
		{
			l_componentEventObserver.reset();
		}
		else
		{
			l_componentEventObserver = std::make_unique<Observer<Enum::ComponentEvent>>();
			l_componentEventObserver->INIT                                             ();
		}
	}

	if (!l_componentEventObserver) { return; }

	l_componentEventObserver->EditInspector();
}
void FWK::Editor::DetailsEditorWindow::DrawGameObjectTransformComponentDetails(const GameObject& a_gameObject) const
{
	const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock();

	if (!l_transformComponent)
	{
		ImGui::TextDisabled(k_invalidTransformComponentMessage.data());

		return;
	}

	if (!ImGui::CollapsingHeader(k_transformComponentHeaderName.data(), ImGuiTreeNodeFlags_None)) { return; }

	// TransformComponentはGameObjectに必須であり、
	// 通常ComponentListとは別に所有されるため、
	// 削除用のXボタンは表示しない
	l_transformComponent->EditInspector();
}
void FWK::Editor::DetailsEditorWindow::DrawGameObjectComponentDetails(GameObject& a_gameObject) const
{
	const auto& l_componentDataList = a_gameObject.GetREFComponentSmartPointerVectorArray().GetREFArrayElementDataList();

	// ComponentListを捜査している途中では削除せず、
	// 捜査終了後にGameObjectへ削除を依頼する
	std::weak_ptr<ComponentBase> l_removeRequestedComponent = {};
	
	for (const auto& l_componentData : l_componentDataList)
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { continue; }

		// 同じ型のComponentを複数保持できる場合でも、
		// ImGui内部では別のItemとして扱えるようにする
		ImGui::PushID(std::to_address(l_componentData.m_type));

		      bool l_isKeepComponent       = true;
		const bool l_isComponentHeaderOpen = ImGui::CollapsingHeader(l_component->GetREFRuntimeTypeINFO().k_name.data(), &l_isKeepComponent, ImGuiTreeNodeFlags_None);

		// 直前に描画したCollapsingHeaderへカーソルが重なっている場合だけ、
		// Componentを取り外す方法をTooltipで表示する
		Utility::DelayedTooltip(k_componentRemoveTooltip.data());

		// CollapsingHeader右端のXを押した場合だけfalseになる
		if (!l_isKeepComponent)
		{
			l_removeRequestedComponent = l_component;

			ImGui::PopID();

			continue;
		}

		// 右端のXを押した場合だけfalseになる
		if (!l_isKeepComponent)
		{
			l_removeRequestedComponent = l_component;

			ImGui::PopID();

			// 捜査中にはComponentListを変更しない。
			// 範囲forを終了してから削除する
			break;
		}

		if (l_isComponentHeaderOpen)
		{
			l_component->EditInspector();
		}

		ImGui::PopID();
	}

	// ComponentListの捜査が完全に終了した後で、
	// GameObjectへ取り外しを依頼する
	if (!l_removeRequestedComponent.expired())
	{
		a_gameObject.RemoveComponent(l_removeRequestedComponent);
	}
}
void FWK::Editor::DetailsEditorWindow::DrawAddComponentMenu(const std::weak_ptr<GameObject>& a_gameObject) const
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject) { return; }

	if (ImGui::Button(k_addComponentButtonName.data()))
	{
		ImGui::OpenPopup(k_addComponentPopupLabel.data());
	}

	if (!ImGui::BeginPopup(k_addComponentPopupLabel.data())) { return; }

	const auto& l_componentFactory = TypeAlias::ComponentSharedFactory::GetInstance();
	const auto& l_factoryMap       = l_componentFactory.GetREFFactoryMap           ();

	for (const auto& l_factoryData : l_factoryMap)
	{
		const auto& l_componentTypeName = l_factoryData.first;

		if (!ImGui::MenuItem(l_componentTypeName.c_str())) { continue; }

		auto l_component = l_componentFactory.Create(l_componentTypeName);

		if (!l_component)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "Component : {}\nComponentFactoryからの生成に失敗しました。");

			continue;
		}

		// ゲームオブジェクトのPostDeserialize関数を呼ぶことで
		// 全てのコンポーネントのポインタの結び付け処理を行う
		l_component->INIT            ();
		l_component->SetOwner        (l_gameObject);
		l_gameObject->AddComponent   (l_component);
		l_gameObject->PostDeserialize();

		ImGui::CloseCurrentPopup();

		break;
	}

	ImGui::EndPopup();
}

std::string_view FWK::Editor::DetailsEditorWindow::FetchVALGameObjectDisplayName(const GameObject& a_gameObject) const
{
	if (const auto& l_gameObjectName = a_gameObject.FetchVALGameObjectName();
		!l_gameObjectName.empty())
	{
		return l_gameObjectName; 
	}

	if (const auto& l_prefabName = a_gameObject.GetREFSceneInstanceName();
		!l_prefabName.empty())
	{
		return l_prefabName; 
	}

	return Constant::k_gameObjectString;
}