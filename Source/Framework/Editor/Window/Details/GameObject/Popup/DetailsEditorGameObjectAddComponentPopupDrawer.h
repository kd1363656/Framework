#pragma once

namespace FWK
{
    class GameObject;
    class GameObjectComponentContainer;
}

namespace FWK::Editor
{
    class DetailsEditorGameObjectAddComponentPopupDrawer final
    {
    public:

         DetailsEditorGameObjectAddComponentPopupDrawer() = default;
        ~DetailsEditorGameObjectAddComponentPopupDrawer() = default;

        void Draw(const std::string_view& a_popupLabel,
                  const ImVec2&           a_popupPosition,
                  const float             a_popupWidth,
                        GameObject&       a_gameObject);

    private:

        void Reset();

        void DrawSearchBar();
        void DrawHeader   ();
        void DrawItemList (const std::vector<std::string>& a_itemList, GameObject& a_gameObject);

        void DrawCenteredText(const std::string_view& a_text) const;

        void HandleShortcut(const std::vector<std::string>& a_itemList, const bool a_wasSearching, GameObject& a_gameObject);

        void DecideItem(const std::string& a_item, GameObject& a_gameObject);

        void ClearSelectedTag();

        void AddComponent(const std::string& a_componentTypeName, GameObject& a_gameObject) const;

        bool IsSearching        ()                                        const;
        bool IsMatchedSearchText(const std::string& a_componentTypeName) const;

        bool CanAddComponent(const std::string& a_componentTypeName, const GameObjectComponentContainer& a_componentContainer) const;

        std::vector<std::string> FetchVALItemList(const GameObject& a_gameObject) const;

        static constexpr std::string_view k_imguiFontAwesomeMagnifyingGlassIcon = "\xEF\x80\x82";
        static constexpr std::string_view k_searchInputTextLabel                = "##DetailsEditorGameObjectAddComponentSearch";
        static constexpr std::string_view k_searchHintLabel                     = "検索";
        static constexpr std::string_view k_clearSearchButtonLabel              = "\xEF\x80\x8D##DetailsEditorGameObjectAddComponentClearSearch";
        static constexpr std::string_view k_backArrowButtonLabel                = "##DetailsEditorGameObjectAddComponentBack";
        static constexpr std::string_view k_itemListChildLabel                  = "##DetailsEditorGameObjectAddComponentItemList";
        static constexpr std::string_view k_componentTagLabel                   = "コンポーネントの種類";
        static constexpr std::string_view k_searchResultLabel                   = "検索結果";
        static constexpr std::string_view k_noAddableComponentLabel             = "追加できるコンポーネントがありません";

        static constexpr ImVec2 k_itemListChildSize = { 0.0F, 240.0F };

        static constexpr float k_autoFitPopupHeight       = 0.0F;
        static constexpr float k_framePaddingBothSidesNUM = 2.0F;

        static constexpr std::size_t k_searchBufferSize      = 256ULL;
        static constexpr std::size_t k_initialHighlightIndex = 0ULL;
        static constexpr std::size_t k_highlightStep         = 1ULL;

        std::array<char, k_searchBufferSize> m_searchBuffer = {};

        std::string m_selectedTag = {};

        std::size_t m_highlightIndex = k_initialHighlightIndex;

        bool m_shouldFocusSearchBar    = false;
        bool m_shouldScrollToHighlight = false;
    };
}