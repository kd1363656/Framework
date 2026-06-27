#pragma once

namespace FWK
{
	// テンプレートの明示的特殊化で初期化が完了する変数
	template <typename FactoryType, typename DerivedType>
	inline const bool k_isFactoryRegistered = false;
}

// 明示的特殊化を利用してファクトリーに登録したいクラスを自動登録できるようにするためのマクロ
// 登録に使用する際にはTypeINFORegistryに名前情報をあらかじめ定義しておく必要がある
#define FWK_REGISTER_FACTORY_METHOD(FactoryType, DerivedType)																							                           \
namespace FWK																																			                           \
{																																						                           \
	template <>																																			                           \
	inline const bool k_isFactoryRegistered<FactoryType, DerivedType> = []()																			                           \
	{																																					                           \
		const auto& l_typeINFO = DerivedType::GetREFTypeINFO();																							                           \
																																						                           \
		FactoryType::GetInstance().Register<DerivedType>(std::string(l_typeINFO.k_name));																                           \
		FWK_ADD_LOG				("[ファクトリー登録]\nName : {}\nStaticTypeID : {}\nファクトリーへの登録に成功しました。\n", l_typeINFO.k_name.data(), l_typeINFO.k_staticTypeID); \
																																						                           \
		return true;																																	                           \
	}();																																				                           \
}