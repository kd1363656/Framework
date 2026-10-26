#pragma once

namespace FWK
{
    // テンプレートの明示的特殊化で初期化が完了する変数
    template <typename Type>
    inline const bool k_isTypeINFOTagRegistered = false;
}

// 明示的特殊化を利用してタグ名と型情報を自動で紐づけるためのマクロ
// 1つの型につき1回だけ書けるため、複数のタグに所属させたい場合はタグ名を続けて渡す
// 登録に使用する際にはFWK_DEFINE_TYPE_INFO系のマクロで型情報をあらかじめ定義しておく必要がある
// 例 : FWK_REGISTER_TYPE_INFO_TAG(CameraComponent, "Camera")
//      FWK_REGISTER_TYPE_INFO_TAG(DebugCameraComponent, "Camera", "Debug")
#define FWK_REGISTER_TYPE_INFO_TAG(Type, ...)                                                       \
namespace FWK                                                                                       \
{                                                                                                   \
    template <>                                                                                     \
    inline const bool k_isTypeINFOTagRegistered<Type> = []()                                        \
    {                                                                                               \
        for (const auto& l_tagName : std::initializer_list<std::string_view>{ __VA_ARGS__ })        \
        {                                                                                           \
            TypeINFOTagRegistry::GetInstance().Register(l_tagName, Type::GetREFTypeINFO());         \
        }                                                                                           \
                                                                                                    \
        return true;                                                                                \
    }();                                                                                            \
}
