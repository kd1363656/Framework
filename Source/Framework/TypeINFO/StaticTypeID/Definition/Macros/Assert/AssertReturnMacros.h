#pragma once

// do-whileを使う理由はマクロにセミコロンを使用したときにセミコロンが二重定義
// されることを防ぎ、できるだけコードの一貫性を守るため

#define FWK_ASSERT_RETURN(a_message) \
do									 \
{									 \
	assert(false && a_message);		 \
	return;							 \
}									 \
while (false)

#define FWK_ASSERT_RETURN_VALUE(a_message, a_returnValue) \
do													      \
{													      \
	assert(false && a_message);						      \
	return a_returnValue;							      \
}													      \
while (false)

#define FWK_ASSERT_RETURN_IF_FAILED(a_conditions, a_message) \
do															 \
{															 \
	if (a_conditions)								         \
	{												         \
		assert(false && a_message);					         \
		return;										         \
	}														 \
}															 \
while (false)

#define FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_conditions, a_message, a_returnValue) \
do																				  \
{																				  \
	if (a_conditions)														      \
	{																		      \
		assert(false && a_message);											      \
		return a_returnValue;												      \
	}																			  \
}																				  \
while (false)																	