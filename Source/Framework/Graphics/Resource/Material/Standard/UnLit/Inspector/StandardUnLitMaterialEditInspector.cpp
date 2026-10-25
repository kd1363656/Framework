#include "StandardUnLitMaterialEditInspector.h"

bool FWK::Graphics::StandardUnLitMaterialEditInspector::EditInspector(StandardUnLitMaterial& a_standardUnLitMaterial) const
{
    bool l_isChanged = false;

    // ベースカラーの変更
    if (auto l_baseColorFactor = a_standardUnLitMaterial.GetREFBaseColorFactor();
        ImGui::ColorEdit4(k_baseColorFactorLabel.data(), &l_baseColorFactor.x))
    {
        a_standardUnLitMaterial.SetBaseColorFactor(l_baseColorFactor);

        l_isChanged = true;
    }

    // テクスチャは読み込み直しが必要になるため、ここでは表示だけ行う
    const auto& l_baseColorTextureFilePath = a_standardUnLitMaterial.GetREFBaseColorTextureFilePath();

    ImGui::Text(k_baseColorTextureLabel.data(), Utility::WStringToString(l_baseColorTextureFilePath).c_str());

    return l_isChanged;
}