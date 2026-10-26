#include "TypeINFOTagRegistry.h"

void FWK::TypeINFOTagRegistry::Register(const std::string_view& a_tagName, const TypeINFORegistry::TypeINFO& a_typeINFO)
{
    FWK_ASSERT_RETURN_IF(a_tagName.empty(), "タグ名が空になっており、型情報のタグ登録に失敗しました。");

    // タグが未登録なら空のリストが作成される
    auto& l_typeINFOList = m_tagTypeINFOListMap[a_tagName];

    const bool l_isDuplicated = std::ranges::find(l_typeINFOList, &a_typeINFO) != l_typeINFOList.end();

    FWK_ASSERT_RETURN_IF(l_isDuplicated, "同じタグに同じ型情報が重複して登録されており、型情報のタグ登録に失敗しました。");

    l_typeINFOList.emplace_back(&a_typeINFO);

    FWK_ADD_LOG(Constant::k_imguiDebugSuccessColor,
                "[型情報タグ登録]\nTag : {}\nName : {}\nStaticTypeID : {}\n型情報のタグ登録に成功しました。\n",
                a_tagName.data(),
                a_typeINFO.k_name.data(),
                a_typeINFO.k_staticTypeID);
}

const FWK::TypeINFOTagRegistry::TypeINFOList* FWK::TypeINFOTagRegistry::FindPTRTypeINFOListByTag(const std::string_view& a_tagName) const
{
    const auto& l_itr = m_tagTypeINFOListMap.find(a_tagName);

    // 該当するタグの型情報リストを取得できなければreturn;
    if (l_itr == m_tagTypeINFOListMap.end()) { return nullptr; }

    return &l_itr->second;
}
