#pragma once

namespace FWK::Editor
{
	class WorldOutlinerEditorWindowGameObjectHierarchy final
	{
	public:

		 WorldOutlinerEditorWindowGameObjectHierarchy() = default;
		~WorldOutlinerEditorWindowGameObjectHierarchy() = default;

		void RequestApplyParent(const std::weak_ptr<GameObject>& a_parentGameObject, const std::weak_ptr<GameObject>& a_childGameObject);
		void RequestUnparent   (const std::weak_ptr<GameObject>& a_childGameObject);

		void ApplyRequest();
		
	private:

		void ClearRequest();

		Struct::GameObjectNodeHierarchyChangeRequest m_gameObjectNodeHierarchyChangeRequest = {};
	};
}