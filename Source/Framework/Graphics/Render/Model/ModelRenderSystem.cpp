#include "ModelRenderSystem.h"

// モデルの描画に使うテーブル(GPUElementTable)を、種類ごとにまとめて持つクラス
// 種類 : FWK_DEFINE_MODEL_RENDER_TABLE_INFO(またはMATERIAL版)を書いたクラス1つにつき、テーブル1つ
//        例 : ModelObjectGPUData(行列) / ModelMeshGPUData(バッファのSRVの番号) / ModelStandardLitMaterialGPUData(マテリアルの値)
// 種類はStaticTypeID(型ごとに起動時に1つ決まる番号)で区別するため、種類が増えてもこのクラスは書き換えない
// 使う側(ModelComponentの描き方・マテリアル)は、FindVALTableでテーブルをweak_ptrで受け取り、
// 番号をもらって、値が変わったときだけ書く
// シェーダーは、ルート定数で受け取った番号でテーブルを引いて値を読む
// ※ 注意 : RendererはModelRenderSystemをコマンドキューより前に宣言している(テーブルの解放をGPUの完了後にするため)
void FWK::Graphics::ModelRenderSystem::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::ModelRenderSystem::Create(const Device&                             a_device,
                                              const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
                                              const std::size_t&                        a_frameCount,
                                                    TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool) const
{
    // 起動時(mainより前)に、マクロを書いたクラスが自分で登録した「型の名前 → テーブルの情報」のmap
    // 例 : "ModelObjectGPUData"(136バイト) / "ModelMeshGPUData"(24バイト) /
    //      "ModelStandardLitMaterialGPUData"(40バイト) / "ModelStandardUnLitMaterialGPUData"(20バイト)
    const auto& l_tableINFORegistry = ModelRenderTableINFORegistry::GetInstance ();
    const auto& l_tableINFONameMap  = l_tableINFORegistry.GetREFTableINFONameMap();

    // Deserialize(GraphicsCONFIG.jsonのTableMap)でAddTableされたテーブルだけを、GPUのメモリに作る
    // CONFIGに書いていない種類のテーブルは作られない(RenderGraphPassListに書いたパスだけが作られるのと同じ考え方)
    // 容量と1要素の大きさは、AddTableでテーブルに持たせてある
    for (const auto& [l_tableStaticTypeID, l_table] : m_tableMap)
    {
        FWK_ASSERT_RETURN_VALUE_IF(!l_table, "テーブルが無効のため、ModelRenderSystemの作成に失敗しました。", false);

        // DEFAULTヒープの本体・フレーム数分のUPLOADヒープ・SRVを作る(S2)
        const bool l_isCreated = l_table->Create(a_device,
                                                 a_gpuMemoryAllocator,
                                                 a_frameCount,
                                                 a_cbvSRVUAVDescriptorPool);

        FWK_ASSERT_RETURN_VALUE_IF(!l_isCreated, "ModelRenderSystemのテーブルの作成に失敗しました。", false);
    }

    // AddTableで入れた描画項目の一覧を、フレームの数で作る
    // 1段目 = メッシュの種類(配列)、2段目 = マテリアルのテーブルのStaticTypeID(map)
    for (const auto& l_drawItemListMap : m_meshTypeDrawItemListMapList)
    {
        for (const auto& [l_materialTableStaticTypeID, l_drawItemList] : l_drawItemListMap)
        {
            FWK_ASSERT_RETURN_VALUE_IF(!l_drawItemList,                       "描画項目の一覧が無効のため、ModelRenderSystemの作成に失敗しました。",         false);
            FWK_ASSERT_RETURN_VALUE_IF(!l_drawItemList->Create(a_frameCount), "描画項目の一覧の作成に失敗したため、ModelRenderSystemの作成に失敗しました。", false);
        }
    }

    // オブジェクトとメッシュのテーブルは、すべてのモデルの描画で使う
    // CONFIGに書き忘れていたら、毎フレームの描画で気づくのではなく、起動時に作成を失敗にする
    FWK_ASSERT_RETURN_VALUE_IF(FindVALTable<ModelObjectGPUData>().expired(), "GraphicsCONFIG.jsonにModelObjectGPUDataが無いため、ModelRenderSystemの作成に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF(FindVALTable<ModelMeshGPUData>().expired(),   "GraphicsCONFIG.jsonにModelMeshGPUDataが無いため、ModelRenderSystemの作成に失敗しました。",   false);

    return true;
}

void FWK::Graphics::ModelRenderSystem::RecordUpload(const DirectCommandList& a_directCommandList, const std::size_t& a_frameIndex) const
{
    // すべてのテーブルについて、このフレームに書き換えた要素だけをGPUへコピーする命令を積む
    // 書き換えが無いテーブルは、何もしない
    for (const auto& [l_type, l_table] : m_tableMap)
    {
        if (!l_table) { continue; }

        l_table->RecordUpload(a_directCommandList, a_frameIndex);
    }
}

void FWK::Graphics::ModelRenderSystem::RecordDrawWithoutMaterial(const RootSignature&      a_rootSignature,
                                                                 const DirectCommandList&  a_directCommandList,
                                                                 const std::size_t&        a_frameIndex,
                                                                 const Enum::ModelMeshType a_meshType) const
{
    // メッシュの種類の、マテリアルごとの一覧のmapを探す(範囲外の種類ならnullptr)
    const auto* l_drawItemListMap = FindPTRDrawItemListMap(a_meshType);

    FWK_ASSERT_RETURN_IF(!l_drawItemListMap, "メッシュの種類が範囲外のため、モデルの描画に失敗しました。");

    // オブジェクトとメッシュのテーブルのSRVの番号を、パスの最初に1回だけ送る
    const auto& l_rcModelTable = FetchVALRCModelTable();

    a_directCommandList.SetupRoot32BitConstants(l_rcModelTable, a_rootSignature, Enum::RootParameterType::RCModelTable);

    // 影のパスのように、マテリアルを使わないパスは、このメッシュの種類の一覧を全部描く
    // 例 : Staticの影なら、Static × StandardLit と Static × StandardUnLit の両方を描く
    // マテリアルのテーブルの番号(RCModelMaterialTable)は送らない(影のルートシグネチャには無い)
    for (const auto& [l_materialTableStaticTypeID, l_drawItemList] : *l_drawItemListMap)
    {
        if (!l_drawItemList) { continue; }

        l_drawItemList->RecordDraw(a_rootSignature, a_directCommandList, a_frameIndex);
    }
}

nlohmann::json FWK::Graphics::ModelRenderSystem::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::ModelRenderSystem::AddTable(const Struct::ModelRenderTableINFO& a_tableINFO, const UINT a_capacity)
{
    FWK_ASSERT_RETURN_IF(!a_tableINFO.k_typeINFO, "TypeINFOが無効のため、テーブルの追加に失敗しました。");

    // 種類はStaticTypeIDで区別する(FindVALTableもこの番号で探す)
    // 同じ型の名前がCONFIGに2回書かれていると、片方が使われないテーブルになるので弾く
    const auto l_tableStaticTypeID = a_tableINFO.k_typeINFO->k_staticTypeID;

    FWK_ASSERT_RETURN_IF(m_tableMap.contains(l_tableStaticTypeID), "同じ種類のテーブルが追加済みのため、テーブルの追加に失敗しました。");

    // ここではGPUのメモリはまだ作らず、容量と1要素の大きさだけを持たせる(GPUのメモリはCreateで作る)
    // 1要素の大きさは、マクロがsizeof(型)で決めた値(HLSLのStructuredBufferの要素と同じ大きさ・同じ並び)
    auto l_table = std::make_shared<GPUElementTable>();

    l_table->SetCapacity         (a_capacity);
    l_table->SetElementByteStride(a_tableINFO.k_elementByteStride);

    m_tableMap.try_emplace(l_tableStaticTypeID, std::move(l_table));

    // マテリアルのテーブル(FWK_DEFINE_MODEL_MATERIAL_RENDER_TABLE_INFOを書いた型)なら、
    // メッシュの種類(Static / Skeletal)ごとに、描画項目の一覧を入れる
    // 例 : CONFIGにStandardLitとStandardUnLitが書かれていれば、
    //      Static × StandardLit / Static × StandardUnLit / Skeletal × StandardLit / Skeletal × StandardUnLit の4つ
    // テーブルと同じく、CONFIGに書いたマテリアルの分だけ入る(パスの種類のenumは要らない)
    // ここでは入れるだけで、一覧の中身(フレームの数だけの入れ物)はCreateで作る(テーブルと同じ流れ)
    if (!a_tableINFO.k_isMaterial) { return; }

    // 配列の添字がメッシュの種類(0 = Static / 1 = Skeletal)なので、配列を回せば全部の種類へ入れられる
    // それぞれのmapへ、マテリアルのテーブルのStaticTypeIDをキーにして一覧を入れる
    for (auto& l_drawItemListMap : m_meshTypeDrawItemListMapList)
    {
        l_drawItemListMap.try_emplace(l_tableStaticTypeID, std::make_shared<ModelDrawItemList>());
    }
}

FWK::Struct::RCModelTable FWK::Graphics::ModelRenderSystem::FetchVALRCModelTable() const
{
    // パスの最初に1回だけ送る、オブジェクトとメッシュのテーブルのSRVの番号
    // シェーダーはこの番号でテーブル(StructuredBuffer)を取り出し、描画ごとの番号で要素を読む
    // テーブルは要素の型で指定する(種類のenumを用意しなくてよい)
    Struct::RCModelTable l_rcModelTable = {};

    l_rcModelTable.m_objectTableSRVDescriptorIndex = FetchVALTableSRVDescriptorIndex<ModelObjectGPUData>();
    l_rcModelTable.m_meshTableSRVDescriptorIndex   = FetchVALTableSRVDescriptorIndex<ModelMeshGPUData>  ();

    return l_rcModelTable;
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::ModelRenderSystem::FetchVALTableSRVDescriptorIndex(const TypeAlias::StaticTypeID a_tableStaticTypeID) const
{
    const auto& l_tableITR = m_tableMap.find(a_tableStaticTypeID);

    FWK_ASSERT_RETURN_VALUE_IF(l_tableITR == m_tableMap.end(), "対応するテーブルが無いため、テーブルのSRVの番号の取得に失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);

    const auto& l_table = l_tableITR->second;

    FWK_ASSERT_RETURN_VALUE_IF(!l_table, "テーブルが無効のため、テーブルのSRVの番号の取得に失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);

    return l_table->GetVALSRVDescriptorIndex();
}

std::weak_ptr<FWK::Graphics::GPUElementTable> FWK::Graphics::ModelRenderSystem::FindVALTable(const TypeAlias::StaticTypeID a_tableStaticTypeID) const
{
    const auto& l_tableITR = m_tableMap.find(a_tableStaticTypeID);

    if (l_tableITR == m_tableMap.end()) { return {}; }

    return l_tableITR->second;
}

std::weak_ptr<FWK::Graphics::ModelDrawItemList> FWK::Graphics::ModelRenderSystem::FindVALDrawItemList(const Enum::ModelMeshType a_meshType, const TypeAlias::StaticTypeID a_materialTableStaticTypeID) const
{
    // 1. メッシュの種類(Static / Skeletal)で、マテリアルごとの一覧のmapを探す
    const auto* l_drawItemListMap = FindPTRDrawItemListMap(a_meshType);

    if (!l_drawItemListMap) { return {}; }

    // 2. そのmapの中で、マテリアルのテーブル(GPUデータの型のStaticTypeID)の一覧を探す
    const auto& l_drawItemListITR = l_drawItemListMap->find(a_materialTableStaticTypeID);

    if (l_drawItemListITR == l_drawItemListMap->end()) { return {}; }

    return l_drawItemListITR->second;
}

void FWK::Graphics::ModelRenderSystem::RecordMaterialDraw(const RootSignature&          a_rootSignature,
                                                          const DirectCommandList&      a_directCommandList,
                                                          const std::size_t&            a_frameIndex,
                                                          const Enum::ModelMeshType     a_meshType,
                                                          const TypeAlias::StaticTypeID a_materialTableStaticTypeID) const
{
    const auto& l_drawItemList = FindVALDrawItemList(a_meshType, a_materialTableStaticTypeID).lock();

    FWK_ASSERT_RETURN_IF(!l_drawItemList, "メッシュの種類とマテリアルに対応する描画項目の一覧が無いため、モデルの描画に失敗しました。");

    // オブジェクトとメッシュのテーブルのSRVの番号を、パスの最初に1回だけ送る
    // 描画ごとに変わらないので、描画項目のループの外で送る
    const auto& l_rcModelTable = FetchVALRCModelTable();

    a_directCommandList.SetupRoot32BitConstants(l_rcModelTable, a_rootSignature, Enum::RootParameterType::RCModelTable);

    // このパスが描くマテリアルのテーブルのSRVの番号も送る
    // PSは、描画ごとのマテリアルの番号(RCModelDrawItemのg_materialIndex)で、このテーブルを引く
    Struct::RCModelMaterialTable l_rcModelMaterialTable = {};

    l_rcModelMaterialTable.m_materialTableSRVDescriptorIndex = FetchVALTableSRVDescriptorIndex(a_materialTableStaticTypeID);

    a_directCommandList.SetupRoot32BitConstants(l_rcModelMaterialTable, a_rootSignature, Enum::RootParameterType::RCModelMaterialTable);

    l_drawItemList->RecordDraw(a_rootSignature, a_directCommandList, a_frameIndex);
}

const std::unordered_map<FWK::TypeAlias::StaticTypeID, std::shared_ptr<FWK::Graphics::ModelDrawItemList>>* FWK::Graphics::ModelRenderSystem::FindPTRDrawItemListMap(const Enum::ModelMeshType a_meshType) const
{
    // メッシュの種類(Static = 0 / Skeletal = 1)を、そのまま配列の添字に使う
    // Countなどの範囲外の値が渡されたときは、配列の外を読まないようにnullptrを返す
    const auto& l_meshTypeIndex = static_cast<std::size_t>(a_meshType);

    if (l_meshTypeIndex >= m_meshTypeDrawItemListMapList.size()) { return nullptr; }

    return &m_meshTypeDrawItemListMapList[l_meshTypeIndex];
}