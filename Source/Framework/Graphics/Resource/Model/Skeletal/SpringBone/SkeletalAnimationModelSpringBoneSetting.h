#pragma once

namespace FWK::Graphics
{
    class SkeletalAnimationModelRecord;
}

namespace FWK::Graphics
{
    // SkeletalAnimationModelの揺れもの(SpringBone : 胸揺れ、スカートや髪などの衣装揺れ)の設定(エディターで編集する内容)を持つクラス
    //
    // ・FBXから読み込んだボーンなどのデータ(変更しない)とは分けて、.springファイルに保存する
    // ・.springファイルが無いモデルは揺らさない(ベイクの方式で描画できる)
    // ・実際に揺らす計算はSkeletalAnimationSpringBoneSolverが行い、このクラスは「どう揺らすか」だけを持つ
    // ・保存、読み込み、Inspectorの処理は専用のクラスに任せる
    //   バイナリ(.spring) : SkeletalAnimationModelSpringBoneSettingBinaryConverter
    //   json              : SkeletalAnimationModelSpringBoneSettingJsonConverter(Undo/Redoで設定を丸ごと戻すのにも使う)
    //   Inspector         : SkeletalAnimationModelSpringBoneSettingEditInspector
    class SkeletalAnimationModelSpringBoneSetting final
    {
    public:

         SkeletalAnimationModelSpringBoneSetting() = default;
        ~SkeletalAnimationModelSpringBoneSetting() = default;

        // BinaryConverterは読み書き中のファイルを持つためコピー・ムーブできない
        // そのため、ムーブでは設定の値だけを移し、Converterなどは移さない
        SkeletalAnimationModelSpringBoneSetting(const SkeletalAnimationModelSpringBoneSetting&)         = delete;
        SkeletalAnimationModelSpringBoneSetting(      SkeletalAnimationModelSpringBoneSetting&& a_other) noexcept;

        SkeletalAnimationModelSpringBoneSetting& operator=(const SkeletalAnimationModelSpringBoneSetting&)         = delete;
        SkeletalAnimationModelSpringBoneSetting& operator=(      SkeletalAnimationModelSpringBoneSetting&& a_other) noexcept;

        // a_modelFilePath : モデル(FBX)のパス(.springファイルのパスはここから作る)
        // .springファイルが無い場合はfalseを返す(設定は空のまま = 揺らさない)
        bool LoadBinaryData(const std::filesystem::path& a_modelFilePath);

        void Deserialize(const nlohmann::json& a_rootJson);

        // a_ownerRecord : この設定を持っているモデル(ボーンの名前の一覧、保存先のパス、Undo/Redoの対象に使う)
        // 値が変わった場合はtrueを返す
        bool EditInspector(const std::weak_ptr<SkeletalAnimationModelRecord>& a_ownerRecord);

        // BinaryConverterが書き込み中のファイルを持つため、constにしない
        bool SaveBinaryData(const std::filesystem::path& a_modelFilePath);

        nlohmann::json Serialize() const;

        // 設定を変えたことを知らせる(SkeletalAnimationSpringBoneSolverは、番号が変わったらチェーンを作り直す)
        void MarkChanged();

        // 有効なチェーンが1つでもあるか(無い場合は揺らす計算をしない)
        bool IsActive() const;

        void SetChainSettingList   (std::vector<Struct::ModelSpringBoneChainSetting>&&    a_set) { m_chainSettingList    = std::move(a_set); }
        void SetColliderSettingList(std::vector<Struct::ModelSpringBoneColliderSetting>&& a_set) { m_colliderSettingList = std::move(a_set); }

        void SetPrewarmIterationCount(const std::uint32_t a_set) { m_prewarmIterationCount = a_set; }

        const auto& GetREFChainSettingList   () const { return m_chainSettingList; }
        const auto& GetREFColliderSettingList() const { return m_colliderSettingList; }

        auto GetVALPrewarmIterationCount() const { return m_prewarmIterationCount; }
        auto GetVALRevision             () const { return m_revision; }

        bool GetVALIsUnsaved() const { return m_isUnsaved; }

        auto& GetMutableREFChainSettingList   () { return m_chainSettingList; }
        auto& GetMutableREFColliderSettingList() { return m_colliderSettingList; }

    private:

        std::vector<Struct::ModelSpringBoneChainSetting>    m_chainSettingList    = {};
        std::vector<Struct::ModelSpringBoneColliderSetting> m_colliderSettingList = {};

        SkeletalAnimationModelSpringBoneSettingEditInspector m_editInspector = {};

        Converter::SkeletalAnimationModelSpringBoneSettingBinaryConverter m_binaryConverter = {};
        Converter::SkeletalAnimationModelSpringBoneSettingJsonConverter   m_jsonConverter   = {};

        std::uint32_t m_prewarmIterationCount = Constant::k_defaultModelSpringBonePrewarmIterationCount;

        // ※ 実行中だけ使う値(.springファイルには保存しない)
        std::uint32_t m_revision = Constant::k_initialModelSpringBoneSettingRevision;

        bool m_isUnsaved = false;
    };
}