#pragma once

namespace FWK::Graphics
{
    class MaterialBase;
}

namespace FWK::Graphics
{
    // MaterialBaseの共通部分(型名、ファイルパス、保存ボタン)をInspectorに表示するクラス
    // 派生クラス固有の値の編集は、MaterialBase::EditInspectorParameterを通して
    // 派生クラスごとのEditInspector(例 : StandardLitMaterialEditInspector)が行う
    class MaterialBaseEditInspector final
    {
    public:

         MaterialBaseEditInspector() = default;
        ~MaterialBaseEditInspector() = default;

        // 値が変わった場合はGPUDataBufferの作り直しが必要になるため、作り直し待ちの状態にする
        void EditInspector(MaterialBase& a_materialBase) const;

    private:

        static constexpr std::string_view k_typeNameLabel   = "Type : %s";
        static constexpr std::string_view k_filePathLabel   = "File : %s";
        static constexpr std::string_view k_saveButtonLabel = "Save";
    };
}