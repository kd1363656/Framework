#include "ModelRenderTableINFORegistry.h"

void FWK::Graphics::ModelRenderTableINFORegistry::Register(const Struct::ModelRenderTableINFO& a_tableINFO)
{
    FWK_ASSERT_RETURN_IF(!a_tableINFO.k_typeINFO,                                                                "TypeINFOが無効のため、テーブルの情報の登録に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_tableINFO.k_typeINFO->k_staticTypeID == StaticTypeIDGenerator::k_invalidStaticTypeID, "無効なStaticTypeIDを検出したため、テーブルの情報の登録に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_tableINFO.k_typeINFO->k_name.empty(),                                                 "型の名前が空のため、テーブルの情報の登録に失敗しました。");

    // 型の名前をキーにして登録する
    // GraphicsCONFIG.jsonの容量は型の名前で探すため、同じ名前が2つあると区別できない
    // (別のnamespaceに、同じ名前のクラスを作ったときなど)
    // try_emplaceは、同じキーが既にあれば何もせず、戻り値のsecondがfalseになる(重複の確認と登録が1回で済む)
    // 情報の実体は、マクロが作った関数の中のstatic変数(アプリの終了まで消えない)なので、ポインタで持ってよい
    // キーのstring_viewも、TypeINFOのk_name(static)を指すので消えない
    const bool l_isRegistered = m_tableINFONameMap.try_emplace(a_tableINFO.k_typeINFO->k_name, &a_tableINFO).second;

    FWK_ASSERT_RETURN_IF(!l_isRegistered, "同じ名前のテーブルの情報が登録済みのため、テーブルの情報の登録に失敗しました。");
    
    FWK_ADD_LOG(Constant::k_imguiDebugSuccessColor,
                "[テーブルの情報の登録]\nName : {}\nElementByteStride : {}\nテーブルの情報の登録に成功しました。\n",
                a_tableINFO.k_typeINFO->k_name.data(),
                a_tableINFO.k_elementByteStride);
}