#pragma once

namespace FWK::Utility
{
	template <typename Type>
	TypeAlias::StaticTypeID GetVALStaticTypeID()
	{
		const auto l_staticTypeID = Type::GetREFTypeINFO().k_staticTypeID;

		// 無効な値ならAssert
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_staticTypeID == Constant::k_invalidStaticTypeID, "無効なStaticTypeIDを返そうとしました", Constant::k_invalidStaticTypeID);

		return l_staticTypeID;
	}
}