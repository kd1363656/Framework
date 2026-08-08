#pragma once

namespace FWK::Editor
{
	class WorldOutlinerEditorWindowGameObjectSelection final
	{
	public:

		 WorldOutlinerEditorWindowGameObjectSelection() = default;
		~WorldOutlinerEditorWindowGameObjectSelection() = default;


		void Synchronize();

		void SelectSingleGameObject   (const std::weak_ptr<GameObject>& a_gameObject);
		void SelectRangeGameObject    (const std::weak_ptr<GameObject>& a_gameObject);
		void ToggleGameObjectSelection(const std::weak_ptr<GameObject>& a_gameObject);

		void ClearSelectedGameObjects();

		bool ContainsSelectedGameObject(const std::weak_ptr<GameObject>& a_gameObject) const;

		std::size_t FetchVALSelectedGameObjectCount() const;

	private:

		Enum::GameObjectRangeSelectionState AddGameObjectRangeSelectionRecursive(const std::weak_ptr<GameObject>&          a_gameObject,
			                                                                     const std::weak_ptr<GameObject>&          a_rangeAnchorObject,
			                                                                     const std::weak_ptr<GameObject>&          a_rangeEndGameObject,
			                                                                           Enum::GameObjectRangeSelectionState a_rangeSelectionState);

		void AddSelectedGameObject(const std::weak_ptr<GameObject>& a_gameObject);

		void RemoveSelectedGameObject(const std::weak_ptr<GameObject>& a_gameObject);

		static constexpr std::size_t k_initialSelectedGameObjectCount = 0ULL;

		Utility::SmartPointerVectorArray<std::weak_ptr<GameObject>> m_selectedGameObjectVectorArray = {};
	};
}