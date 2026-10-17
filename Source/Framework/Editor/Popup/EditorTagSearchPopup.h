#pragma once

namespace FWK::Editor
{
    class EditorTagSearchPopup final
    {
    public:

         EditorTagSearchPopup() = default;
        ~EditorTagSearchPopup() = default;

        std::string_view Draw(const std::vector<Struct::TaggedFactoryTagGroup>& a_tagGroupList,
                              const std::string_view&                           a_popupLabel,
                              const ImVec2&                                     a_popupPosition,
                              const Struct::EditorTagSearchPopupText&           a_popupText,
                              const std::size_t&                                a_maxVisibleItemCount,
                              const float                                       a_popupWidth);

    private:

        void Reset();

        void DrawSearchBar();
        void DrawHeader   (const Struct::EditorTagSearchPopupText& a_popupText);

        std::string_view DrawItemList(const std::vector<std::string_view>&    a_itemList,
                                      const Struct::EditorTagSearchPopupText& a_popupText,
                                      const std::size_t&                      a_maxVisibleItemCount,
                                      const bool                              a_isTagList);

        void DrawSearchIcon  ()                                                          const;
        void DrawItemEdgeIcon(const std::string_view& a_icon, const bool a_isRightEdge) const;
        void DrawCenteredText(const std::string_view& a_text)                            const;

        std::string_view HandleShortcut(const std::vector<std::string_view>& a_itemList, const bool a_wasSearching);

        std::string_view DecideItem(const std::string_view& a_item);

        void ClearSelectedTag();

        bool IsSearching        ()                               const;
        bool IsTagList          ()                               const;
        bool IsMatchedSearchText(const std::string_view& a_text) const;

        std::vector<std::string_view> FetchVALItemList(const std::vector<Struct::TaggedFactoryTagGroup>& a_tagGroupList) const;

        static constexpr std::string_view k_imguiFontAwesomeMagnifyingGlassIcon = "\xEF\x80\x82";
        static constexpr std::string_view k_imguiFontAwesomeCaretRightIcon      = "\xEF\x83\x9A";
        static constexpr std::string_view k_imguiFontAwesomeCaretLeftIcon       = "\xEF\x83\x99";
        static constexpr std::string_view k_searchInputTextLabel                = "##EditorTagSearchPopupSearch";
        static constexpr std::string_view k_searchHintText                      = "検索";
        static constexpr std::string_view k_clearSearchButtonLabel              = "\xEF\x80\x8D##EditorTagSearchPopupClearSearch";
        static constexpr std::string_view k_backHeaderLabel                     = "##EditorTagSearchPopupBackHeader";
        static constexpr std::string_view k_itemListChildLabel                  = "##EditorTagSearchPopupItemList";
        static constexpr std::string_view k_searchResultText                    = "検索結果";

        static constexpr ImVec2 k_itemListChildSize = { 0.0F, 0.0F };

        static constexpr float k_autoFitPopupHeight = 0.0F;
        static constexpr float k_searchIconScale    = 1.3F;
        static constexpr float k_noWrapWidth        = 0.0F;
        static constexpr float k_unlimitedTextWidth = std::numeric_limits<float>::max();
        static constexpr float k_minItemListWidth   = 0.0F;
        static constexpr float k_maxItemListWidth   = std::numeric_limits<float>::max();

        static constexpr std::size_t k_searchBufferSize      = 256ULL;
        static constexpr std::size_t k_initialHighlightIndex = 0ULL;
        static constexpr std::size_t k_highlightStep         = 1ULL;
        static constexpr std::size_t k_minVisibleItemCount   = 1ULL;

        std::array<char, k_searchBufferSize> m_searchBuffer = {};

        std::string_view m_selectedTag = {};

        std::size_t m_highlightIndex = k_initialHighlightIndex;

        bool m_shouldFocusSearchBar    = false;
        bool m_shouldScrollToHighlight = false;
    };
}