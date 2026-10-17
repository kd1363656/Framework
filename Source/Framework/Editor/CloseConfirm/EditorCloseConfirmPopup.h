#pragma once

namespace FWK::Editor
{
    class EditorManager;
}

namespace FWK::Editor
{
    class EditorCloseConfirmPopup final
    {
    public:

         EditorCloseConfirmPopup() = default;
        ~EditorCloseConfirmPopup() = default;

        void Request();

        void Draw(EditorManager& a_editorManager);

        void ApproveClose();

        bool IsCloseApproved() const { return m_isCloseApproved; }

    private:

        void DrawMessage() const;

        static constexpr std::string_view k_titleLabel            = "保存の確認";
        static constexpr std::string_view k_messageFormat         = "シーン「{}」には保存されていない変更があります。\n終了する前に保存しますか？";
        static constexpr std::string_view k_saveAndCloseLabel     = "保存して終了";
        static constexpr std::string_view k_closeWithoutSaveLabel = "保存せず終了";
        static constexpr std::string_view k_cancelLabel           = "キャンセル";
        static constexpr std::string_view k_emptySceneNameLabel   = "Untitled";

        static constexpr std::array<std::string_view, 3ULL> k_buttonLabelList = { k_saveAndCloseLabel,
                                                                                  k_closeWithoutSaveLabel,
                                                                                  k_cancelLabel };

        static constexpr ImVec2 k_modalPopupPivot = { 0.5F, 0.5F };

        static constexpr std::size_t k_initialKeySelectedButtonIndex = 0ULL;

        EditorPopupNavigator m_popupNavigator = {};

        bool m_isOpenRequested = false;
        bool m_isCloseApproved = false;
    };
}