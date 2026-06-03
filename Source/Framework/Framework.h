#pragma once

//===============================================================================
// マクロ
//===============================================================================
#include "Definition/Macros/AssertReturnMacros.h"

//===============================================================================
// 継承しているかどうかを確認するコンセプト
//===============================================================================
#include "Definition/Concept/IsDerivedBase/IsDerivedBaseConcept.h"

//===============================================================================
// 文字列ハッシュ化構造体
//===============================================================================
#include "Definition/Struct/StringHash/StringHashStruct.h"

//===============================================================================
// シングルトンを安全に扱うための基底クラス
//===============================================================================
#include "Utility/Singleton/SingletonUtility.h"

//===============================================================================
// 型情報
//===============================================================================
#include "TypeINFO/Definition/Type/TypeINFOTypeAlias.h"
#include "TypeINFO/Definition/Constant/TypeINFOConstant.h"
#include "TypeINFO/Definition/Struct/TypeINFOStruct.h"
#include "TypeINFO/StaticTypeID/StaticTypeIDGenerator.h"
#include "TypeINFO/Registry/TypeINFORegistry.h"

// 型情報登録マクロ(通常版)
#include "TypeINFO/Definition/Macros/TypeINFOMacros.h"
#include "TypeINFO/Definition/Macros/TypeInfoTagMacros.h"

// タグ用TypeAlias
#include "TypeINFO/Definition/Tag/Type/TagTypeAlias.h"

// タグ基底構造体
#include "TypeINFO/Definition/Tag/TagBase.h"

// タグを継承しているかどうかのコンセプト
#include "TypeINFO/Definition/Tag/Concept/IsDerivedBase/IsDerivedTagBaseConcept.h"

// タグ用便利関数
#include "TypeINFO/Definition/Tag/Utility/TagUtility.h"

//===============================================================================
// 文字列変換便利関数
//===============================================================================
#include "Utility/String/Constant/StringUtilityConstant.h"
#include "Utility/String/StringUtility.h"

//===============================================================================
// ファイル読み込み便利関数
//===============================================================================
#include "Utility/File/Definition/Constant/FileUtilityConstant.h"
#include "Utility/File/FileUtility.h"

//===============================================================================
// json変便利関数
//===============================================================================
#include "Utility/Json/JsonUtility.h"

//===============================================================================
// ウィンドウクラス
//===============================================================================
#include "Window/Definition/Tag/WindowTag.h"
#include "Window/Definition/Constant/WindowConstant.h"
#include "Window/Definition/Struct/WindowStruct.h"
#include "Window/Converter/Json/WindowJsonConverter.h"
#include "Window/Window.h"