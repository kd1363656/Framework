#pragma once

namespace FWK
{
    // テンプレートの明示的特殊化で初期化が完了する変数
    template <typename DerivedType>
    inline const bool k_isTaggedGameObjectComponentFactoryRegistered = false;
}

// 明示的特殊化を利用してファクトリーに登録したいクラスを自動登録できるようにするためのマクロ
// 登録に使用する際にはTypeINFORegistryに名前情報をあらかじめ定義しておく必要がある
#define FWK_REGISTER_TAGGED_GAME_OBJECT_COMPONENT_FACTORY(Tag, DerivedType)                                                                                                                       \
namespace FWK                                                                                                                                                                                     \
{                                                                                                                                                                                                 \
    template <>                                                                                                                                                                                   \
    inline const bool k_isTaggedGameObjectComponentFactoryRegistered<DerivedType> = []()                                                                                                          \
    {                                                                                                                                                                                             \
        TaggedGameObjectComponentFactory::GetInstance().Register<DerivedType>(Tag);                                                                                                               \
        FWK_ADD_LOG                                  (Constant::k_imguiDebugSuccessColor, "[タグ付け後コンポーネントファクトリー登録]\nName : {}\nファクトリーへの登録に成功しました。\n", #Tag); \
                                                                                                                                                                                                  \
        return true;                                                                                                                                                                              \
    }();                                                                                                                                                                                          \
}