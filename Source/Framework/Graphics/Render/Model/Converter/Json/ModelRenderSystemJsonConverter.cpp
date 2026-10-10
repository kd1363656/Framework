#include "ModelRenderSystemJsonConverter.h"

void FWK::Converter::ModelRenderSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::ModelRenderSystem& a_modelRenderSystem) const
{
    if (a_rootJson.is_null() ||
        !Utility::IsJsonArray(a_rootJson, k_tableMapJsonKey))
    {
        return; 
    }
    
    // 起動時(mainより前)に、マクロを書いたクラスが自分で登録した「型の名前 → テーブルの情報」のmap
    // 例 : "ModelObjectGPUData"(136バイト) / "ModelMeshGPUData"(24バイト) /
    //      "ModelStandardLitMaterialGPUData"(40バイト) / "ModelStandardUnLitMaterialGPUData"(20バイト)
    const auto& l_tableINFORegistry = Graphics::ModelRenderTableINFORegistry::GetInstance();
    const auto& l_tableINFONameMap  = l_tableINFORegistry.GetREFTableINFONameMap         ();

    // 1件 = { "Capacity" : 4096, "TypeName" : "ModelObjectGPUData" }
    // 名前はマクロの#Type(クラスの名前そのもの)と同じ文字列にする
    for (const auto& l_json : a_rootJson[k_tableMapJsonKey])
    {
        // 型の名前で、マクロが登録したテーブルの情報を探す
        // mapのキーが型の名前なので、全部を比べずに1回で見つかる
        // (StringHashとstd::equal_to<>にしてあるので、std::stringのまま探せる)
        const auto& l_typeName     = l_json.value           (k_typeNameJsonKey, std::string{});
        const auto& l_tableINFOITR = l_tableINFONameMap.find(l_typeName);

        FWK_ASSERT_RETURN_IF(l_tableINFOITR == l_tableINFONameMap.end(), "GraphicsCONFIG.jsonに登録されていない型の名前が書かれているため、ModelRenderSystemの読み込みに失敗しました。");
        FWK_ASSERT_RETURN_IF(!l_tableINFOITR->second,                    "テーブルの情報が無効のため、ModelRenderSystemの読み込みに失敗しました。");

        // 容量を省いたときは1024にする
        const auto& l_tableINFO = *l_tableINFOITR->second;
        const auto  l_capacity  = l_json.value(k_capacityJsonKey, k_defaultCapacity);

        a_modelRenderSystem.AddTable(l_tableINFO, l_capacity);
    }
}

nlohmann::json FWK::Converter::ModelRenderSystemJsonConverter::Serialize(const Graphics::ModelRenderSystem& a_modelRenderSystem) const
{
          nlohmann::json l_rootJson         = {};
          nlohmann::json l_tableJsonArray   = nlohmann::json::array             ();
    const auto&          l_typeINFORegistry = TypeINFORegistry::GetInstance     ();
    const auto&          l_tableMap         = a_modelRenderSystem.GetREFTableMap();

    // テーブルのmap(StaticTypeID → テーブル)を、{ 型の名前, 容量 } の組で書き出す
    // StaticTypeIDは起動のたびに変わりうる番号なので、ファイルには型の名前で残す
    // 型の名前は、マクロの中のFWK_DEFINE_TYPE_INFO_SINGLEがTypeINFORegistryへ登録したものを、StaticTypeIDで引く
    for (const auto& [l_tableStaticTypeID, l_table] : l_tableMap)
    {
        if (!l_table) { continue; }

        const auto* l_typeINFO = l_typeINFORegistry.FindPTRByID(l_tableStaticTypeID);

        if (!l_typeINFO) { continue; }

        nlohmann::json l_json = {};

        l_json[k_typeNameJsonKey] = l_typeINFO->k_name;
        l_json[k_capacityJsonKey] = l_table->GetVALCapacity();

        l_tableJsonArray.emplace_back(std::move(l_json));
    }

    l_rootJson[k_tableMapJsonKey] = std::move(l_tableJsonArray);

    return l_rootJson;
}