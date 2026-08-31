#pragma once

namespace FWK::Editor
{
	class ContentBrowserEditorWindow;
}

namespace FWK::Editor
{
	class WorldOutlinerEditorWindow final : public EditorWindowBase
	{
	public:

		 WorldOutlinerEditorWindow()          = default;
		~WorldOutlinerEditorWindow() override = default;

		void Draw() override;

	private:

		bool CreateDroppedPrefabInstance(const ContentBrowserEditorWindow& a_contentBrowserEditorWindow);

		void DrawSceneNode(const std::weak_ptr<Scene>& a_scene);

		void                             DrawGameObjectNode           (const std::weak_ptr<GameObject>& a_gameObject);
		Struct::GameObjectNodeDrawResult DrawGameObjectNodeHeader     (const std::weak_ptr<GameObject>& a_gameObject);
		void                             DrawGameObjectNodeContextMenu(const std::weak_ptr<GameObject>& a_gameObject);
		bool                             DrawGameObjectNodeDragDrop   (const std::weak_ptr<GameObject>& a_gameObject);
		void                             DrawRootDropArea             ();

		bool HasValidChildGameObject(const std::weak_ptr<GameObject>& a_gameObject) const;

		void ApplySceneNodeSelection();

		void ApplyGameObjectNodeSelection          (const std::weak_ptr<GameObject>& a_gameObject);
		void ApplySelectedGameObjectDestroyShortcut();
		void ApplySelectedGameObjectDestroyRequest ();

		bool UnparentDroppedGameObject();

		void RequestAddGameObject();

		static constexpr ImVec4 k_prefabGameObjectTextColor = { 0.30F,
		                                                        0.65F, 
		                                                        1.00F,
		                                                        1.00F };

		static constexpr ImVec4 k_nonPrefabGameObjectTextColor = { 1.00F,
																   0.35F,
																   0.35F,
																   1.00f };

		static constexpr std::string_view k_editorName                            = "アウトライナー";
		static constexpr std::string_view k_sceneNodeInternalLabel                = "###SceneNode";
		static constexpr std::string_view k_rootDropAreaLabel                     = "##RootDropArea";
		static constexpr std::string_view k_rootContextMenuLabel                  = "##RootContextMenu";
		static constexpr std::string_view k_addRootGameObjectMenuItemText         = "空のGameObjectを追加";
		static constexpr std::string_view k_destroyGameObjectMenuItemText         = "GameObjectを削除";
		static constexpr std::string_view k_destroyGameObjectMenuItemShortcutText = "Delete";

		static constexpr std::size_t k_emptySelectionCount  = 0ULL;
		
		static constexpr float k_rootDropAreaMINWidth  = 1.0F;
		static constexpr float k_rootDropAreaMINHeight = 32.0F;

		WorldOutlinerEditorWindowGameObjectSelection m_gameObjectSelection = {};
		WorldOutlinerEditorWindowGameObjectHierarchy m_gameObjectHierarchy = {};
		WorldOutlinerEditorWindowGameObjectRename    m_gameObjectRename    = {};
		WorldOutlinerEditorWindowSceneRename         m_sceneRename         = {};

		bool m_isSelectedGameObjectDestroyRequested  = false;
		bool m_isSceneSelected                       = false;

		FWK_DEFINE_TYPE_INFO(WorldOutlinerEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::WorldOutlinerEditorWindow)