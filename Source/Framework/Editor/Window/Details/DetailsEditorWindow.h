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
		void DrawGameObjectComponentObserverDetails (      GameObject&                a_gameObject) const;
		void DrawGameObjectTransformComponentDetails(const GameObject&                a_gameObject) const;
		void DrawGameObjectComponentDetails         (      GameObject&                a_gameObject) const;
		void DrawAddComponentMenu                   (const std::weak_ptr<GameObject>& a_gameObject) const;

		std::string_view FetchVALGameObjectDisplayName(const GameObject& a_gameObject) const;

		static constexpr std::string_view k_editorName                          = "詳細";
		static constexpr std::string_view k_defaultINFOSeparatorLabel           = "基本情報";
		static constexpr std::string_view k_componentSeparatorLabel             = "コンポーネント";
		static constexpr std::string_view k_componentEventObserverLabel         = "コンポーネントオブザーバー";
		static constexpr std::string_view k_componentEventObserverUseLabel      = "コンポーネントオブザーバーの使用";
		static constexpr std::string_view k_componentEventObserverSelectorLabel = "イベント送受信セレクター";
		static constexpr std::string_view k_noSelectedGameObjectMessage         = "ゲームオブジェクトが選択されていません。";
		static constexpr std::string_view k_gameObjectNameLabel                 = "名前";
		static constexpr std::string_view k_destroyedGameObjectMessage          = "削除申請済みのゲームオブジェクトは編集できません。";
		static constexpr std::string_view k_invalidTransformComponentMessage    = "TransformComponentを取得できませんでした。";
		static constexpr std::string_view k_uuidLabel                           = "UUID";
		static constexpr std::string_view k_transformComponentHeaderName        = "TransformComponent";
		static constexpr std::string_view k_addComponentButtonName              = "コンポーネントを追加";
		static constexpr std::string_view k_addComponentPopupLabel              = "##AddComponentPopup";
		static constexpr std::string_view k_componentRemoveTooltip              = "右端のxを押すと、このコンポーネントを取り外せます。";

		static constexpr std::size_t k_initialComponentIndex = 0ULL;

		FWK_DEFINE_TYPE_INFO(DetailsEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::DetailsEditorWindow)