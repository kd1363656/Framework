#pragma once

namespace FWK
{
	// テンプレートの明示的特殊化で初期化が完了する変数
	template <typename Type, typename RegisterValue>
	inline const bool k_isStringKeyRegistered = false;
}

#define FWK_REGISTER_STRING_KEY_REGISTRY(Type, RegisterValue)                           \
namespace FWK                                                                           \
{                                                                                       \
	template <>																			\
	inline const bool k_isStringKeyRegistered<Type, RegisterValue> = []()				\
	{																					\
																						\
		StringKeyRegistry<Type>::GetInstance().Register(RegisterValue, #RegisterValue);	\
																						\
		return true;																	\
	}();	                                                                            \
}