#include "SkeletalAnimationSpringBoneSolver.h"

void FWK::Graphics::SkeletalAnimationSpringBoneSolver::Reset()
{
    m_isBuilt           = false;
    m_isTailInitialized = false;
    m_accumulatedSecond = k_initialAccumulatedSecond;
}

void FWK::Graphics::SkeletalAnimationSpringBoneSolver::Update(const SkeletalAnimationModelRecord::ModelData&     a_modelData,
                                                              const SkeletalAnimationModelSpringBoneSetting&     a_springBoneSetting,
                                                              const float                                        a_deltaTime,
                                                              const TypeAlias::Math::Matrix&                     a_worldMatrix,
                                                                    std::vector<TypeAlias::Math::Matrix>&        a_globalBoneMatrixList)
{
    const auto& l_boneList = a_modelData.m_boneList;

    FWK_ASSERT_RETURN_IF(a_globalBoneMatrixList.size() != l_boneList.size(), "GlobalBoneMatrixListの要素数がModelBoneListと一致しないため、揺れものを計算できません。");

    // エディターで設定が変わった(番号が変わった)ら、チェーンを作り直してPrewarmからやり直す
    if (!m_isBuilt ||
        m_builtRevision != a_springBoneSetting.GetVALRevision())
    {
        Build(a_modelData, a_springBoneSetting);
    }

    if (m_jointList.empty()) { return; }

    // モデルが拡大縮小されていても同じ見た目で揺れるように、当たり判定の半径などに掛ける
    const auto l_worldScale = Utility::CalculateWorldMaxScale(a_worldMatrix);

    UpdateAnimatedBoneMatrix(l_boneList,         a_worldMatrix, a_globalBoneMatrixList);
    UpdateWorldCollider     (a_springBoneSetting, l_worldScale);

    if (!m_isTailInitialized)
    {
        InitializeTail(l_boneList, a_worldMatrix);

        // Prewarm : 1フレーム目は「モデルを作った姿勢(Tポーズなど) → アニメーションの姿勢」へ一瞬で動いたことになり、大きく揺れてしまう
        // そのため、画面に出す前に同じ姿勢のまま何回か計算し、揺れが落ち着いた状態から始める
        const auto l_prewarmIterationCount = a_springBoneSetting.GetVALPrewarmIterationCount();

        for (std::uint32_t l_iterationIndex = 0U; l_iterationIndex < l_prewarmIterationCount; ++l_iterationIndex)
        {
            Step(l_boneList,
                 a_springBoneSetting,
                 a_worldMatrix,
                 l_worldScale);
        }

        m_accumulatedSecond = k_initialAccumulatedSecond;
        m_isTailInitialized = true;
    }

    // 経過時間を貯めて、1秒60回の決まった間隔で計算する
    // (フレームの時間がばらついても、計算1回分の時間は変わらないため揺れ方が安定する)
    m_accumulatedSecond += a_deltaTime;

    auto l_stepCount = k_initialStepCount;

    while (m_accumulatedSecond >= k_stepSecond &&
           l_stepCount         <  k_maxStepCountPerFrame)
    {
        Step(l_boneList,
             a_springBoneSetting,
             a_worldMatrix,
             l_worldScale);

        m_accumulatedSecond -= k_stepSecond;

        ++l_stepCount;
    }

    // 処理落ちで計算しきれなかった時間は捨てる(次のフレームに持ち越すと、計算がずっと追いつかなくなる)
    if (m_accumulatedSecond >= k_stepSecond)
    {
        m_accumulatedSecond = k_initialAccumulatedSecond;
    }

    WriteBack(l_boneList,
              a_springBoneSetting,
              a_worldMatrix,
              l_worldScale,
              a_globalBoneMatrixList);
}

void FWK::Graphics::SkeletalAnimationSpringBoneSolver::Build(const SkeletalAnimationModelRecord::ModelData& a_modelData, const SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting)
{
    const auto& l_boneList            = a_modelData.m_boneList;
    const auto& l_chainSettingList    = a_springBoneSetting.GetREFChainSettingList   ();
    const auto& l_colliderSettingList = a_springBoneSetting.GetREFColliderSettingList();

    m_jointList.clear   ();
    m_linkList.clear    ();
    m_colliderList.clear();

    m_boneJointIndexList.assign(l_boneList.size(), k_invalidIndex);

    // グループごとに、根元ごとのチェーン(ボーンの番号の列)を作る
    // 隣のチェーンとつなぐときに「同じ段のボーン」を探すため、チェーンの形のまま覚えておく
    std::vector<std::vector<std::vector<std::uint32_t>>> l_chainBoneIndexListPerSetting = {};

    l_chainBoneIndexListPerSetting.resize(l_chainSettingList.size());

    for (std::size_t l_chainSettingIndex = 0ULL; l_chainSettingIndex < l_chainSettingList.size(); ++l_chainSettingIndex)
    {
        const auto& l_chainSetting = l_chainSettingList[l_chainSettingIndex];

        if (!l_chainSetting.m_isEnabled) { continue; }

        auto& l_chainBoneIndexList = l_chainBoneIndexListPerSetting[l_chainSettingIndex];

        for (const auto& l_rootBoneName : l_chainSetting.m_rootBoneNameList)
        {
            const auto l_rootBoneIndex = FindVALBoneIndex(l_boneList, l_rootBoneName);

            // 名前のボーンが無い(別のモデルの設定、名前の打ち間違い)場合は、そのチェーンだけ揺らさない
            if (l_rootBoneIndex == k_invalidIndex) { continue; }

            std::vector<std::uint32_t> l_boneIndexList = {};

            BuildChain(l_boneList,
                       l_chainSetting,
                       static_cast<std::uint32_t>(l_chainSettingIndex),
                       l_rootBoneIndex,
                       l_boneIndexList);

            if (l_boneIndexList.empty()) { continue; }

            l_chainBoneIndexList.emplace_back(std::move(l_boneIndexList));
        }
    }

    // ボーンの番号順(親 → 子の順)に並べ替える
    // 別のグループのチェーンが、他のチェーンの子孫から始まっていても、必ず親から先に計算できるようにする
    std::ranges::sort(m_jointList, {}, &Joint::m_boneIndex);

    for (std::size_t l_jointIndex = 0ULL; l_jointIndex < m_jointList.size(); ++l_jointIndex)
    {
        m_boneJointIndexList[m_jointList[l_jointIndex].m_boneIndex] = static_cast<std::uint32_t>(l_jointIndex);
    }

    // 隣り合うチェーンをつなぐバネを作る(並べ替えた後のm_jointListの番号で覚える)
    for (std::size_t l_chainSettingIndex = 0ULL; l_chainSettingIndex < l_chainSettingList.size(); ++l_chainSettingIndex)
    {
        const auto& l_chainSetting = l_chainSettingList[l_chainSettingIndex];

        if (!l_chainSetting.m_isEnabled             ||
            !l_chainSetting.m_isAdjacentChainLinked)
        {
            continue;
        }

        BuildAdjacentLink(l_chainBoneIndexListPerSetting[l_chainSettingIndex], l_chainSetting, static_cast<std::uint32_t>(l_chainSettingIndex));
    }

    for (std::size_t l_colliderSettingIndex = 0ULL; l_colliderSettingIndex < l_colliderSettingList.size(); ++l_colliderSettingIndex)
    {
        const auto l_boneIndex = FindVALBoneIndex(l_boneList, l_colliderSettingList[l_colliderSettingIndex].m_boneName);

        if (l_boneIndex == k_invalidIndex) { continue; }

        Collider l_collider = {};

        l_collider.m_boneIndex            = l_boneIndex;
        l_collider.m_colliderSettingIndex = static_cast<std::uint32_t>(l_colliderSettingIndex);

        m_colliderList.emplace_back(l_collider);
    }

    // 揺らすボーンとその子孫は、書き戻すときに行列を計算し直す
    // ModelBoneListは親 → 子の順に並んでいるため、先頭から親の結果を見ていけば子孫まで伝わる
    m_boneAffectedList.assign(l_boneList.size(), false);

    for (std::size_t l_boneIndex = 0ULL; l_boneIndex < l_boneList.size(); ++l_boneIndex)
    {
        const auto l_parentBoneIndex = l_boneList[l_boneIndex].m_parentBoneIndex;
        const bool l_isParentAffected = l_parentBoneIndex != SkeletalAnimationModelRecord::k_invalidBoneIndex &&
                                        m_boneAffectedList[l_parentBoneIndex];

        m_boneAffectedList[l_boneIndex] = m_boneJointIndexList[l_boneIndex] != k_invalidIndex ||
                                          l_isParentAffected;
    }

    m_worldColliderList.resize(m_colliderList.size());

    m_builtRevision     = a_springBoneSetting.GetVALRevision();
    m_isBuilt           = true;
    m_isTailInitialized = false;
}

void FWK::Graphics::SkeletalAnimationSpringBoneSolver::BuildChain(const std::vector<SkeletalAnimationModelRecord::ModelBone>& a_boneList,
                                                                  const Struct::ModelSpringBoneChainSetting&                  a_chainSetting,
                                                                  const std::uint32_t                                         a_chainSettingIndex,
                                                                  const std::uint32_t                                         a_rootBoneIndex,
                                                                        std::vector<std::uint32_t>&                           a_chainBoneIndexList)
{
    auto l_boneIndex = a_rootBoneIndex;

    // 根元から「最初の子ボーン」を順にたどる
    while (l_boneIndex != k_invalidIndex)
    {
        // 既に別のチェーンで揺らしているボーンは、2回揺らさない
        if (m_boneJointIndexList[l_boneIndex] != k_invalidIndex) { return; }

        const auto  l_childBoneIndex = FindVALFirstChildBoneIndex(a_boneList, l_boneIndex);
        const auto& l_bone           = a_boneList[l_boneIndex];

        Joint l_joint = {};

        l_joint.m_boneIndex         = l_boneIndex;
        l_joint.m_chainSettingIndex = a_chainSettingIndex;

        if (l_childBoneIndex != k_invalidIndex)
        {
            // 先端 = 子ボーンの位置(子ボーンのBindPoseLocalMatrixの移動成分が、このボーンから見た子ボーンの位置)
            l_joint.m_localTailPosition = a_boneList[l_childBoneIndex].m_bindPoseLocalMatrix.Translation();
        }
        else if (a_chainSetting.m_endLength > k_initialLength)
        {
            // 末端のボーンには子ボーンが無いため、親 → このボーン の向きへm_endLengthだけ伸ばした位置を先端にする
            const auto& l_parentPositionInBone = TypeAlias::Math::Vector3::Transform(TypeAlias::Math::Vector3::Zero, l_bone.m_bindPoseLocalMatrix.Invert());

            auto l_endDirection = -l_parentPositionInBone;

            if (l_bone.m_parentBoneIndex == SkeletalAnimationModelRecord::k_invalidBoneIndex ||
                l_endDirection.Length()  <= k_minLength)
            {
                l_endDirection = k_defaultEndDirection;
            }

            l_endDirection.Normalize();

            l_joint.m_localTailPosition = l_endDirection * a_chainSetting.m_endLength;
        }
        else
        {
            // 先端が無いボーンは向きが決まらないため揺らさない
            return;
        }

        m_boneJointIndexList[l_boneIndex] = static_cast<std::uint32_t>(m_jointList.size());

        m_jointList.emplace_back(l_joint);
        a_chainBoneIndexList.emplace_back(l_boneIndex);

        l_boneIndex = l_childBoneIndex;
    }
}
void FWK::Graphics::SkeletalAnimationSpringBoneSolver::BuildAdjacentLink(const std::vector<std::vector<std::uint32_t>>& a_chainBoneIndexList, const Struct::ModelSpringBoneChainSetting& a_chainSetting, const std::uint32_t a_chainSettingIndex)
{
    const auto l_chainCount = a_chainBoneIndexList.size();

    for (std::size_t l_chainIndex = 0ULL; l_chainIndex < l_chainCount; ++l_chainIndex)
    {
        auto l_nextChainIndex = l_chainIndex + k_nextChainOffset;

        // 最後のチェーンは、一周つなぐ設定なら最初のチェーンとつなぐ(スカートのように輪になっている物)
        // 2本しか無いときに一周つなぐと、同じ2本を2回つなぐことになるため3本以上のときだけにする
        if (l_nextChainIndex >= l_chainCount)
        {
            if (!a_chainSetting.m_isAdjacentChainLooped ||
                l_chainCount < k_minLoopedChainCount)
            {
                break;
            }

            l_nextChainIndex = k_firstChainIndex;
        }

        const auto& l_chainA = a_chainBoneIndexList[l_chainIndex];
        const auto& l_chainB = a_chainBoneIndexList[l_nextChainIndex];

        // 長さが違うチェーン同士は、短い方の段までつなぐ
        const auto l_depthCount = std::min(l_chainA.size(), l_chainB.size());

        for (std::size_t l_depthIndex = 0ULL; l_depthIndex < l_depthCount; ++l_depthIndex)
        {
            Link l_link = {};

            l_link.m_jointIndexA       = m_boneJointIndexList[l_chainA[l_depthIndex]];
            l_link.m_jointIndexB       = m_boneJointIndexList[l_chainB[l_depthIndex]];
            l_link.m_chainSettingIndex = a_chainSettingIndex;

            m_linkList.emplace_back(l_link);
        }
    }
}

void FWK::Graphics::SkeletalAnimationSpringBoneSolver::UpdateAnimatedBoneMatrix(const std::vector<SkeletalAnimationModelRecord::ModelBone>& a_boneList, const TypeAlias::Math::Matrix& a_worldMatrix, const std::vector<TypeAlias::Math::Matrix>& a_globalBoneMatrixList)
{
    m_animatedLocalBoneMatrixList.resize(a_boneList.size());
    m_worldBoneMatrixList.resize        (a_boneList.size());

    for (std::size_t l_boneIndex = 0ULL; l_boneIndex < a_boneList.size(); ++l_boneIndex)
    {
        const auto& l_globalBoneMatrix = a_globalBoneMatrixList[l_boneIndex];
        const auto  l_parentBoneIndex  = a_boneList[l_boneIndex].m_parentBoneIndex;

        // アニメーションのワールド座標の姿勢(揺らさないボーン、当たり判定はこのまま使う)
        m_worldBoneMatrixList[l_boneIndex] = l_globalBoneMatrix * a_worldMatrix;

        // 揺らすボーンとその子孫は、親が揺れた後の姿勢から計算し直すため、親から見た姿勢(LocalMatrix)を覚えておく
        // 行ベクトルの方式では Global = Local × ParentGlobal なので Local = Global × ParentGlobalの逆行列
        if (!m_boneAffectedList[l_boneIndex]) { continue; }

        if (l_parentBoneIndex == SkeletalAnimationModelRecord::k_invalidBoneIndex)
        {
            m_animatedLocalBoneMatrixList[l_boneIndex] = l_globalBoneMatrix;

            continue;
        }

        m_animatedLocalBoneMatrixList[l_boneIndex] = l_globalBoneMatrix * a_globalBoneMatrixList[l_parentBoneIndex].Invert();
    }
}
void FWK::Graphics::SkeletalAnimationSpringBoneSolver::UpdateWorldCollider(const SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting, const float a_worldScale)
{
    const auto& l_colliderSettingList = a_springBoneSetting.GetREFColliderSettingList();

    for (std::size_t l_colliderIndex = 0ULL; l_colliderIndex < m_colliderList.size(); ++l_colliderIndex)
    {
        const auto& l_collider         = m_colliderList[l_colliderIndex];
        const auto& l_colliderSetting  = l_colliderSettingList[l_collider.m_colliderSettingIndex];
        const auto& l_boneWorldMatrix  = m_worldBoneMatrixList[l_collider.m_boneIndex];

        auto& l_worldCollider = m_worldColliderList[l_colliderIndex];

        // ボーンの座標系での位置を、ワールド座標へ変える
        l_worldCollider.m_head   = TypeAlias::Math::Vector3::Transform(l_colliderSetting.m_offset, l_boneWorldMatrix);
        l_worldCollider.m_tail   = l_worldCollider.m_head;
        l_worldCollider.m_radius = l_colliderSetting.m_radius * a_worldScale;

        if (l_colliderSetting.m_shape == Enum::ModelSpringBoneColliderShape::Capsule)
        {
            l_worldCollider.m_tail = TypeAlias::Math::Vector3::Transform(l_colliderSetting.m_tailOffset, l_boneWorldMatrix);
        }
    }
}

void FWK::Graphics::SkeletalAnimationSpringBoneSolver::InitializeTail(const std::vector<SkeletalAnimationModelRecord::ModelBone>& a_boneList, const TypeAlias::Math::Matrix& a_worldMatrix)
{
    // 先端をアニメーションの姿勢の位置に置く(前の位置も同じにするので、動き出す勢いは0)
    for (auto& l_joint : m_jointList)
    {
        const auto l_parentBoneIndex = a_boneList[l_joint.m_boneIndex].m_parentBoneIndex;

        const auto& l_parentWorldMatrix = l_parentBoneIndex == SkeletalAnimationModelRecord::k_invalidBoneIndex ? a_worldMatrix : m_worldBoneMatrixList[l_parentBoneIndex];
        const auto& l_jointWorldMatrix  = m_animatedLocalBoneMatrixList[l_joint.m_boneIndex] * l_parentWorldMatrix;

        l_joint.m_currentTailPosition  = TypeAlias::Math::Vector3::Transform(l_joint.m_localTailPosition, l_jointWorldMatrix);
        l_joint.m_previousTailPosition = l_joint.m_currentTailPosition;
        l_joint.m_linkOffset           = TypeAlias::Math::Vector3::Zero;
    }

    // 隣のチェーンとの、元の(縫い合わせたい)距離を覚える
    for (auto& l_link : m_linkList)
    {
        const auto& l_jointA = m_jointList[l_link.m_jointIndexA];
        const auto& l_jointB = m_jointList[l_link.m_jointIndexB];

        l_link.m_restLength = TypeAlias::Math::Vector3::Distance(l_jointA.m_currentTailPosition, l_jointB.m_currentTailPosition);
    }
}

void FWK::Graphics::SkeletalAnimationSpringBoneSolver::Step(const std::vector<SkeletalAnimationModelRecord::ModelBone>& a_boneList,
                                                            const SkeletalAnimationModelSpringBoneSetting&               a_springBoneSetting,
                                                            const TypeAlias::Math::Matrix&                               a_worldMatrix,
                                                            const float                                                  a_worldScale)
{
    UpdateLinkOffset(a_springBoneSetting);

    // m_jointListは親 → 子の順に並んでいるため、親が揺れた後の姿勢を使って子を計算できる
    for (auto& l_joint : m_jointList)
    {
        SolveJoint(a_boneList,
                   a_springBoneSetting,
                   a_worldMatrix,
                   a_worldScale,
                   true,
                   l_joint);
    }
}

void FWK::Graphics::SkeletalAnimationSpringBoneSolver::UpdateLinkOffset(const SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting)
{
    const auto& l_chainSettingList = a_springBoneSetting.GetREFChainSettingList();

    for (auto& l_joint : m_jointList)
    {
        l_joint.m_linkOffset = TypeAlias::Math::Vector3::Zero;
    }

    // COLOPL SpringBone第2回の式の第3項 k・(1/2)・(ΔL/M)・Δt²
    //   ΔL : 今の距離と元の距離の差(離れすぎたら近づき、近すぎたら離れる)
    //   k  : バネの強さ(エディターで0～1500まで変えられる。500～800くらいが丁度よい)
    // 2つの先端を、同じ量だけ反対向きに動かす
    const auto l_stepSecondSquared = k_stepSecond * k_stepSecond;

    for (const auto& l_link : m_linkList)
    {
        auto& l_jointA = m_jointList[l_link.m_jointIndexA];
        auto& l_jointB = m_jointList[l_link.m_jointIndexB];

        auto       l_direction = l_jointB.m_currentTailPosition - l_jointA.m_currentTailPosition;
        const auto l_distance  = l_direction.Length();

        if (l_distance <= k_minLength) { continue; }

        l_direction /= l_distance;

        const auto l_stiffness    = l_chainSettingList[l_link.m_chainSettingIndex].m_adjacentLinkStiffness;
        const auto l_lengthDelta  = l_distance - l_link.m_restLength;
        const auto l_offsetLength = l_stiffness * k_linkSpringHalfRatio * (l_lengthDelta / k_linkMass) * l_stepSecondSquared;

        l_jointA.m_linkOffset += l_direction * l_offsetLength;
        l_jointB.m_linkOffset -= l_direction * l_offsetLength;
    }
}

void FWK::Graphics::SkeletalAnimationSpringBoneSolver::SolveJoint(const std::vector<SkeletalAnimationModelRecord::ModelBone>& a_boneList,
                                                                  const SkeletalAnimationModelSpringBoneSetting&               a_springBoneSetting,
                                                                  const TypeAlias::Math::Matrix&                               a_worldMatrix,
                                                                  const float                                                  a_worldScale,
                                                                  const bool                                                   a_isTailIntegrated,
                                                                        Joint&                                                 a_joint)
{
    const auto  l_boneIndex       = a_joint.m_boneIndex;
    const auto  l_parentBoneIndex = a_boneList[l_boneIndex].m_parentBoneIndex;
    const auto& l_chainSetting    = a_springBoneSetting.GetREFChainSettingList()[a_joint.m_chainSettingIndex];

    // 親が揺れた後の姿勢に、アニメーションの(親から見た)姿勢を重ねる = 揺らす前のこのボーンの姿勢
    const auto& l_parentWorldMatrix = l_parentBoneIndex == SkeletalAnimationModelRecord::k_invalidBoneIndex ? a_worldMatrix : m_worldBoneMatrixList[l_parentBoneIndex];
    const auto& l_jointWorldMatrix  = m_animatedLocalBoneMatrixList[l_boneIndex] * l_parentWorldMatrix;

    const auto& l_headPosition         = l_jointWorldMatrix.Translation();
    const auto& l_animatedTailPosition = TypeAlias::Math::Vector3::Transform(a_joint.m_localTailPosition, l_jointWorldMatrix);

    auto       l_restDirection = l_animatedTailPosition - l_headPosition;
    const auto l_length        = l_restDirection.Length();

    if (l_length <= k_minLength)
    {
        m_worldBoneMatrixList[l_boneIndex] = l_jointWorldMatrix;

        return;
    }

    l_restDirection /= l_length;

    auto l_tailPosition = a_joint.m_currentTailPosition;

    if (a_isTailIntegrated)
    {
        // 1. ベレの方法で先端を動かす
        //    慣性   : 前回からの移動量(今の位置 - 前の位置)を、空気抵抗(Drag)の分だけ弱めて足す
        //    戻る力 : 本来の向きへ戻ろうとする力(Stiffness)。ボーンの長さに比例させ、大きさが違うモデルでも同じ値で揺れるようにする
        //    重力   : 指定した向きへ引っ張る力
        //    つなぎ : 隣のチェーンとの距離を保つバネ(UpdateLinkOffsetで計算済み)
        auto l_gravityDirection = l_chainSetting.m_gravityDirection;

        if (l_gravityDirection.Length() > k_minLength) { l_gravityDirection.Normalize(); }

        const auto& l_inertia = (a_joint.m_currentTailPosition - a_joint.m_previousTailPosition) * (k_noDragRatio - l_chainSetting.m_drag);
        const auto& l_restore = l_restDirection    * (l_chainSetting.m_stiffness    * l_length * k_stepSecond);
        const auto& l_gravity = l_gravityDirection * (l_chainSetting.m_gravityPower * l_length * k_stepSecond);

        auto l_nextTailPosition = a_joint.m_currentTailPosition + l_inertia + l_restore + l_gravity + a_joint.m_linkOffset;

        // 2. 棒の長さを元に戻す(先端は根元からl_lengthの球面の上にしか行けない)
        auto l_nextDirection = l_nextTailPosition - l_headPosition;

        if (l_nextDirection.Length() <= k_minLength) { l_nextDirection = l_restDirection; }

        l_nextDirection.Normalize();

        l_nextTailPosition = l_headPosition + l_nextDirection * l_length;

        // 3. 体(当たり判定)にめり込んでいたら押し出し、押し出した後でもう一度長さを戻す
        l_nextTailPosition = PushOutFromCollider(l_nextTailPosition, l_chainSetting.m_hitRadius * a_worldScale);

        l_nextDirection = l_nextTailPosition - l_headPosition;

        if (l_nextDirection.Length() <= k_minLength) { l_nextDirection = l_restDirection; }

        l_nextDirection.Normalize();

        l_nextTailPosition = l_headPosition + l_nextDirection * l_length;

        // 4. 角速度の上限(COLOPL SpringBone第1回) : 1回で回ってよい角度を超えた分は回さない
        const auto l_maxAngleRadian = DirectX::XMConvertToRadians(l_chainSetting.m_maxAngularVelocityDegree) * k_stepSecond;

        l_nextTailPosition = LimitAngularVelocity(l_headPosition,
                                                  a_joint.m_currentTailPosition,
                                                  l_nextTailPosition,
                                                  l_maxAngleRadian);

        a_joint.m_previousTailPosition = a_joint.m_currentTailPosition;
        a_joint.m_currentTailPosition  = l_nextTailPosition;

        l_tailPosition = l_nextTailPosition;
    }
    else
    {
        // 計算と計算の間のフレームでは、前回と今回の先端の位置を、貯まっている時間の割合で混ぜて滑らかに見せる
        const auto l_interpolationRatio = std::clamp(m_accumulatedSecond / k_stepSecond, k_minInterpolationRatio, k_maxInterpolationRatio);

        l_tailPosition = TypeAlias::Math::Vector3::Lerp(a_joint.m_previousTailPosition, a_joint.m_currentTailPosition, l_interpolationRatio);
    }

    auto l_tailDirection = l_tailPosition - l_headPosition;

    if (l_tailDirection.Length() <= k_minLength)
    {
        m_worldBoneMatrixList[l_boneIndex] = l_jointWorldMatrix;

        return;
    }

    l_tailDirection.Normalize();

    // 5. 本来の向き → 先端の向き へ回す回転を、根元を中心にボーンへ掛ける
    //    行ベクトルの方式なので、ボーンの行列 × (根元を原点へ移す × 回す × 根元へ戻す)
    const auto& l_rotation = CreateFromToRotation(l_restDirection, l_tailDirection);

    m_worldBoneMatrixList[l_boneIndex] = l_jointWorldMatrix                                           *
                                         TypeAlias::Math::Matrix::CreateTranslation(-l_headPosition) *
                                         TypeAlias::Math::Matrix::CreateFromQuaternion(l_rotation)    *
                                         TypeAlias::Math::Matrix::CreateTranslation(l_headPosition);
}

void FWK::Graphics::SkeletalAnimationSpringBoneSolver::WriteBack(const std::vector<SkeletalAnimationModelRecord::ModelBone>& a_boneList,
                                                                 const SkeletalAnimationModelSpringBoneSetting&               a_springBoneSetting,
                                                                 const TypeAlias::Math::Matrix&                               a_worldMatrix,
                                                                 const float                                                  a_worldScale,
                                                                       std::vector<TypeAlias::Math::Matrix>&                  a_globalBoneMatrixList)
{
    const auto& l_inverseWorldMatrix = a_worldMatrix.Invert();

    // 親 → 子の順に、揺らすボーンは今の先端へ向け直し、その子孫は親から見た姿勢を親の結果へ重ね直す
    // (計算をしなかったフレームでも、キャラクターが動いた分だけ根元は動くため、毎フレーム向け直す)
    for (std::size_t l_boneIndex = 0ULL; l_boneIndex < a_boneList.size(); ++l_boneIndex)
    {
        if (!m_boneAffectedList[l_boneIndex]) { continue; }

        if (const auto l_jointIndex = m_boneJointIndexList[l_boneIndex];
            l_jointIndex != k_invalidIndex)
        {
            SolveJoint(a_boneList,
                       a_springBoneSetting,
                       a_worldMatrix,
                       a_worldScale,
                       false,
                       m_jointList[l_jointIndex]);
        }
        else
        {
            const auto l_parentBoneIndex = a_boneList[l_boneIndex].m_parentBoneIndex;

            const auto& l_parentWorldMatrix = l_parentBoneIndex == SkeletalAnimationModelRecord::k_invalidBoneIndex ? a_worldMatrix : m_worldBoneMatrixList[l_parentBoneIndex];

            m_worldBoneMatrixList[l_boneIndex] = m_animatedLocalBoneMatrixList[l_boneIndex] * l_parentWorldMatrix;
        }

        // ワールド座標 → モデルの座標へ戻す(GPUへ送るボーンの行列はモデルの座標)
        a_globalBoneMatrixList[l_boneIndex] = m_worldBoneMatrixList[l_boneIndex] * l_inverseWorldMatrix;
    }
}

FWK::TypeAlias::Math::Vector3 FWK::Graphics::SkeletalAnimationSpringBoneSolver::PushOutFromCollider(const TypeAlias::Math::Vector3& a_tailPosition, const float a_hitRadius) const
{
    auto l_tailPosition = a_tailPosition;

    for (const auto& l_worldCollider : m_worldColliderList)
    {
        // カプセルの線分の上で、先端に一番近い点を求める(球は線分の長さが0のカプセル)
        const auto& l_segment       = l_worldCollider.m_tail - l_worldCollider.m_head;
        const auto  l_segmentLength = l_segment.LengthSquared();

        auto l_closestPosition = l_worldCollider.m_head;

        if (l_segmentLength > k_minLength)
        {
            const auto l_segmentRatio = std::clamp((l_tailPosition - l_worldCollider.m_head).Dot(l_segment) / l_segmentLength, k_minInterpolationRatio, k_maxInterpolationRatio);

            l_closestPosition = l_worldCollider.m_head + l_segment * l_segmentRatio;
        }

        // 一番近い点からの距離が「当たり判定の半径 + 揺れもの側の半径」より短ければ、その距離まで押し出す
        auto       l_pushDirection = l_tailPosition - l_closestPosition;
        const auto l_distance      = l_pushDirection.Length();
        const auto l_minDistance   = l_worldCollider.m_radius + a_hitRadius;

        if (l_distance >= l_minDistance) { continue; }
        if (l_distance <= k_minLength)   { continue; }

        l_pushDirection /= l_distance;

        l_tailPosition = l_closestPosition + l_pushDirection * l_minDistance;
    }

    return l_tailPosition;
}
FWK::TypeAlias::Math::Vector3 FWK::Graphics::SkeletalAnimationSpringBoneSolver::LimitAngularVelocity(const TypeAlias::Math::Vector3& a_headPosition,
                                                                                                    const TypeAlias::Math::Vector3& a_previousTailPosition,
                                                                                                    const TypeAlias::Math::Vector3& a_nextTailPosition,
                                                                                                    const float                     a_maxAngleRadian) const
{
    if (a_maxAngleRadian <= k_minAngleRadian) { return a_nextTailPosition; }

    auto l_previousDirection = a_previousTailPosition - a_headPosition;
    auto l_nextDirection     = a_nextTailPosition     - a_headPosition;

    const auto l_length = l_nextDirection.Length();

    if (l_previousDirection.Length() <= k_minLength ||
        l_length                     <= k_minLength)
    {
        return a_nextTailPosition;
    }

    l_previousDirection.Normalize();
    l_nextDirection.Normalize    ();

    // 前回の向きと今回の向きの間の角度(内積 = cosθ)
    const auto l_angleRadian = std::acos(std::clamp(l_previousDirection.Dot(l_nextDirection), k_minDotProduct, k_maxDotProduct));

    if (l_angleRadian <= a_maxAngleRadian) { return a_nextTailPosition; }

    // 上限を超えた場合は、前回の向きから上限の角度だけ回した向きにする
    auto l_rotationAxis = l_previousDirection.Cross(l_nextDirection);

    if (l_rotationAxis.Length() <= k_minLength) { return a_nextTailPosition; }

    l_rotationAxis.Normalize();

    const auto& l_limitedRotation  = TypeAlias::Math::Quaternion::CreateFromAxisAngle(l_rotationAxis, a_maxAngleRadian);
    const auto& l_limitedDirection = TypeAlias::Math::Vector3::Transform(l_previousDirection, l_limitedRotation);

    return a_headPosition + l_limitedDirection * l_length;
}

FWK::TypeAlias::Math::Quaternion FWK::Graphics::SkeletalAnimationSpringBoneSolver::CreateFromToRotation(const TypeAlias::Math::Vector3& a_fromDirection, const TypeAlias::Math::Vector3& a_toDirection) const
{
    // 2つの向き(長さ1)の間の回転を、回転軸 = 外積、回転量 = 内積 から作る
    // (外積の長さはsinθ、内積はcosθ。(外積, 1 + cosθ)を正規化すると、θだけ回すQuaternionになる)
    const auto l_dotProduct = a_fromDirection.Dot(a_toDirection);

    // ほぼ真逆の向きは外積が0になり回転軸が決まらないため、向きに垂直な軸で半回転させる
    if (l_dotProduct <= k_minDotProduct + k_minLength)
    {
        auto l_halfTurnAxis = a_fromDirection.Cross(k_halfTurnAxisCandidate);

        if (l_halfTurnAxis.Length() <= k_minLength) { l_halfTurnAxis = a_fromDirection.Cross(k_halfTurnAxisFallback); }

        l_halfTurnAxis.Normalize();

        return TypeAlias::Math::Quaternion{ l_halfTurnAxis, k_halfTurnQuaternionW };
    }

    const auto& l_crossProduct = a_fromDirection.Cross(a_toDirection);

    TypeAlias::Math::Quaternion l_rotation = { l_crossProduct, k_quaternionRealPartOffset + l_dotProduct };

    l_rotation.Normalize();

    return l_rotation;
}

std::uint32_t FWK::Graphics::SkeletalAnimationSpringBoneSolver::FindVALBoneIndex(const std::vector<SkeletalAnimationModelRecord::ModelBone>& a_boneList, const std::wstring& a_boneName) const
{
    const auto& l_itr = std::ranges::find(a_boneList, a_boneName, &SkeletalAnimationModelRecord::ModelBone::m_boneName);

    if (l_itr == a_boneList.end()) { return k_invalidIndex; }

    return static_cast<std::uint32_t>(std::distance(a_boneList.begin(), l_itr));
}
std::uint32_t FWK::Graphics::SkeletalAnimationSpringBoneSolver::FindVALFirstChildBoneIndex(const std::vector<SkeletalAnimationModelRecord::ModelBone>& a_boneList, const std::uint32_t a_boneIndex) const
{
    // 子ボーンは必ず親ボーンより後ろに並んでいるため、親の次から探す
    for (auto l_boneIndex = static_cast<std::size_t>(a_boneIndex) + k_nextBoneOffset; l_boneIndex < a_boneList.size(); ++l_boneIndex)
    {
        if (a_boneList[l_boneIndex].m_parentBoneIndex == a_boneIndex) { return static_cast<std::uint32_t>(l_boneIndex); }
    }

    return k_invalidIndex;
}