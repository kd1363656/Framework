#pragma once

// Enumとその文字列について展開するマクロ
#define FWK_JSON_ENUM_VALUE(a_value) { a_value, #a_value }

// NLOHMANN_JSON_SERIALIZE_ENUMを少ない記述量で呼び出すための補助マクロ
#define FWK_JSON_SERIALIZE_ENUM(a_enumType, ...) \
NLOHMANN_JSON_SERIALIZE_ENUM					 \
(												 \
	a_enumType,									 \
	{										     \
		__VA_ARGS__							     \
	}											 \
)