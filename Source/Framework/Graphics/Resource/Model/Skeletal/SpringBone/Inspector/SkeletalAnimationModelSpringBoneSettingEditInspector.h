#pragma once

namespace FWK::Graphics
{
    class SkeletalAnimationModelSpringBoneSetting;
    class SkeletalAnimationModelRecord;
}

namespace FWK::Graphics
{
    // SkeletalAnimationModelSpringBoneSettingをInspectorで編集するクラス
    //
    // ・値を変えると、その場で揺れ方に反映される(揺らす計算側が設定の番号の変化を見て作り直す)
    // ・ドラッグを離した時点(編集が1回終わった時点)で、編集前と編集後の設定をUndo/Redoへ積む
    // ・「保存」を押すまで.springファイルには書き込まない
    class SkeletalAnimationModelSpringBoneSettingEditInspector final
    {
    public:

         SkeletalAnimationModelSpringBoneSettingEditInspector() = default;
        ~SkeletalAnimationModelSpringBoneSettingEditInspector() = default;

        // 値が変わった場合はtrueを返す
        // 編集中の状態(Undo用の編集前の設定)を覚えるため、constにしない
        bool EditInspector(SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting, const std::weak_ptr<SkeletalAnimationModelRecord>& a_ownerRecord);

    private:

        void UpdateBoneNameList(const std::shared_ptr<SkeletalAnimationModelRecord>& a_ownerRecord);

        bool EditGlobalSetting      (SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting) const;
        bool EditChainSettingList   (SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting) const;
        bool EditColliderSettingList(SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting) const;

        bool EditChainSetting    (Struct::ModelSpringBoneChainSetting&    a_chainSetting)    const;
        bool EditRootBoneNameList(std::vector<std::wstring>&              a_rootBoneNameList) const;
        bool EditColliderSetting (Struct::ModelSpringBoneColliderSetting& a_colliderSetting) const;

        // a_boneName : 選んだボーンの名前を書き込む(選ばなかった場合は変えない)
        bool SelectBoneName(const std::string_view& a_label, std::wstring& a_boneName) const;

        void CommitUndoCommand(SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting, const std::weak_ptr<SkeletalAnimationModelRecord>& a_ownerRecord);

        void DrawSaveButton(SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting, const SkeletalAnimationModelRecord& a_ownerRecord) const;

        std::string FetchVALChainHeaderLabel(const Struct::ModelSpringBoneChainSetting& a_chainSetting, const std::size_t a_chainSettingIndex) const;

        static constexpr std::array<const char*, static_cast<std::size_t>(Enum::ModelSpringBoneColliderShape::Count)> k_colliderShapeNameList = { "球", "カプセル" };

        static constexpr std::string_view k_noModelText            = "モデルがありません";
        static constexpr std::string_view k_unsavedText            = "未保存の変更があります";
        static constexpr std::string_view k_saveButtonLabel        = "保存(.spring)";
        static constexpr std::string_view k_globalHeaderLabel      = "全体";
        static constexpr std::string_view k_chainHeaderLabel       = "チェーン(揺らすボーン)";
        static constexpr std::string_view k_colliderHeaderLabel    = "当たり判定(体)";
        static constexpr std::string_view k_addChainButtonLabel    = "グループを追加";
        static constexpr std::string_view k_addRootButtonLabel     = "根元のボーンを追加";
        static constexpr std::string_view k_addColliderButtonLabel = "当たり判定を追加";
        static constexpr std::string_view k_removeButtonLabel      = "削除";
        static constexpr std::string_view k_unselectedBoneText     = "(未選択)";
        static constexpr std::string_view k_chainHeaderFormat      = "グループ{} : {}###ChainSettingTreeNode";
        static constexpr std::string_view k_rootBoneLabel          = "根元";
        static constexpr std::string_view k_colliderBoneLabel      = "ボーン";

        static constexpr std::string_view k_prewarmLabel               = "開始前の計算回数(Prewarm)";
        static constexpr std::string_view k_isEnabledLabel             = "有効";
        static constexpr std::string_view k_stiffnessLabel             = "戻る力(Stiffness)";
        static constexpr std::string_view k_dragLabel                  = "空気抵抗(Drag)";
        static constexpr std::string_view k_gravityPowerLabel          = "重力の強さ";
        static constexpr std::string_view k_gravityDirectionLabel      = "重力の向き";
        static constexpr std::string_view k_hitRadiusLabel             = "揺れもの側の半径";
        static constexpr std::string_view k_maxAngularVelocityLabel    = "角速度の上限(度/秒)";
        static constexpr std::string_view k_endLengthLabel             = "末端の長さ";
        static constexpr std::string_view k_isAdjacentChainLinkedLabel = "隣のチェーンとつなぐ(布)";
        static constexpr std::string_view k_isAdjacentChainLoopedLabel = "最後と最初もつなぐ(輪)";
        static constexpr std::string_view k_adjacentLinkStiffnessLabel = "つなぐバネの強さ(k)";
        static constexpr std::string_view k_colliderShapeLabel         = "形";
        static constexpr std::string_view k_colliderOffsetLabel        = "位置(ボーン基準)";
        static constexpr std::string_view k_colliderTailOffsetLabel    = "終点(ボーン基準)";
        static constexpr std::string_view k_colliderRadiusLabel        = "半径";

        static constexpr std::string_view k_prewarmTooltip               = "揺らし始める前に、画面に出さずに計算する回数です。\n1フレーム目の「フワッ」とした揺れを消します(60回 = 1秒分)。";
        static constexpr std::string_view k_stiffnessTooltip             = "アニメーションの向きへ戻ろうとする力です。\n大きいほど硬く、小さいほどよく揺れます。";
        static constexpr std::string_view k_dragTooltip                  = "動きを弱める割合です(0～1)。\n大きいほど揺れがすぐ止まり、小さいほど長く揺れ続けます。";
        static constexpr std::string_view k_gravityPowerTooltip          = "重力の向きへ引っ張る強さです。\nスカートや髪を垂れ下がらせたいときに使います。";
        static constexpr std::string_view k_hitRadiusTooltip             = "揺れもの(ボーンの先端)の太さです。\n当たり判定から、この半径の分だけ離して止めます。";
        static constexpr std::string_view k_maxAngularVelocityTooltip    = "1秒間に回ってよい角度の上限です(0 = 上限なし)。\n吹っ飛ぶなどの大きな動きで暴れるのを防ぎます。";
        static constexpr std::string_view k_endLengthTooltip             = "子ボーンが無い末端のボーンを、この長さだけ伸ばして揺らします(0 = 末端は揺らさない)。";
        static constexpr std::string_view k_isAdjacentChainLinkedTooltip = "根元の並び順で隣り合うチェーンの、同じ段のボーン同士を見えないバネでつなぎます。\nスカートが「のれん」のように開いて脚が貫通するのを防ぎます。";
        static constexpr std::string_view k_isAdjacentChainLoopedTooltip = "最後のチェーンと最初のチェーンもつなぎ、輪にします(3本以上のとき)。";
        static constexpr std::string_view k_adjacentLinkStiffnessTooltip = "つなぐバネの強さです(0～1500)。\n500～800くらいが丁度よい値です。";

        static constexpr float k_stiffnessDragSpeed             = 0.05F;
        static constexpr float k_stiffnessMIN                   = 0.0F;
        static constexpr float k_stiffnessMAX                   = 50.0F;
        static constexpr float k_dragDragSpeed                  = 0.005F;
        static constexpr float k_dragMIN                        = 0.0F;
        static constexpr float k_dragMAX                        = 1.0F;
        static constexpr float k_gravityPowerDragSpeed          = 0.05F;
        static constexpr float k_gravityPowerMIN                = 0.0F;
        static constexpr float k_gravityPowerMAX                = 50.0F;
        static constexpr float k_gravityDirectionDragSpeed      = 0.01F;
        static constexpr float k_gravityDirectionMIN            = -1.0F;
        static constexpr float k_gravityDirectionMAX            =  1.0F;
        static constexpr float k_lengthDragSpeed                = 0.001F;
        static constexpr float k_lengthMIN                      = 0.0F;
        static constexpr float k_lengthMAX                      = 10.0F;
        static constexpr float k_offsetDragSpeed                = 0.001F;
        static constexpr float k_maxAngularVelocityDragSpeed    = 1.0F;
        static constexpr float k_maxAngularVelocityMIN          = 0.0F;
        static constexpr float k_maxAngularVelocityMAX          = 3600.0F;
        static constexpr float k_adjacentLinkStiffnessDragSpeed = 1.0F;
        static constexpr float k_adjacentLinkStiffnessMIN       = 0.0F;
        static constexpr float k_adjacentLinkStiffnessMAX       = 1500.0F;

        static constexpr float k_prewarmDragSpeed = 1.0F;

        static constexpr std::uint32_t k_prewarmIterationCountMIN = 0U;
        static constexpr std::uint32_t k_prewarmIterationCountMAX = 600U;
        static constexpr std::uint32_t k_invalidRevision          = std::numeric_limits<std::uint32_t>::max();

        static constexpr std::size_t k_invalidListIndex = std::numeric_limits<std::size_t>::max();

        std::vector<std::string>  m_boneNameList  = {};
        std::vector<std::wstring> m_boneWNameList = {};

        std::weak_ptr<SkeletalAnimationModelRecord> m_boneNameListOwnerRecord = {};

        nlohmann::json m_committedJson = {};

        std::uint32_t m_committedRevision = k_invalidRevision;

        bool m_hasPendingEdit = false;
    };
}