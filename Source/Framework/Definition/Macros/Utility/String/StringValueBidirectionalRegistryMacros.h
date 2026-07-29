#pragma once

namespace FWK
{
	// テンプレートの明示的特殊化で初期化が完了する変数
	template <typename Type, Type RegisterValue>
	inline const bool k_isStringValueBidirectionalRegistered = false;
}

#define FWK_REGISTER_STRING_VALUE_BIDIRECTIONAL_REGISTRY(Type, RegisterValue)                                   \
namespace FWK                                                                                                   \
{                                                                                                               \
	template <>																			                        \
	inline const bool k_isStringValueBidirectionalRegistered<Type, RegisterValue> = []()			            \
	{																					                        \
																						                        \
		Utility::StringValueBidirectionalRegistry<Type>::GetInstance().Register(RegisterValue, #RegisterValue); \
																						                        \
		return true;																	                        \
	}();	                                                                                                    \
}