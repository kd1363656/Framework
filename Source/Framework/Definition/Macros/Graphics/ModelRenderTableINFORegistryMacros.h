#pragma once

// GPUのテーブル(GPUElementTable)の1要素として送るクラスに書くマクロ
// ※注意 : TypeINFOのマクロと同じく、クラスの一番下に書くこと(最後にprivateへ戻すため)
// 書くだけで、次の3つが行われる
// 1. FWK_DEFINE_TYPE_INFO_SINGLE : 型の名前・StaticTypeIDをTypeINFORegistryへ登録する(仮想関数は増えないので、GPUへ送る並びは変わらない)
// 2. GetREFModelRenderTableINFO() : 上のTypeINFOへのポインタ・1要素の大きさ(sizeof)・マテリアルかどうかを返す
// 3. アプリの起動時(mainより前)に、ModelRenderTableINFORegistryへ自動で登録する
// ModelRenderSystemは登録された一覧からテーブルを作るため、種類を増やしてもModelRenderSystemは書き換えない
#define FWK_DEFINE_MODEL_RENDER_TABLE_INFO_BASE(Type, IsMaterial)                                                                                                                  \
    FWK_DEFINE_TYPE_INFO_SINGLE(Type)                                                                                                                                              \
                                                                                                                                                                                   \
public:                                                                                                                                                                            \
                                                                                                                                                                                   \
    static const auto& GetREFModelRenderTableINFO()                                                                                                                                \
    {                                                                                                                                                                              \
        static_assert(std::is_trivially_copyable_v<Type>, "テーブルの要素の型は、memcpyでGPUへ送るため、triviallyCopyableである必要があります。");                                        \
        static_assert(std::is_standard_layout_v<Type>,    "テーブルの要素の型は、HLSLの構造体と並びを合わせるため、standardLayoutである必要があります。");                                        \
                                                                                                                                                                                   \
        static const auto l_modelRenderTableINFO = FWK::Struct::ModelRenderTableINFO{ &GetREFTypeINFO(), static_cast<UINT>(sizeof(Type)), IsMaterial };                           \
                                                                                                                                                                                   \
        return l_modelRenderTableINFO;                                                                                                                                             \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    static constexpr bool k_isModelMaterialRenderTableElement = IsMaterial;                                                                                                        \
                                                                                                                                                                                   \
private:                                                                                                                                                                           \
                                                                                                                                                                                   \
    class RegisterModelRenderTableINFO                                                                                                                                             \
    {                                                                                                                                                                              \
    public:                                                                                                                                                                        \
                                                                                                                                                                                   \
        RegisterModelRenderTableINFO()                                                                                                                                             \
        {                                                                                                                                                                          \
            FWK::Graphics::ModelRenderTableINFORegistry::GetInstance().Register(GetREFModelRenderTableINFO());                                                                     \
        }                                                                                                                                                                          \
        ~RegisterModelRenderTableINFO() = default;                                                                                                                                 \
    };                                                                                                                                                                             \
                                                                                                                                                                                   \
    inline static const RegisterModelRenderTableINFO k_autoRegisterModelRenderTableINFO = {};

// オブジェクト・メッシュのように、マテリアルではないテーブルの要素に書く
#define FWK_DEFINE_MODEL_RENDER_TABLE_INFO(Type) FWK_DEFINE_MODEL_RENDER_TABLE_INFO_BASE(Type, false)

// マテリアルのテーブルの要素に書く
// マテリアルのテーブルは、ModelRenderSystemが「メッシュの種類(Static / Skeletal) × マテリアルの種類」ごとに描画項目の一覧も作る
#define FWK_DEFINE_MODEL_MATERIAL_RENDER_TABLE_INFO(Type) FWK_DEFINE_MODEL_RENDER_TABLE_INFO_BASE(Type, true)