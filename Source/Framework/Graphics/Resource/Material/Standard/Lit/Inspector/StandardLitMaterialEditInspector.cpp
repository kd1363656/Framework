#include "StandardLitMaterialInspector.h"

bool FWK::Graphics::StandardLitMaterialInspector::EditInspector(StandardLitMaterial& a_standardLitMaterial) const
{
    bool l_isChanged = false;

    // ベースカラーの変更
    if (auto l_baseColorFactor = a_standardLitMaterial.GetREFBaseColorFactor();
        ImGui::ColorEdit4(k_baseColorFactorLabel.data(), &l_baseColorFactor.x))
    {
        a_standardLitMaterial.SetBaseColorFactor(l_baseColorFactor);

        l_isChanged = true;
    }

    // ラフネスの変更
    if (float l_roughnessFactor = a_standardLitMaterial.GetVALRoughnessFactor();
        ImGui::SliderFloat(k_roughnessFactorLabel.data(), &l_roughnessFactor, k_minFactor, k_maxFactor))
    {
        a_standardLitMaterial.SetRoughnessFactor(l_roughnessFactor);

        l_isChanged = true;
    }

    // メタリックの変更
    if (float l_metallicFactor = a_standardLitMaterial.GetVALMetallicFactor();
        ImGui::SliderFloat(k_metallicFactorLabel.data(), &l_metallicFactor, k_minFactor, k_maxFactor))
    {
        a_standardLitMaterial.SetMetallicFactor(l_metallicFactor);

        l_isChanged = true;
    }

    // テクスチャは読み込み直しが必要になるため、ここでは表示だけ行う
    const auto& l_baseColorTextureFilePath = a_standardLitMaterial.GetREFBaseColorTextureFilePath();
    const auto& l_normalTextureFilePath    = a_standardLitMaterial.GetREFNormalTextureFilePath   ();
    const auto& l_roughnessTextureFilePath = a_standardLitMaterial.GetREFRoughnessTextureFilePath();
    const auto& l_metallicTextureFilePath  = a_standardLitMaterial.GetREFMetallicTextureFilePath ();

    ImGui::Text(k_baseColorTextureLabel.data(), Utility::WStringToString(l_baseColorTextureFilePath).c_str());
    ImGui::Text(k_normalTextureLabel.data(),    Utility::WStringToString(l_normalTextureFilePath).c_str());
    ImGui::Text(k_roughnessTextureLabel.data(), Utility::WStringToString(l_roughnessTextureFilePath).c_str());
    ImGui::Text(k_metallicTextureLabel.data(),  Utility::WStringToString(l_metallicTextureFilePath).c_str());

    return l_isChanged;
}