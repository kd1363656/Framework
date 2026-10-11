#include "ModelDrawItemList.h"

// 1つのパス(例 : StaticModelのLit)で描く「描画項目」の一覧
// 描画項目 = ルート定数(オブジェクト・メッシュ・マテリアルの番号) + ASのグループ数
// 登録・解除は、モデルやマテリアルが変わったときだけ起きる
// パスは、毎フレーム「詰め直した一覧」を先頭から回すだけにする
// 例 : 100体 × 5メッシュなら、一覧は500項目。登録が変わらない限り、詰め直しは起きない
FWK::Graphics::ModelDrawItemList::ModelDrawItemList() :
    m_frameDrawItemList(),

    m_registrationMap(),

    m_frameCount(k_emptyFrameCount),

    m_nextRegistrationID(k_firstRegistrationID),

    m_isDirty(false)
{}
FWK::Graphics::ModelDrawItemList::~ModelDrawItemList() = default;

bool FWK::Graphics::ModelDrawItemList::Create(const std::size_t& a_frameCount)
{
    FWK_ASSERT_RETURN_VALUE_IF(a_frameCount == k_emptyFrameCount, "フレーム数が0のため、ModelDrawItemListの作成に失敗しました。", false);

    // Skeletalは、フレームリソースごとにメッシュの要素(スキニング後の頂点のバッファ)が違うため、
    // 描画項目もフレームごとに持つ(Staticはどのフレームも同じ項目)
    m_frameCount = a_frameCount;

    m_frameDrawItemList.resize(a_frameCount);

    return true;
}

void FWK::Graphics::ModelDrawItemList::RecordDraw(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const std::size_t& a_frameIndex)
{
    // 登録・解除があったフレームだけ、一覧を詰め直す
    RebuildFrameDrawItemListIfNeeded();

    FWK_ASSERT_RETURN_IF(a_frameIndex >= m_frameDrawItemList.size(), "フレームの番号が範囲外のため、描画項目の描画に失敗しました。");

    const auto& l_drawItemList = m_frameDrawItemList[a_frameIndex];

    for (const auto& l_drawItem : l_drawItemList)
    {
        // この描画のオブジェクト・メッシュ・マテリアルの番号を、ルート定数(12バイト)で送る
        // シェーダーはこの番号でテーブルを引く(Model.hlsliのRCModelDrawItem)
        a_directCommandList.SetupRoot32BitConstants(l_drawItem.m_rootConstant, a_rootSignature, Enum::RootParameterType::RCModelDrawItem);

        // ASをメッシュのMeshletの数に合わせた数だけ起動する
        // 例 : 100 Meshletなら、1グループ32 Meshletなので X = 4、Y = 1、Z = 1
        const auto& l_dispatchMeshArguments = l_drawItem.m_dispatchMeshArguments;

        a_directCommandList.DispatchMesh(l_dispatchMeshArguments.ThreadGroupCountX, l_dispatchMeshArguments.ThreadGroupCountY, l_dispatchMeshArguments.ThreadGroupCountZ);
    }
}

std::uint64_t FWK::Graphics::ModelDrawItemList::Register(std::vector<std::vector<Struct::ModelDrawItem>>&& a_frameDrawItemList)
{
    FWK_ASSERT_RETURN_VALUE_IF(a_frameDrawItemList.size() != m_frameCount, "描画項目のフレームの数が一覧と違うため、描画項目の登録に失敗しました。", k_invalidRegistrationID);

    // 登録番号は増えていくだけの通し番号(64ビットなので、使い切る心配はない)
    // k_invalidRegistrationID(64ビットの最大値)は失敗を表す番号で、通し番号がそこまで届くことはない
    // 解除するときは、この番号を渡す
    const auto l_registrationID = m_nextRegistrationID;

    ++m_nextRegistrationID;

    m_registrationMap.try_emplace(l_registrationID, std::move(a_frameDrawItemList));

    // 次に描くときに、一覧を詰め直す
    m_isDirty = true;

    return l_registrationID;
}

void FWK::Graphics::ModelDrawItemList::Unregister(const std::uint64_t& a_registrationID)
{
    // 登録が無ければ(既に解除済みなど)何もしない
    // eraseは消した個数(0か1)を返す
    if (m_registrationMap.erase(a_registrationID) == k_noErasedCount) { return; }

    m_isDirty = true;
}

void FWK::Graphics::ModelDrawItemList::RebuildFrameDrawItemListIfNeeded()
{
    if (!m_isDirty) { return; }

    // 登録の一覧(登録番号ごとに分かれている)を、フレームごとに1本の配列へ詰め直す
    // パスはこの配列を回すだけなので、登録番号ごとのmapをたどるより速い
    // (S6のExecuteIndirectでは、この配列をそのままGPUのバッファへ写す)
    for (std::size_t l_frameIndex = 0ULL; l_frameIndex < m_frameCount; ++l_frameIndex)
    {
        auto& l_drawItemList = m_frameDrawItemList[l_frameIndex];

        // clearは要素を消すだけで、確保済みのメモリは残す(詰め直しのたびに確保し直さない)
        l_drawItemList.clear();

        for (const auto& [l_registrationID, l_frameDrawItemList] : m_registrationMap)
        {
            const auto& l_registeredDrawItemList = l_frameDrawItemList[l_frameIndex];

            l_drawItemList.insert(l_drawItemList.end(), l_registeredDrawItemList.begin(), l_registeredDrawItemList.end());
        }
    }

    m_isDirty = false;
}