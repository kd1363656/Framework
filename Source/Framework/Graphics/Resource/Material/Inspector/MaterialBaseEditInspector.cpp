#include "MaterialBaseEditInspector.h"

void FWK::Graphics::MaterialBaseEditInspector::EditInspector(MaterialBase& a_materialBase) const
{
    const auto& l_typeINFO = a_materialBase.GetREFRuntimeTypeINFO();
    const auto& l_filePath = a_materialBase.GetREFFilePath       ();

    ImGui::Text(k_typeNameLabel.data(), l_typeINFO.k_name.data());
    ImGui::Text(k_filePathLabel.data(), Utility::WStringToString(l_filePath).c_str());

    // 値が変わったら、次のフレームでGPUDataBufferを作り直す
    if (a_materialBase.EditInspectorParameter())
    {
        a_materialBase.SetIsGPUDataDirty(true);
    }

    // ファイルを持たないMaterial(デフォルトマテリアル)は保存しない
    if (l_filePath.empty()) { return; }

    if (ImGui::Button(k_saveButtonLabel.data()))
    {
        FWK_ASSERT_RETURN_IF(!a_materialBase.SaveBinaryData(l_filePath), "Materialの保存に失敗しました。");
    }
}