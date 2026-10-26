#pragma once

namespace FWK::Editor
{
    // 「Add ～」で開く、検索とカテゴリー選択ができる生成メニュー
    // コンポーネントの追加やノードエディターのノード作成のように
    // 「タグ(カテゴリー)ごとに登録された型の中から1つ選んで生成する」場面で共通して使う
    //
    // カテゴリーはTypeINFOTagRegistryに登録したタグ名をそのまま使う
    // このクラスは選ばれた型情報を返すだけで、生成そのものは呼び出し側がファクトリーで行う
    // 例 : GenericFactory<...>::GetInstance().Create(std::string{ l_typeINFO->k_name })
    class EditorAddMenu final
    {
    public:

         EditorAddMenu() = default;
        ~EditorAddMenu() = default;

        void Open();

        const TypeINFORegistry::TypeINFO* DrawButton(const std::string_view& a_buttonLabel, const std::string_view&              a_rootLabel, const std::vector<std::string_view>& a_categoryNameList);
        const TypeINFORegistry::TypeINFO* DrawPopup (const std::string_view& a_rootLabel,   const std::vector<std::string_view>& a_categoryNameList);

    private:

        void OpenAt(const ImVec2& a_popupPos, const float a_popupWidth);

        void DrawSearchInputText();

        void DrawCurrentLabel(const std::string_view& a_rootLabel);
        void DrawCenteredText(const std::string_view& a_text)      const;

        void DrawCategoryList(const std::vector<std::string_view>& a_categoryNameList);

        const TypeINFORegistry::TypeINFO* DrawSearchResultList(const std::vector<std::string_view>& a_categoryNameList) const;
        const TypeINFORegistry::TypeINFO* DrawCategoryTypeList()                                                        const;

        const TypeINFORegistry::TypeINFO* DrawTypeSelectableList(std::vector<const TypeINFORegistry::TypeINFO*>& a_typeINFOList) const;

        void ShiftCursorToCenter(const float a_itemWidth) const;

        bool IsSearchHit(const std::string_view& a_typeName) const;

        bool IsSearching() const;

        std::array<char, Constant::k_editorAddMenuSearchInputBufferSize> m_searchInputBuffer = {};

        // 空ならカテゴリー選択中、空でなければそのカテゴリーの型一覧を表示中
        // タグ名の定数(静的寿命)を指している前提でstd::string_viewで持つ
        std::string_view m_selectedCategoryName = {};

        ImVec2 m_popupPos = {};

        float m_popupWidth = k_mousePopupWidth;

        static constexpr std::string_view k_popupLabel          = "##EditorAddMenuPopup";
        static constexpr std::string_view k_searchInputLabel    = "##EditorAddMenuSearch";
        static constexpr std::string_view k_searchHintLabel     = "検索";
        static constexpr std::string_view k_listChildLabel      = "##EditorAddMenuList";
        static constexpr std::string_view k_searchResultLabel   = "検索結果";
        static constexpr std::string_view k_noSearchHitLabel    = "一致する項目がありません。";
        static constexpr std::string_view k_noCategoryTypeLabel = "このカテゴリーには項目がありません。";
        static constexpr std::string_view k_backSelectableLabel = "##EditorAddMenuBack";

        static constexpr std::string_view k_imguiFontAwesomeSearchIcon = "\xEF\x80\x82";

        static constexpr float k_buttonWidth           = 230.0F;
        static constexpr float k_buttonAutoHeight      = 0.0F;
        static constexpr float k_mousePopupWidth       = 230.0F;
        static constexpr float k_popupMINHeight        = 0.0F;
        static constexpr float k_popupMAXHeight        = std::numeric_limits<float>::max();
        static constexpr float k_listMINSize           = 0.0F;
        static constexpr float k_listMAXWidth          = std::numeric_limits<float>::max();
        static constexpr float k_listMAXHeight         = 320.0F;
        static constexpr float k_centerRatio           = 0.5F;
        static constexpr float k_minCursorOffsetX      = 0.0F;
        static constexpr float k_searchInputFullWidth  = -FLT_MIN;
        static constexpr float k_backArrowHeightRatio  = 0.7F;
        static constexpr float k_backArrowWidthRatio   = 0.6F;
        static constexpr float k_backArrowThickness    = 1.5F;

        static constexpr int k_keyboardFocusNextItem = 0;
    };
}