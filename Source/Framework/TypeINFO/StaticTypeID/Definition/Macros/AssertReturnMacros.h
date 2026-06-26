#pragma once

// do-whileを使う理由はマクロにセミコロンを使用したときにセミコロンが二重定義
// されることを防ぐ、できるだけコードの一貫性を守るため
// (マクロが関数定義と間違われ警告が出る場合はセミコロンをつけない場合もある)
#define FWK_ASSERT_RETURN(Message) \
do								   \
{								   \
	assert(false && Message);	   \
								   \
	return;						   \
}								   \
while (false)

#define FWK_ASSERT_RETURN_VALUE(Message, ReturnValue) \
do													  \
{													  \
	assert(false && Message);						  \
													  \
	return ReturnValue;							      \
}													  \
while (false)

#define FWK_ASSERT_RETURN_IF_FAILED(Conditions, Message) \
do														 \
{														 \
	if (Conditions)								         \
	{												     \
		assert(false && Message);					     \
													     \
		return;										     \
	}													 \
}														 \
while (false)

#define FWK_ASSERT_RETURN_VALUE_IF_FAILED(Conditions, Message, ReturnValue) \
do																		    \
{																		    \
	if (Conditions)														    \
	{																	    \
		assert(false && Message);										    \
																		    \
		return ReturnValue;												    \
	}																		\
}																			\
while (false)																	