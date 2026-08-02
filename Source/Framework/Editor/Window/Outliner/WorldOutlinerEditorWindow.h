#pragma once

namespace FWK::Editor
{
	class WorldOutlinerEditorWindow final : public EditorWindowBase
	{
	public:

		 WorldOutlinerEditorWindow()          = default;
		~WorldOutlinerEditorWindow() override = default;

		void Draw() override;

	private:

		bool DrawGameObjectNode(const std::shared_ptr<GameObject>& a_gameObject) const;

		void DrawRootDropArea(Scene& a_scene);

		bool TryUnparentDroppedGameObject(Scene& a_scene) const;

		void RequestAddGameObject(Scene& a_scene);

		static constexpr std::string_view k_editorName                     = "アウトライナー";
		static constexpr std::string_view k_gameObjectDragDropPayloadLabel = "ゲームオブジェクト";
		static constexpr std::string_view k_rootDropAreaLabel              = "##RootDropArea";
		static constexpr std::string_view k_rootContextMenuLabel           = "##RootContextMenu";
		static constexpr std::string_view k_addRootGameObjectMenuItemName  = "空のGameObjectを追加";
		static constexpr std::string_view k_destroyGameObjectMenuItemName  = "GameObjectを削除";
		
		static constexpr float k_rootDropAreaMINWidth  = 1.0F;
		static constexpr float k_rootDropAreaMINHeight = 32.0F;

		FWK_DEFINE_TYPE_INFO(WorldOutlinerEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::WorldOutlinerEditorWindow)