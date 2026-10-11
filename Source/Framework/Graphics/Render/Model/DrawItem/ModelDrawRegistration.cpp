#include "ModelDrawRegistration.h"

// 1体のモデルの「描画項目の登録」をまとめて持つクラス
// 描画項目 = ルート定数(オブジェクト・メッシュ・マテリアルの番号) + ASのグループ数(Struct::ModelDrawItem)
// 描画項目は、マテリアルの種類ごとに、ModelRenderSystemの一覧(メッシュの種類 × マテリアルのテーブル)へ入れる
// 例 : Staticで、Body・FaceがStandardLit、EyeがStandardUnLitなら
//      「Static × StandardLit」の一覧に2項目、「Static × StandardUnLit」の一覧に1項目を登録する
// どの一覧に、どの登録番号で入れたかを覚えておき、破棄されるとき(デストラクタ)に自動で外す
// (参照が消えたら描画から自動で外れる。毎フレーム全項目をlockして確かめる必要はない)
FWK::Graphics::ModelDrawRegistration::ModelDrawRegistration() :
    m_drawItemListRegistrationList()
{}
FWK::Graphics::ModelDrawRegistration::~ModelDrawRegistration()
{
    Unregister();
}

void FWK::Graphics::ModelDrawRegistration::Register(const std::vector<std::vector<std::uint32_t>>& a_frameMeshIndexList,
                                                    const std::vector<std::uint32_t>&              a_meshletCountList,
                                                    const std::vector<ModelMaterial>&              a_materialList,
                                                    const Enum::ModelMeshType                      a_meshType,
                                                    const std::uint32_t                            a_objectIndex)
{
    // 登録し直すとき(マテリアルを変えたときなど)は、前の登録を先に外す
    // 外さないと、同じメッシュが古いマテリアルと新しいマテリアルで2回描かれる
    Unregister();

    // フレームごとのメッシュの番号が無ければ(モデルがまだ無い)、登録するものがない
    if (a_frameMeshIndexList.empty()) { return; }

    // メッシュの番号・Meshletの数・マテリアルは、どれも「メッシュの順番」で並んでいる必要がある
    // 例 : メッシュが3つなら、どのフレームのメッシュの番号も3つ、Meshletの数も3つ、マテリアルも3つ
    // 数が違うのは呼ぶ側の誤りなので、アサートで知らせる
    const bool l_isSameMeshCount = std::ranges::all_of(a_frameMeshIndexList,
                                                       [&a_materialList](const auto& a_meshIndexList)
                                                       {
                                                           return a_meshIndexList.size() == a_materialList.size();
                                                       });

    FWK_ASSERT_RETURN_IF(!l_isSameMeshCount,                                 "フレームごとのメッシュの数がマテリアルの数と違うため、描画項目の登録に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_meshletCountList.size() != a_materialList.size(), "Meshletの数の一覧がマテリアルの数と違うため、描画項目の登録に失敗しました。");

    // マテリアルの種類ごとに、フレームごとの描画項目の一覧を作る
    // 後で一覧をstd::moveで渡す(中身を移す)ので、constを付けずに値で受ける
    auto l_materialFrameDrawItemListMap = BuildMaterialFrameDrawItemListMap(a_frameMeshIndexList,
                                                                            a_meshletCountList,
                                                                            a_materialList,
                                                                            a_objectIndex);

    // マテリアルの種類ごとに、「メッシュの種類 × マテリアルのテーブル」の一覧へ登録する
    // 例 : StandardLit → 「Static × StandardLit」の一覧(StaticModelStandardLitPassと、Staticの影のパスが描く)
    for (auto& [l_materialTableStaticTypeID, l_frameDrawItemList] : l_materialFrameDrawItemListMap)
    {
        RegisterDrawItemList(a_meshType, l_materialTableStaticTypeID, std::move(l_frameDrawItemList));
    }
}

void FWK::Graphics::ModelDrawRegistration::Unregister()
{
    for (const auto& l_drawItemListRegistration : m_drawItemListRegistrationList)
    {
        // 一覧が先に破棄されている(アプリの終了時に、Rendererが先に消えた)なら、外す必要がない
        // weak_ptrで覚えているので、消えた一覧(壊れたメモリ)には触らない
        const auto& l_drawItemList = l_drawItemListRegistration.m_drawItemList.lock();

        if (!l_drawItemList) { continue; }

        l_drawItemList->Unregister(l_drawItemListRegistration.m_registrationID);
    }

    m_drawItemListRegistrationList.clear();
}

std::unordered_map<FWK::TypeAlias::StaticTypeID, std::vector<std::vector<FWK::Struct::ModelDrawItem>>> FWK::Graphics::ModelDrawRegistration::BuildMaterialFrameDrawItemListMap(const std::vector<std::vector<std::uint32_t>>& a_frameMeshIndexList,
                                                                                                                                                                              const std::vector<std::uint32_t>&              a_meshletCountList,
                                                                                                                                                                              const std::vector<ModelMaterial>&              a_materialList,
                                                                                                                                                                              const std::uint32_t                            a_objectIndex) const
{
    const auto& l_graphicsManager     = GraphicsManager::GetInstance               ();
    const auto& l_resourceContext     = l_graphicsManager.GetREFResourceContext    ();
    const auto& l_modelMaterialSystem = l_resourceContext.GetREFModelMaterialSystem();
    const auto& l_errorMaterial       = l_modelMaterialSystem.GetREFErrorMaterial  ();
    const auto& l_frameCount          = a_frameMeshIndexList.size                  ();

    // キー = マテリアルの種類(GPUデータの型のStaticTypeID)、値 = フレームごとの描画項目の一覧
    // 例 : フレーム3つ、Body(StandardLit)・Eye(StandardUnLit)なら
    //      StandardLit   → [ [Body(フレーム0)], [Body(フレーム1)], [Body(フレーム2)] ]
    //      StandardUnLit → [ [Eye (フレーム0)], [Eye (フレーム1)], [Eye (フレーム2)] ]
    std::unordered_map<TypeAlias::StaticTypeID, std::vector<std::vector<Struct::ModelDrawItem>>> l_materialFrameDrawItemListMap = {};

    for (std::size_t l_meshListIndex = 0ULL; l_meshListIndex < a_materialList.size(); ++l_meshListIndex)
    {
        // 1. このメッシュのマテリアルを、ハンドルから取り出す
        //    .matが読み込めていない(ファイルが無い・壊れている)ときはnullptrなので、エラーマテリアル(マゼンタ)にする
        const auto& l_modelMaterial = a_materialList[l_meshListIndex];
              auto  l_material      = l_modelMaterial.FetchVALMaterial();

        if (!l_material)
        {
            l_material = l_errorMaterial;
        }

        // エラーマテリアルも無い(作成に失敗している)ときは、このメッシュを描かない
        if (!l_material) { continue; }

        // 2. マテリアルの種類(GPUデータの型のStaticTypeID)と、テーブルの何番目かを取り出す
        //    種類は「どの一覧に入れるか」、番号は「PSがマテリアルのテーブルの何番目を読むか」に使う
        //    例 : StandardLitのテーブルの3番 → 「Static × StandardLit」の一覧へ、m_materialIndex = 3 で入れる
        const auto& l_tableINFO = l_material->FetchREFTableINFO();
        const auto* l_typeINFO  = l_tableINFO.k_typeINFO;

        // TypeINFOが無い(マクロの登録が壊れている)マテリアルは、どの一覧に入れるか決められないので、このメッシュを描かない
        if (!l_typeINFO) { continue; }

        const auto l_materialTableElementIndex = l_material->GetVALTableElementIndex();
        const auto l_materialTableStaticTypeID = l_typeINFO->k_staticTypeID;

        // 3. ASのグループ数(DispatchMeshのX)を、このメッシュのMeshletの数から求める
        //    メッシュごとに決まっていて変わらないので、ここで1回だけ計算して描画項目に入れておく
        const auto l_amplificationShaderGroupCount = CalculateAmplificationShaderGroupCount(a_meshletCountList[l_meshListIndex]);

        // 4. 初めて出てきたマテリアルの種類なら、フレームの数だけ空の一覧を用意する
        //    operator[]は、まだ無いキーなら空の値(フレームの一覧)を作ってから参照を返す
        //    ModelDrawItemList::Registerは「フレームの数が一覧と同じ」ことを求めるので、ここで数を合わせる
        auto& l_frameDrawItemList = l_materialFrameDrawItemListMap[l_materialTableStaticTypeID];

        if (l_frameDrawItemList.empty())
        {
            l_frameDrawItemList.resize(l_frameCount);
        }

        // 5. フレームごとに描画項目を作って入れる
        //    Skeletalは、スキニング後の頂点のバッファがフレームごとに別なので、メッシュの番号がフレームごとに違う
        //    Staticは、どのフレームにも同じメッシュの番号が渡される(呼ぶ側が同じ一覧をフレームの数だけ並べる)
        //    例 : フレーム3つ、Skeletalのメッシュ0 → [フレーム0 : メッシュの10番] [フレーム1 : 11番] [フレーム2 : 12番]
        for (std::size_t l_frameIndex = 0ULL; l_frameIndex < l_frameCount; ++l_frameIndex)
        {
            const auto& l_meshIndexList = a_frameMeshIndexList[l_frameIndex];

            Struct::ModelDrawItem l_drawItem = {};

            // シェーダーは、この3つの番号でオブジェクト・メッシュ・マテリアルのテーブルを引く(Model.hlsliのRCModelDrawItem)
            l_drawItem.m_rootConstant.m_objectIndex   = a_objectIndex;
            l_drawItem.m_rootConstant.m_meshIndex     = l_meshIndexList[l_meshListIndex];
            l_drawItem.m_rootConstant.m_materialIndex = l_materialTableElementIndex;

            // DispatchMeshの引数(ASのグループの数)。Y・Zは常に1
            l_drawItem.m_dispatchMeshArguments.ThreadGroupCountX = l_amplificationShaderGroupCount;
            l_drawItem.m_dispatchMeshArguments.ThreadGroupCountY = k_dispatchMeshThreadGroupCountY;
            l_drawItem.m_dispatchMeshArguments.ThreadGroupCountZ = k_dispatchMeshThreadGroupCountZ;

            l_frameDrawItemList[l_frameIndex].emplace_back(l_drawItem);
        }
    }

    return l_materialFrameDrawItemListMap;
}

void FWK::Graphics::ModelDrawRegistration::RegisterDrawItemList(const Enum::ModelMeshType a_meshType, const TypeAlias::StaticTypeID a_materialTableStaticTypeID, std::vector<std::vector<Struct::ModelDrawItem>>&& a_frameDrawItemList)
{
    // メッシュの種類 × マテリアルのテーブルの一覧を探す
    // 例 : Static × StandardLit の一覧(StaticModelStandardLitPassと、Staticの影のパスが描く)
    const auto& l_graphicsManager   = GraphicsManager::GetInstance           ();
    const auto& l_renderer          = l_graphicsManager.GetREFRenderer       ();
    const auto& l_modelRenderSystem = l_renderer.GetREFModelRenderSystem     ();
    const auto& l_drawItemListWeak  = l_modelRenderSystem.FindVALDrawItemList(a_meshType, a_materialTableStaticTypeID);
    const auto& l_drawItemList      = l_drawItemListWeak.lock                ();

    FWK_ASSERT_RETURN_IF(!l_drawItemList, "メッシュの種類とマテリアルに対応する描画項目の一覧が無いため、描画項目の登録に失敗しました。");

    // 一覧へ入れて、登録番号を受け取る(外すときに、この番号を渡す)
    DrawItemListRegistration l_drawItemListRegistration = {};

    l_drawItemListRegistration.m_drawItemList   = l_drawItemListWeak;
    l_drawItemListRegistration.m_registrationID = l_drawItemList->Register(std::move(a_frameDrawItemList));

    if (l_drawItemListRegistration.m_registrationID == ModelDrawItemList::k_invalidRegistrationID) { return; }

    m_drawItemListRegistrationList.emplace_back(std::move(l_drawItemListRegistration));
}

UINT FWK::Graphics::ModelDrawRegistration::CalculateAmplificationShaderGroupCount(const std::uint32_t a_meshletCount)
{
    // ASのグループ数 = ceil(Meshletの数 ÷ 32)
    // ASは1グループで32個のMeshletを調べる(k_meshletCountPerAmplificationShaderGroup。Model_AS.hlslと同じ値)
    // 割り切れないときは1グループ足す(余りのMeshletを担当するグループ)
    // 例 : 100 Meshletなら 100 ÷ 32 = 3 余り 4 なので 4 グループ / 64 Meshletなら 2 グループ
    auto l_amplificationShaderGroupCount = a_meshletCount / Constant::k_meshletCountPerAmplificationShaderGroup;

    if (a_meshletCount % Constant::k_meshletCountPerAmplificationShaderGroup != Constant::k_noRemainder)
    {
        ++l_amplificationShaderGroupCount;
    }

    return l_amplificationShaderGroupCount;
}