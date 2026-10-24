#pragma once

namespace FWK::Editor
{
    // EditorWindowでコピー、切り取りした要素を貼り付けまで覚えておくクラス
    // AssetBrowserはファイルパス、WorldOutlinerはGameObjectのUUIDを入れる
    // (入れる物の型が違うだけで処理は同じなので、テンプレートで共通化している)
    //
    // ElementType : 覚えておく要素の型(std::unordered_setに入れるため、std::hashで扱える型にする)
    template <typename ElementType>
    class EditorWindowClipboard final
    {
    public:

         EditorWindowClipboard() = default;
        ~EditorWindowClipboard() = default;

        void Apply(const std::vector<ElementType>& a_elementList, const Enum::EditorWindowClipboardOperationType a_operationType)
        {
            // 前回コピー、切り取りした要素が残っていると意図しない貼り付けになるため
            // ここで削除する
            Clear();

            m_operationType = a_operationType;

            // a_elementListの順番を維持したまま格納する
            // 同じ要素が複数渡された場合は1つだけ格納する
            for (const auto& l_element : a_elementList)
            {
                // 既に登録されているなら登録しない
                if (m_elementSet.contains(l_element)) { continue; }

                m_elementSet.emplace      (l_element);
                m_elementList.emplace_back(l_element);
            }
        }

        void Clear()
        {
            m_elementList.clear();
            m_elementSet.clear ();

            m_operationType = Enum::EditorWindowClipboardOperationType::Invalid;
        }

        bool Contains(const ElementType& a_element) const { return m_elementSet.contains(a_element); }

        bool IsEmpty() const { return m_elementList.empty(); }

        // 切り取りした要素か(切り取り対象は半透明で描画するため)
        bool IsCutTarget(const ElementType& a_element) const
        {
            return m_operationType == Enum::EditorWindowClipboardOperationType::Cut &&
                   Contains(a_element);
        }

        const auto& GetREFElementList() const { return m_elementList; }

        auto GetVALOperationType() const { return m_operationType; }

    private:

        std::unordered_set<ElementType> m_elementSet = {};

        // 貼り付け順を維持するためのリスト
        std::vector<ElementType> m_elementList = {};

        Enum::EditorWindowClipboardOperationType m_operationType = Enum::EditorWindowClipboardOperationType::Invalid;
    };
}