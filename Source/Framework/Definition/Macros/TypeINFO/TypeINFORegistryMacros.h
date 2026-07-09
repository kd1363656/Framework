#pragma once

// ※注意 : これらのマクロは型情報のゲッターと型情報登録クラスに自動で
// 登録する機能を提供するがクラスや構造体内部で書く必要がある
// マクロを書く際には絶対にクラスの一番下に書くこと
// 基底クラスを継承した派生クラスでテンプレートクラスにする必要があるなら
// テンプレートクラスにする前の基底クラスをテンプレートクラスの派生クラスでBaseTypeとして扱う
// 本来BaseTypeはそのクラスの継承する一つ前のクラス

// 型情報登録マクロ(基底)
#define FWK_DEFINE_TYPE_INFO_ROOT(Type)																							            \
public:																															            \
																																            \
	static const auto& GetREFTypeINFO()																							            \
	{																															            \
		static const auto l_typeINFO = FWK::TypeINFORegistry::TypeINFO{ nullptr, #Type, FWK::StaticTypeIDGenerator::GetVALTypeID<Type>() }; \
																																            \
		return l_typeINFO;																										            \
	}																															            \
																																            \
	virtual const FWK::TypeINFORegistry::TypeINFO& GetREFRuntimeTypeINFO() const { return GetREFTypeINFO(); }                               \
																																            \
private:																														            \
																																            \
	class RegisterTypeINFO																										            \
	{																															            \
	public:																														            \
																																            \
		RegisterTypeINFO()																										            \
		{																														            \
			FWK::TypeINFORegistry::GetInstance().Register(GetREFTypeINFO());                                                                \
		}																														            \
		~RegisterTypeINFO() = default;																							            \
	};																															            \
																																            \
	inline static const RegisterTypeINFO k_autoRegister = {};

// 型情報登録マクロ(派生)
#define FWK_DEFINE_TYPE_INFO(Type, BaseType)																										            \
public:																																				            \
																																					            \
	static const auto& GetREFTypeINFO()																												            \
	{																																				            \
		static const auto l_typeINFO = FWK::TypeINFORegistry::TypeINFO{ &BaseType::GetREFTypeINFO(), #Type, FWK::StaticTypeIDGenerator::GetVALTypeID<Type>() }; \
																																					            \
		return l_typeINFO;																															            \
	}																																				            \
																																					            \
	const FWK::TypeINFORegistry::TypeINFO& GetREFRuntimeTypeINFO() const override { return GetREFTypeINFO(); }												    \
																																					            \
private:																																			            \
																																					            \
	class RegisterTypeINFO																															            \
	{																																				            \
	public:																																			            \
																																					            \
		RegisterTypeINFO()																															            \
		{																																			            \
			FWK::TypeINFORegistry::GetInstance().Register(GetREFTypeINFO());																		            \
		}																																			            \
		~RegisterTypeINFO() = default;																												            \
	};																																				            \
																																					            \
	inline static const RegisterTypeINFO k_autoRegister = {};