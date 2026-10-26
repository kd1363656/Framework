#pragma once

namespace FWK::Graphics
{
    // 揺れもの(SpringBone : 胸揺れ、スカートや髪などの衣装揺れ)を計算し、アニメーションの姿勢へ上書きするクラス
    //
    // 仕組み(1本のボーンを「根元(head)から先端(tail)へ伸びる棒」として考える)
    //   1. アニメーションの姿勢から、棒の「本来の向き(先端の位置)」を求める
    //   2. 先端だけを、前のフレームからの動き(慣性)・本来の向きへ戻る力(Stiffness)・重力で動かす(ベレの方法)
    //   3. 棒の長さを元に戻し、当たり判定(体)にめり込んでいたら押し出す
    //   4. 本来の向き → 動かした先端の向き へ回す回転をボーンへ掛ける(子ボーンも一緒に回る)
    //
    // 暴れを抑える工夫(COLOPL SpringBone第1回)
    //   ・1回の計算で先端が回ってよい角度を「角速度の上限」で抑える(大きな動きでも暴れない)
    //   ・揺らし始める前に、画面に出さずに何回か計算して落ち着かせる(Prewarm : 開始時のフワッとした揺れを消す)
    //   ・計算は1秒60回の決まった間隔で行う(フレームの時間がばらついても結果が変わらない)
    //
    // 布の表現(COLOPL SpringBone第2回)
    //   ・隣り合うチェーンの同じ段のボーン同士を見えないバネでつなぎ、距離を保つ(スカートが「のれん」のように開かない)
    //   ・バネの強さkはエディターで0～1500まで変えられる(SpringBoneのk = 0.5固定では弱すぎて効かなかった)
    //
    // 座標
    //   ・揺らす計算はワールド座標で行う(キャラクターが歩く・走ると、その動きで揺れる)
    //   ・最後にモデルの座標へ戻して、GPUへ送るボーンの行列(GlobalBoneMatrix)を書き換える
    class SkeletalAnimationSpringBoneSolver final
    {
    private:

        // 揺らすボーン1本分(根元 = このボーンの位置、先端 = 子ボーンの位置)
        // 先端の位置はワールド座標
        struct Joint final
        {
            TypeAlias::Math::Vector3 m_localTailPosition    = TypeAlias::Math::Vector3::Zero;
            TypeAlias::Math::Vector3 m_currentTailPosition  = TypeAlias::Math::Vector3::Zero;
            TypeAlias::Math::Vector3 m_previousTailPosition = TypeAlias::Math::Vector3::Zero;
            TypeAlias::Math::Vector3 m_linkOffset           = TypeAlias::Math::Vector3::Zero;

            std::uint32_t m_boneIndex         = k_invalidIndex;
            std::uint32_t m_chainSettingIndex = k_invalidIndex;
        };

        // 隣り合うチェーンの、同じ段の先端同士をつなぐ見えないバネ
        struct Link final
        {
            float m_restLength = k_initialLength;

            std::uint32_t m_jointIndexA       = k_invalidIndex;
            std::uint32_t m_jointIndexB       = k_invalidIndex;
            std::uint32_t m_chainSettingIndex = k_invalidIndex;
        };

        struct Collider final
        {
            std::uint32_t m_boneIndex            = k_invalidIndex;
            std::uint32_t m_colliderSettingIndex = k_invalidIndex;
        };

        // そのフレームのワールド座標での当たり判定(球はm_head == m_tail のカプセルとして扱う)
        struct WorldCollider final
        {
            TypeAlias::Math::Vector3 m_head = TypeAlias::Math::Vector3::Zero;
            TypeAlias::Math::Vector3 m_tail = TypeAlias::Math::Vector3::Zero;

            float m_radius = k_initialLength;
        };

    public:

         SkeletalAnimationSpringBoneSolver() = default;
        ~SkeletalAnimationSpringBoneSolver() = default;

        // 次のUpdateでチェーンを作り直し、Prewarmからやり直す(モデルの切り替え、瞬間移動のときに呼ぶ)
        void Reset();

        // a_globalBoneMatrixList : アニメーションで計算した姿勢(モデルの座標)。揺らした結果で上書きする
        void Update(const SkeletalAnimationModelRecord::ModelData&     a_modelData,
                    const SkeletalAnimationModelSpringBoneSetting&     a_springBoneSetting,
                    const float                                        a_deltaTime,
                    const TypeAlias::Math::Matrix&                     a_worldMatrix,
                          std::vector<TypeAlias::Math::Matrix>&        a_globalBoneMatrixList);

    private:

        void Build(const SkeletalAnimationModelRecord::ModelData& a_modelData, const SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting);

        void BuildChain(const std::vector<SkeletalAnimationModelRecord::ModelBone>& a_boneList,
                        const Struct::ModelSpringBoneChainSetting&                  a_chainSetting,
                        const std::uint32_t                                         a_chainSettingIndex,
                        const std::uint32_t                                         a_rootBoneIndex,
                              std::vector<std::uint32_t>&                           a_chainBoneIndexList);

        void BuildAdjacentLink(const std::vector<std::vector<std::uint32_t>>& a_chainBoneIndexList, const Struct::ModelSpringBoneChainSetting& a_chainSetting, const std::uint32_t a_chainSettingIndex);

        void UpdateAnimatedBoneMatrix(const std::vector<SkeletalAnimationModelRecord::ModelBone>& a_boneList, const TypeAlias::Math::Matrix& a_worldMatrix, const std::vector<TypeAlias::Math::Matrix>& a_globalBoneMatrixList);
        void UpdateWorldCollider     (const SkeletalAnimationModelSpringBoneSetting&              a_springBoneSetting, const float a_worldScale);

        void InitializeTail(const std::vector<SkeletalAnimationModelRecord::ModelBone>& a_boneList, const TypeAlias::Math::Matrix& a_worldMatrix);

        void Step(const std::vector<SkeletalAnimationModelRecord::ModelBone>& a_boneList,
                  const SkeletalAnimationModelSpringBoneSetting&               a_springBoneSetting,
                  const TypeAlias::Math::Matrix&                               a_worldMatrix,
                  const float                                                  a_worldScale);

        void UpdateLinkOffset(const SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting);

        // a_isTailIntegrated : trueなら先端を1回分動かす、falseなら今の先端の位置へボーンを向けるだけ
        void SolveJoint(const std::vector<SkeletalAnimationModelRecord::ModelBone>& a_boneList,
                        const SkeletalAnimationModelSpringBoneSetting&               a_springBoneSetting,
                        const TypeAlias::Math::Matrix&                               a_worldMatrix,
                        const float                                                  a_worldScale,
                        const bool                                                   a_isTailIntegrated,
                              Joint&                                                 a_joint);

        void WriteBack(const std::vector<SkeletalAnimationModelRecord::ModelBone>& a_boneList,
                       const SkeletalAnimationModelSpringBoneSetting&               a_springBoneSetting,
                       const TypeAlias::Math::Matrix&                               a_worldMatrix,
                       const float                                                  a_worldScale,
                             std::vector<TypeAlias::Math::Matrix>&                  a_globalBoneMatrixList);

        TypeAlias::Math::Vector3 PushOutFromCollider (const TypeAlias::Math::Vector3& a_tailPosition, const float a_hitRadius) const;
        TypeAlias::Math::Vector3 LimitAngularVelocity(const TypeAlias::Math::Vector3& a_headPosition,
                                                      const TypeAlias::Math::Vector3& a_previousTailPosition,
                                                      const TypeAlias::Math::Vector3& a_nextTailPosition,
                                                      const float                     a_maxAngleRadian) const;

        TypeAlias::Math::Quaternion CreateFromToRotation(const TypeAlias::Math::Vector3& a_fromDirection, const TypeAlias::Math::Vector3& a_toDirection) const;

        std::uint32_t FindVALBoneIndex          (const std::vector<SkeletalAnimationModelRecord::ModelBone>& a_boneList, const std::wstring&  a_boneName)  const;
        std::uint32_t FindVALFirstChildBoneIndex(const std::vector<SkeletalAnimationModelRecord::ModelBone>& a_boneList, const std::uint32_t a_boneIndex) const;

        static constexpr std::uint32_t k_invalidIndex = std::numeric_limits<std::uint32_t>::max();

        static constexpr float k_initialLength = 0.0F;

        // 揺らす計算の1回分の時間(1秒60回)
        static constexpr float k_stepSecond = 1.0F / 60.0F;

        // 1フレームで計算してよい回数の上限(処理落ちで時間が飛んだときに、計算が追いつかなくなるのを防ぐ)
        static constexpr std::uint32_t k_maxStepCountPerFrame = 4U;

        // COLOPL SpringBone第2回の式 x(n+1) = x(n) + D・v(n)・Δt + k・(1/2)・(ΔL/M)・Δt² の 1/2 と M
        static constexpr float k_linkSpringHalfRatio = 0.5F;
        static constexpr float k_linkMass            = 1.0F;

        static constexpr float k_noDragRatio = 1.0F;

        static constexpr float k_minLength = 0.000001F;

        static constexpr float k_initialAccumulatedSecond = 0.0F;

        static constexpr float k_minInterpolationRatio = 0.0F;
        static constexpr float k_maxInterpolationRatio = 1.0F;

        static constexpr float k_minAngleRadian = 0.0F;

        static constexpr float k_minDotProduct = -1.0F;
        static constexpr float k_maxDotProduct =  1.0F;

        static constexpr float k_quaternionRealPartOffset = 1.0F;
        static constexpr float k_halfTurnQuaternionW      = 0.0F;

        static constexpr std::uint32_t k_initialStepCount = 0U;

        static constexpr std::size_t k_nextBoneOffset      = 1ULL;
        static constexpr std::size_t k_nextChainOffset     = 1ULL;
        static constexpr std::size_t k_minLoopedChainCount = 3ULL;
        static constexpr std::size_t k_firstChainIndex     = 0ULL;

        static constexpr TypeAlias::Math::Vector3 k_defaultEndDirection  = { 0.0F, 1.0F, 0.0F };
        static constexpr TypeAlias::Math::Vector3 k_halfTurnAxisCandidate = { 1.0F, 0.0F, 0.0F };
        static constexpr TypeAlias::Math::Vector3 k_halfTurnAxisFallback  = { 0.0F, 1.0F, 0.0F };

        std::vector<Joint>         m_jointList         = {};
        std::vector<Link>          m_linkList          = {};
        std::vector<Collider>      m_colliderList      = {};
        std::vector<WorldCollider> m_worldColliderList = {};

        std::vector<TypeAlias::Math::Matrix> m_animatedLocalBoneMatrixList = {};
        std::vector<TypeAlias::Math::Matrix> m_worldBoneMatrixList         = {};

        // ※ 番号 = ボーンの番号。揺らすボーンならm_jointListの番号、揺らさないならk_invalidIndex
        std::vector<std::uint32_t> m_boneJointIndexList = {};

        // ※ 番号 = ボーンの番号。揺らすボーンか、その子孫ならtrue(書き戻すときに計算し直すボーン)
        std::vector<bool> m_boneAffectedList = {};

        float m_accumulatedSecond = k_initialAccumulatedSecond;

        std::uint32_t m_builtRevision = k_invalidIndex;

        bool m_isBuilt           = false;
        bool m_isTailInitialized = false;
    };
}