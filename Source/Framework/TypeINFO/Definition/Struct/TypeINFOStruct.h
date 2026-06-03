#pragma once

namespace FWK::Struct
{
	// 型情報を扱う構造体
	struct TypeINFO final
	{
		explicit TypeINFO(const TypeINFO* const a_baseINFO, const std::string_view& a_name, const TypeAlias::StaticTypeID a_staticTypeID) :
			k_baseINFO    (a_baseINFO),
			k_name        (a_name),
			k_staticTypeID(a_staticTypeID)
		{}
		~TypeINFO() = default;

		const TypeINFO* const         k_baseINFO;
		const std::string_view        k_name;
		const TypeAlias::StaticTypeID k_staticTypeID;
	};
}