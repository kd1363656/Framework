#pragma once

namespace FWK::Utility
{
    // 表示中の一覧の要素と、探している要素が同じか判定する
    // ファイルパスのように==で比べられる型はそのまま比べる
    template <typename Type>
    inline bool IsSameEditorListElement(const Type& a_element, const Type& a_target)
    {
        return a_element == a_target;
    }

    // weak_ptrには==が無いため、lockしたshared_ptr(指している実体のアドレス)で比べる
    // 探している要素が無効なときは、破棄済みの要素(lockするとnullptr)と一致しないように必ず不一致にする
    template <typename Type>
    inline bool IsSameEditorListElement(const std::weak_ptr<Type>& a_element, const std::shared_ptr<Type>& a_target)
    {
        if (!a_target) { return false; }

        return a_element.lock() == a_target;
    }

    // 表示中の一覧から要素を探し、見つかった位置のイテレーターを返す
    // 見つからなければend()を返す(std::findと同じ約束)
    // 例 : { A, B, C } からBを探す → 2番目(添字1)を指すイテレーター
    template <typename ElementType, typename TargetType>
    inline auto FindVALEditorListITR(const std::vector<ElementType>& a_list, const TargetType& a_target)
    {
        return std::ranges::find_if(a_list,
                                    [&a_target](const ElementType& a_element)
                                    {
                                        return IsSameEditorListElement(a_element, a_target);
                                    });
    }

    // カーソルの位置からa_step個ずらした位置のイテレーターを返す
    // 一覧の外へ出るとき(先頭より前・末尾より後)はend()を返し、呼び出し側は「移動しない」として扱う
    // 1列の一覧(アウトライナー・フォルダツリー)は上下で-1/+1、
    // カードのグリッド(アセット一覧)は上下で-1行分/+1行分、左右で-1/+1を渡す
    // 例 : 10個の一覧、カーソルが添字5、1行4枚
    //      上 : 5 - 4 =  1 → 添字1へ移動
    //      下 : 5 + 4 =  9 → 添字9へ移動
    //      もう一度下 : 9 + 4 = 13 → 10以上なので移動しない
    template <typename ElementType>
    inline auto FetchVALEditorListMovedITR(const std::vector<ElementType>& a_list, const typename std::vector<ElementType>::const_iterator& a_cursorITR, const std::ptrdiff_t a_step)
    {
        if (a_cursorITR == a_list.end()) { return a_list.end(); }

        // 添字で計算すると、先頭より前へ出たかを負の数で判定できる
        // (イテレーターをbegin()より前へ動かすのは未定義動作になるため、イテレーターのまま足し引きしない)
        const auto& l_movedIndex = std::distance(a_list.begin(), a_cursorITR) + a_step;

        if (l_movedIndex < Constant::k_editorListFirstIndex ||
            l_movedIndex >= std::ssize(a_list))
        {
            return a_list.end();
        }

        return std::next(a_list.begin(), l_movedIndex);
    }
}