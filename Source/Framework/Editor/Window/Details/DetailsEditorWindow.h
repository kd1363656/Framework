#pragma once

namespace FWK::Editor
{
	class DetailsEditorWindow final : public EditorWindowBase
	{
	public:

		 DetailsEditorWindow()          = default;
		~DetailsEditorWindow() override = default;

		void Draw() override;

	private:

		void DrawGameObjectDetails                  (const std::weak_ptr<GameObject>& a_gameObject) const;
		void DrawGameObjectTransformComponentDetails(const std::weak_ptr<GameObject>& a_gameObject) const;
		void DrawGameObjectComponentDetails         (const std::weak_ptr<GameObject>& a_gameObject) const;
		void DrawAddComponentMenu                   (const std::weak_ptr<GameObject>& a_gameObject) const;

		std::string_view FetchVALGameObjectDisplyaName(const GameObject& a_gameObject) const;

		static constexpr std::string_view k_editorName                       = "詳細";
		static constexpr std::string_view k_noSelectedGameObjectMessage      = "GameObjectが選択されていません。";
		static constexpr std::string_view k_gameObjectNameLabel              = "名前";
		static constexpr std::string_view k_destroyedGameObjectMessage       = "削除申請済みのGameObjectは編集できません。";
		static constexpr std::string_view k_invalidTransformComponentMessage = "TransformComponentを取得できませんでした。";
		static constexpr std::string_view k_uuidLabel                        = "UUID";
		static constexpr std::string_view k_transformComponentHeaderName     = "TransformComponent";
		static constexpr std::string_view k_addComponentButtonName           = "コンポーネントを追加";
		static constexpr std::string_view k_addComponentPopupLabel           = "##AddComponentPopup";
		static constexpr std::string_view k_componentRemoveDescription       = "Component見出し右端の×で取り外せます。";

		static constexpr std::size_t k_initialComponentIndex = 0ULL;

		FWK_DEFINE_TYPE_INFO(DetailsEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::DetailsEditorWindow)