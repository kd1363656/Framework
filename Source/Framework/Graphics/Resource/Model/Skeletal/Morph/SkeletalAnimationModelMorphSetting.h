#pragma once

namespace FWK::Graphics
{
    // SkeletalAnimationModelのモーフターゲットごとの設定(エディターで編集する内容)を持つクラス
    //
    // ・FBXから読み込んだ移動量などのデータ(変更しない)とは分けて、.morphファイルに保存する
    // ・LOD保護の設定は、モデルのバイナリーファイルを作るとき(LODを作るとき)に各ModelMeshのターゲットへ写す
    // ・保存、読み込み、Inspectorの処理は専用のクラスに任せる
    //   バイナリ(.morph) : SkeletalAnimationModelMorphSettingBinaryConverter
    //   json             : SkeletalAnimationModelMorphSettingJsonConverter
    //   Inspector        : SkeletalAnimationModelMorphSettingEditInspector
    class SkeletalAnimationModelMorphSetting final
    {
    public:

         SkeletalAnimationModelMorphSetting() = default;
        ~SkeletalAnimationModelMorphSetting() = default;

        // a_modelFilePath : モデル(FBX)のパス(.morphファイルのパスはここから作る)
        // .morphファイルが無い場合はfalseを返す(設定は空のまま)
        bool LoadBinaryData(const std::filesystem::path& a_modelFilePath);

        void Deserialize(const nlohmann::json& a_rootJson);

        // 値が変わった場合はtrueを返す
        bool EditInspector();

        // ModelMesh1つ分のターゲットのうち、設定が無いターゲットの設定を既定値で追加する
        // 追加した場合はtrueを返す
        bool AddMissingTargetSetting(const std::vector<Struct::ModelMorphTarget>& a_targetList);

        // ModelMesh1つ分のターゲットへ、設定のLOD保護を写す(設定が無いターゲットは通常にする)
        void ApplyLODProtection(std::vector<Struct::ModelMorphTarget>& a_targetList) const;

        bool SaveBinaryData(const std::filesystem::path& a_modelFilePath) const;

        nlohmann::json Serialize() const;

        // ModelMesh1つ分のターゲットのLOD保護が、今の設定と同じか
        // 違う場合は、LODを作り直す必要がある
        bool IsLODProtectionApplied(const std::vector<Struct::ModelMorphTarget>& a_targetList) const;

        // 名前が一致するターゲットの設定を探す(無い場合はnullptr)
        const Struct::ModelMorphTargetSetting* FindPTRTargetSetting(const std::wstring& a_targetName) const;

        void SetTargetSettingList(std::vector<Struct::ModelMorphTargetSetting>&& a_set) { m_targetSettingList = std::move(a_set); }

        const auto& GetREFTargetSettingList() const { return m_targetSettingList; }

        auto& GetMutableREFTargetSettingList() { return m_targetSettingList; }

    private:

        Enum::ModelMorphLODProtection FetchVALLODProtection(const std::wstring& a_targetName) const;

        std::vector<Struct::ModelMorphTargetSetting> m_targetSettingList = {};

        SkeletalAnimationModelMorphSettingEditInspector m_editInspector = {};

        Converter::SkeletalAnimationModelMorphSettingJsonConverter m_jsonConverter = {};
    };
}