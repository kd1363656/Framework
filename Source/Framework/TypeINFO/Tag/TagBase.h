#pragma once

// jsonに保存する際はEnumとNLOHMANN_JSON_SERIALIZE_ENUMを使えばいいが
// バイナリーファイルに保存する際にはEnumは適していない、そこでタグシステム
// Enumを使用した際の数値に変換する形式だと順序を入れ替えたらバグの原因になるが
// Tagシステムではそれが発生しない
namespace FWK::Tag
{
	struct TagBase { FWK_DEFINE_TYPE_INFO_ROOT_TAG(TagBase) };
}