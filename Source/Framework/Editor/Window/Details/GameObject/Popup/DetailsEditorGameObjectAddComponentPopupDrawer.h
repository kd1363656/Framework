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

        void Draw(const std::weak_ptr<GameObject>& a_gameObject,
                  const std::string_view&          a_popupLabel,
                  const ImVec2&                    a_popupPosition,
                  const float                      a_popupWidth);

    private:

        void Reset();

        void DrawSearchBar();
        void DrawHeader   ();
        void DrawItemList (const std::vector<std::string>& a_itemList, const std::weak_ptr<GameObject>& a_gameObject);

        void DrawSearchIcon  ()                               const;
        void DrawTagArrow    ()                               const;
        void DrawBackArrow   ()                               const;
        void DrawCenteredText(const std::string_view& a_text) const;

        void HandleShortcut(const std::vector<std::string>& a_itemList, const std::weak_ptr<GameObject>& a_gameObject, const bool a_wasSearching);

        void DecideItem(const std::weak_ptr<GameObject>& a_gameObject, const std::string& a_item);

        void ClearSelectedTag();

        void AddComponent(const std::weak_ptr<GameObject>& a_gameObject, const std::string& a_componentTypeName) const;

        bool IsSearching        ()                                        const;
        bool IsMatchedSearchText(const std::string& a_componentTypeName) const;

        bool CanAddComponent(const std::string& a_componentTypeName, const GameObjectComponentContainer& a_componentContainer) const;

        std::vector<std::string> FetchVALItemList(const std::weak_ptr<GameObject>& a_gameObject) const;

        static constexpr std::string_view k_imguiFontAwesomeMagnifyingGlassIcon = "\xEF\x80\x82";
        static constexpr std::string_view k_imguiFontAwesomeCaretRightIcon      = "\xEF\x83\x9A";
        static constexpr std::string_view k_imguiFontAwesomeCaretLeftIcon       = "\xEF\x83\x99";
        static constexpr std::string_view k_searchInputTextLabel                = "##DetailsEditorGameObjectAddComponentSearch";
        static constexpr std::string_view k_searchHintLabel                     = "検索";
        static constexpr std::string_view k_clearSearchButtonLabel              = "\xEF\x80\x8D##DetailsEditorGameObjectAddComponentClearSearch";
        static constexpr std::string_view k_backHeaderLabel                     = "##DetailsEditorGameObjectAddComponentBackHeader";
        static constexpr std::string_view k_itemListChildLabel                  = "##DetailsEditorGameObjectAddComponentItemList";
        static constexpr std::string_view k_componentTagLabel                   = "コンポーネントの種類";
        static constexpr std::string_view k_searchResultLabel                   = "検索結果";
        static constexpr std::string_view k_noAddableComponentLabel             = "追加できるコンポーネントがありません";

        static constexpr ImVec2 k_itemListChildSize = { 0.0F, 240.0F };

        static constexpr float k_autoFitPopupHeight       = 0.0F;
        static constexpr float k_framePaddingBothSidesNUM = 2.0F;
        static constexpr float k_searchIconScale          = 1.3F;
        static constexpr float k_noWrapWidth              = 0.0F;
        static constexpr float k_unlimitedTextWidth       = std::numeric_limits<float>::max();

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