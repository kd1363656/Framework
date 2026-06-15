#pragma once

// Enumとその文字列について展開するマクロ
#define FWK_JSON_ENUM_VALUE(Value) { Value, #Value }

// NLOHMANN_JSON_SERIALIZE_ENUMを少ない記述量で呼び出すための補助マクロ
#define FWK_JSON_SERIALIZE_ENUM(EnumType, ...) \
NLOHMANN_JSON_SERIALIZE_ENUM					 \
(												 \
	EnumType,									 \
	{										     \
		__VA_ARGS__							     \
	}											 \
)