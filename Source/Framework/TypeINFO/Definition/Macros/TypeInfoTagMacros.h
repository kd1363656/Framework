#pragma once

// タグ情報登録マクロ(親なし)
#define FWK_DEFINE_TYPE_INFO_ROOT_TAG(Type)	\
public:										\
											\
	         Type() = default;				\
	virtual ~Type() = default;				\
											\
	FWK_DEFINE_TYPE_INFO_ROOT(Type)

// タグ情報登録マクロ(親あり)
#define FWK_DEFINE_TYPE_INFO_TAG(Type, BaseType) \
public:											 \
												 \
	 Type()          = default;					 \
	~Type() override = default;					 \
												 \
	FWK_DEFINE_TYPE_INFO(Type, BaseType)