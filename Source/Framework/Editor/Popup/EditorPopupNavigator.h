#pragma once

namespace FWK::Editor
{
    class EditorPopupNavigator final
    {
    public:

         EditorPopupNavigator() = default;
        ~EditorPopupNavigator() = default;

        void Begin(const Enum::EditorPopupNavigationAxis a_axis, const std::size_t a_initialKeySelectedIndex = Constant::k_noKeySelectedIndex);
        void End  ();

        bool MenuItem(const char* a_label, const char*   a_shortcut = nullptr, const bool a_isEnabled = true);
        bool Button  (const char* a_label, const ImVec2& a_size     = ImVec2{});

    private:

        bool ApplyItemState(const bool a_isClicked, const bool a_isEnabled);

        void MoveKeySelection(const bool a_isNext);

        void DrawItemFrame() const;

        static constexpr float k_itemFrameThickness = 2.0F;

        static constexpr std::size_t k_emptyItemCount   = 0ULL;
        static constexpr std::size_t k_firstItemIndex   = 0ULL;
        static constexpr std::size_t k_keySelectionStep = 1ULL;

        static constexpr int k_pushStyleColorNUM = 4;

        std::vector<bool> m_itemEnabledList        = {};
        std::vector<bool> m_drawingItemEnabledList = {};

        std::size_t m_keySelectedIndex = Constant::k_noKeySelectedIndex;

        bool m_isConfirmRequested = false;
    };
}