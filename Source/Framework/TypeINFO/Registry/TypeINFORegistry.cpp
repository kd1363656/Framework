#include "TypeINFORegistry.h"

void FWK::TypeINFORegistry::Register(const Struct::TypeINFO& a_typeINFO)
{
	FWK_ASSERT_RETURN_IF_FAILED(a_typeINFO.k_staticTypeID == Constant::k_invalidStaticTypeID, "無効なIDを検出しました、型情報の確認をしてください");
	FWK_ASSERT_RETURN_IF_FAILED(a_typeINFO.k_name.empty(),									   "文字列が空です、型情報登録の確認をしてください。");

	m_typeINFONameMap.try_emplace        (a_typeINFO.k_name,         &a_typeINFO);
	m_typeINFOStaticTypeIDMap.try_emplace(a_typeINFO.k_staticTypeID, &a_typeINFO);
}

const FWK::Struct::TypeINFO* FWK::TypeINFORegistry::FindByName(const std::string_view& a_name) const
{
	const auto& l_itr = m_typeINFONameMap.find(a_name);

	// 該当する名前の型情報を取得できなければreturn;
	if (l_itr == m_typeINFONameMap.end()) { return nullptr; }

	return l_itr->second;
}
const FWK::Struct::TypeINFO* FWK::TypeINFORegistry::FindByID(const TypeAlias::StaticTypeID a_staticTypeID) const
{
	const auto& l_itr = m_typeINFOStaticTypeIDMap.find(a_staticTypeID);

	// 該当するStaticIDの型情報を取得できなければreturn;
	if (l_itr == m_typeINFOStaticTypeIDMap.end()) { return nullptr; }

	return l_itr->second;
}