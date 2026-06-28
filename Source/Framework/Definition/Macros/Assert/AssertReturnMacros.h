#pragma once

// do-whileを使う理由はマクロにセミコロンを使用したときにセミコロンが二重定義
// されることを防ぎ警告が出ないようにするため、できるだけコードの一貫性を守るため
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

#define FWK_ASSERT_RETURN_IF(Conditions, Message) \
do												  \
{												  \
	if (Conditions)								  \
	{											  \
		assert(false && Message);				  \
												  \
		return;									  \
	}											  \
}												  \
while (false)

#define FWK_ASSERT_RETURN_VALUE_IF(Conditions, Message, ReturnValue) \
do																	 \
{																	 \
	if (Conditions)													 \
	{																 \
		assert(false && Message);									 \
																	 \
		return ReturnValue;											 \
	}																 \
}																	 \
while (false)																	