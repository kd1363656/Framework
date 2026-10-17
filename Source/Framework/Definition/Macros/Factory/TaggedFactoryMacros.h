#pragma once

namespace FWK
{
    // テンプレートの明示的特殊化で初期化が完了する変数
    template <typename BaseType, typename DerivedType>
    inline const bool k_isTaggedFactoryRegistered = false;
}

// 明示的特殊化を利用して、タグ付きファクトリーに登録したいクラスを自動登録できるようにするためのマクロ
// 登録に使用する際にはTypeINFORegistryに名前情報をあらかじめ定義しておく必要がある
// 並び順はFramework.hでヘッダーを読み込んだ順(登録した順)になる
#define FWK_REGISTER_TAGGED_FACTORY(BaseType, DerivedType, Tag)                                                                                                                                 \
namespace FWK                                                                                                                                                                                   \
{                                                                                                                                                                                               \
    template <>                                                                                                                                                                                 \
    inline const bool k_isTaggedFactoryRegistered<BaseType, DerivedType> = []()                                                                                                                 \
    {                                                                                                                                                                                           \
              auto& l_factory  = TaggedFactory<BaseType>::GetInstance();                                                                                                                        \
        const auto& l_typeINFO = DerivedType::GetREFTypeINFO           ();                                                                                                                      \
                                                                                                                                                                                                \
        l_factory.Register<DerivedType>(Tag);                                                                                                                                                   \
        FWK_ADD_LOG                    (Constant::k_imguiDebugSuccessColor, "[タグ付きファクトリー登録]\nTag : {}\nName : {}\nファクトリーへの登録に成功しました。\n", Tag, l_typeINFO.k_name); \
                                                                                                                                                                                                \
        return true;                                                                                                                                                                            \
    }();                                                                                                                                                                                        \
}