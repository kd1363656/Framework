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

    // オブジェクトとメッシュのテーブルは、すべてのモデルの描画で使う
    // CONFIGに書き忘れていたら、毎フレームの描画で気づくのではなく、起動時に作成を失敗にする
    FWK_ASSERT_RETURN_VALUE_IF(FindVALTable<ModelObjectGPUData>().expired(), "GraphicsCONFIG.jsonにModelObjectGPUDataが無いため、ModelRenderSystemの作成に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF(FindVALTable<ModelMeshGPUData>().expired(),   "GraphicsCONFIG.jsonにModelMeshGPUDataが無いため、ModelRenderSystemの作成に失敗しました。",   false);

    return true;
}

void FWK::Graphics::ModelRenderSystem::RecordUpload(const CopyCommandList& a_copyCommandList, const std::size_t& a_frameIndex) const
{
    // すべてのテーブルについて、このフレームに書き換えた要素だけをGPUへコピーする命令を積む
    // 書き換えが無いテーブルは、何もしない
    for (const auto& [l_type, l_table] : m_tableMap)
    {
        if (!l_table) { continue; }

        l_table->RecordUpload(a_copyCommandList, a_frameIndex);
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