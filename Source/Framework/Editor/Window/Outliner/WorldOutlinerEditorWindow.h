#pragma once

namespace FWK::Editor
{
	class WorldOutlinerEditorWindow final : public EditorWindowBase
	{
	private:

		enum class GameObjectRangeSelectionState 
		{
			None,
			BeforeRange,
			Selecting,
			Completed
		};

		enum class GameObjectHierarchyChangeRequestType
		{
			None,
			ApplyParent,
			Unparent
		};

		struct GameObjectNodeDrawResult final
		{
			bool m_hasChildGameObject = false;
			bool m_isNodeOpen         = false;
		};

		struct GameObjectNodeHierarchyChangeRequest final
		{
			std::weak_ptr<GameObject> m_parentGameObject = {};
			std::weak_ptr<GameObject> m_childGameObject  = {};

			GameObjectHierarchyChangeRequestType m_type = GameObjectHierarchyChangeRequestType::None;
		};

	public:

		 WorldOutlinerEditorWindow()          = default;
		~WorldOutlinerEditorWindow() override = default;

		void Draw() override;

	private:

		void SynchronizeSelectedGameObject();

		void                     DrawGameObjectNode           (const std::weak_ptr<GameObject>& a_gameObject);
		GameObjectNodeDrawResult DrawGameObjectNodeHeader     (const std::weak_ptr<GameObject>& a_gameObject);
		void                     DrawGameObjectNodeContextMenu(const std::weak_ptr<GameObject>& a_gameObject);
		bool                     DrawGameObjectNodeDragDrop   (const std::weak_ptr<GameObject>& a_gameObject);
		void                     DrawGameObjectRenameInput    ();
		void                     DrawRootDropArea             ();

		bool HasValidChildGameObject(const std::weak_ptr<GameObject>& a_gameObject) const;

		void ApplyGameObjectNodeSelection(const std::weak_ptr<GameObject>& a_gameObject);

		void SelectSingleGameObject(const std::weak_ptr<GameObject>& a_gameObject);
		void SelectGameObjectRange (const std::weak_ptr<GameObject>& a_gameObject);

		void ToggleGameObjectSelection(const std::weak_ptr<GameObject>& a_gameObject);

		GameObjectRangeSelectionState AddGameObjectRangeSelectionRecursive(const std::weak_ptr<GameObject>&    a_gameObject,
			                                                               const std::weak_ptr<GameObject>&    a_rangeAnchorObject,
			                                                               const std::weak_ptr<GameObject>&    a_rangeEndGameObject,
			                                                                     GameObjectRangeSelectionState a_rangeSelectionState);

		void AddSelectedGameObject(const std::weak_ptr<GameObject>& a_gameObject);

		void RemoveSelectedGameObject(const std::weak_ptr<GameObject>& a_gameObject);

		void ClearSelectedGameObjects                 ();
		void ClearGameObjectRenameState               ();
		void ClearGameObjectNodeHierarchyChangeRequest();

		bool ContainsSelectedGameObject(const std::weak_ptr<GameObject>& a_gameObject) const;

		void RequestApplyParent(const std::weak_ptr<GameObject>& a_parentGameObject, const std::weak_ptr<GameObject>& a_childGameObject);
		void RequestUnparent   (const std::weak_ptr<GameObject>& a_childGameObject);

		void ApplyGameObjectNodeHierarchyChangeRequest();
		void ApplySelectedGameObjectDestroyRequest    ();

		void RequestGameObjectRename(const std::weak_ptr<GameObject>& a_gameObject);

		void ConfirmGameObjectRename();

		void CancelGameObjectRename();

		bool IsGameObjectRenameTarget(const std::weak_ptr<GameObject>& a_gameObject) const;

		bool UnparentDroppedGameObject();

		void RequestAddGameObject();

		std::size_t FetchVALSelectedGameObjectCount() const;

		static constexpr std::string_view k_editorName                     = "アウトライナー";
		static constexpr std::string_view k_gameObjectDragDropPayloadLabel = "ゲームオブジェクト";
		static constexpr std::string_view k_gameObjectRenameInputLabel     = "##GameObjectRenameInput";
		static constexpr std::string_view k_rootDropAreaLabel              = "##RootDropArea";
		static constexpr std::string_view k_rootContextMenuLabel           = "##RootContextMenu";
		static constexpr std::string_view k_addRootGameObjectMenuItemName  = "空のGameObjectを追加";
		static constexpr std::string_view k_destroyGameObjectMenuItemName  = "GameObjectを削除";

		static constexpr std::size_t k_singleSelectionCount           = 1ULL;
		static constexpr std::size_t k_initialSelectedGameObjectCount = 0ULL;

		static constexpr float k_rootDropAreaMINWidth  = 1.0F;
		static constexpr float k_rootDropAreaMINHeight = 32.0F;

		Utility::SmartPointerVectorArray<std::weak_ptr<GameObject>> m_selectedGameObjectVectorArray = {};

		GameObjectNodeHierarchyChangeRequest m_gameObjectNodeHierarchyChangeRequest = {};

		std::string m_gameObjectRenameBuffer = {};

		bool m_isGameObjectRenameActive              = false;
		bool m_isGameObjectRenameInputFocusRequested = false;
		bool m_isSelectedGameObjectDestroyRequested  = false;

		FWK_DEFINE_TYPE_INFO(WorldOutlinerEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::WorldOutlinerEditorWindow)