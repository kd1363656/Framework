#pragma once

namespace FWK
{
	// テンプレートの明示的特殊化で初期化が完了する変数
	template <typename FactoryType, typename DerivedType>
	inline const bool k_isFactoryRegistered = false;
}

// 明示的特殊化を利用してファクトリーに登録したいクラスを自動登録できるようにするためのマクロ
// 登録に使用する際にはTypeInfoRegisterに名前情報をあらかじめ定義しておく必要がある
#define FWK_REGISTER_FACTORY_METHOD(FactoryType, DerivedType)												 \
namespace FWK																								 \
{																											 \
	template <>																								 \
	inline const bool k_isFactoryRegistered<FactoryType, DerivedType> = []()								 \
	{																										 \
		FactoryType::GetInstance().Register<DerivedType>(std::string(DerivedType::GetREFTypeINFO().k_name)); \
																											 \
		return true;																						 \
	}();																									 \
}