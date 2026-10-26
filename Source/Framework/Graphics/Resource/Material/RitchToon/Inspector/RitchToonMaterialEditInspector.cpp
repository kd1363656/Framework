#include "RitchToonMaterialEditInspector.h"

bool FWK::Graphics::RitchToonMaterialEditInspector::EditInspector(RitchToonMaterial& a_ritchToonMaterial) const
{
    // どれか1つでも変わったらtrueにする(|=を使い、前の項目の結果を消さない)
    bool l_isChanged = false;

    l_isChanged |= EditColor (a_ritchToonMaterial);
    l_isChanged |= EditFactor(a_ritchToonMaterial);

    // テクスチャは読み込み直しが必要になるため、ここでは表示だけ行う
    ShowTexture(a_ritchToonMaterial);

    return l_isChanged;
}

bool FWK::Graphics::RitchToonMaterialEditInspector::EditColor(RitchToonMaterial& a_ritchToonMaterial) const
{
    bool l_isChanged = false;

    // ベースカラーは不透明度(A)も編集する
    if (auto l_color = a_ritchToonMaterial.GetREFBaseColorFactor();
        ImGui::ColorEdit4(k_baseColorFactorLabel.data(), &l_color.x))
    {
        a_ritchToonMaterial.SetBaseColorFactor(l_color);

        l_isChanged = true;
    }

    // 影の所の色(メインテクスチャの色に掛ける)
    if (auto l_color = a_ritchToonMaterial.GetREFShadowColor();
        ImGui::ColorEdit3(k_shadowColorLabel.data(), &l_color.x))
    {
        a_ritchToonMaterial.SetShadowColor(l_color);

        l_isChanged = true;
    }

    if (auto l_color = a_ritchToonMaterial.GetREFMatcapColor();
        ImGui::ColorEdit3(k_matcapColorLabel.data(), &l_color.x, k_hdrColorEditFlags))
    {
        a_ritchToonMaterial.SetMatcapColor(l_color);

        l_isChanged = true;
    }

    if (auto l_color = a_ritchToonMaterial.GetREFSpecularColor();
        ImGui::ColorEdit3(k_specularColorLabel.data(), &l_color.x, k_hdrColorEditFlags))
    {
        a_ritchToonMaterial.SetSpecularColor(l_color);

        l_isChanged = true;
    }

    if (auto l_color = a_ritchToonMaterial.GetREFEmissionColor();
        ImGui::ColorEdit3(k_emissionColorLabel.data(), &l_color.x, k_hdrColorEditFlags))
    {
        a_ritchToonMaterial.SetEmissionColor(l_color);

        l_isChanged = true;
    }

    if (auto l_color = a_ritchToonMaterial.GetREFOutlineColor();
        ImGui::ColorEdit3(k_outlineColorLabel.data(), &l_color.x))
    {
        a_ritchToonMaterial.SetOutlineColor(l_color);

        l_isChanged = true;
    }

    return l_isChanged;
}
bool FWK::Graphics::RitchToonMaterialEditInspector::EditFactor(RitchToonMaterial& a_ritchToonMaterial) const
{
    bool l_isChanged = false;

    // 影の境界 : 光の当たり具合(0.0F = 真裏、0.5F = 真横、1.0F = 正面)がこの値より小さい所が影になる
    if (float l_shadowBorder = a_ritchToonMaterial.GetVALShadowBorder();
        ImGui::SliderFloat(k_shadowBorderLabel.data(), &l_shadowBorder, k_minFactor, k_maxFactor))
    {
        a_ritchToonMaterial.SetShadowBorder(l_shadowBorder);

        l_isChanged = true;
    }

    // 影のぼかし : 0.0Fに近いほど、影の境界がくっきりする
    if (float l_shadowBlur = a_ritchToonMaterial.GetVALShadowBlur();
        ImGui::SliderFloat(k_shadowBlurLabel.data(), &l_shadowBlur, k_minFactor, k_maxFactor))
    {
        a_ritchToonMaterial.SetShadowBlur(l_shadowBlur);

        l_isChanged = true;
    }

    // 法線の強さ : 0.0Fでノーマルマップを使わない(頂点の法線のまま)
    if (float l_normalStrength = a_ritchToonMaterial.GetVALNormalStrength();
        ImGui::SliderFloat(k_normalStrengthLabel.data(), &l_normalStrength, k_minFactor, k_maxFactor))
    {
        a_ritchToonMaterial.SetNormalStrength(l_normalStrength);

        l_isChanged = true;
    }

    // 滑らかさ : 1.0Fに近いほど、光沢のハイライトが小さく鋭くなる
    if (float l_smoothness = a_ritchToonMaterial.GetVALSmoothness();
        ImGui::SliderFloat(k_smoothnessLabel.data(), &l_smoothness, k_minFactor, k_maxFactor))
    {
        a_ritchToonMaterial.SetSmoothness(l_smoothness);

        l_isChanged = true;
    }

    // 輪郭線の太さ(メートル)
    // カメラから1m離れて見たときの太さ。描画時はカメラからの距離に合わせて太さを変え、画面上の太さをほぼ一定に保つ
    if (float l_outlineWidth = a_ritchToonMaterial.GetVALOutlineWidth();
        ImGui::SliderFloat(k_outlineWidthLabel.data(), &l_outlineWidth, k_minFactor, k_maxOutlineWidthMeter, k_outlineWidthFormat.data()))
    {
        a_ritchToonMaterial.SetOutlineWidth(l_outlineWidth);

        l_isChanged = true;
    }

    // 発光にメインテクスチャの色を混ぜる : 1.0Fで「発光色 × メインテクスチャの色」で光る(髪の毛先を髪の色で光らせるなど)
    if (float l_emissionBaseColorBlend = a_ritchToonMaterial.GetVALEmissionBaseColorBlend();
        ImGui::SliderFloat(k_emissionBaseColorLabel.data(), &l_emissionBaseColorBlend, k_minFactor, k_maxFactor))
    {
        a_ritchToonMaterial.SetEmissionBaseColorBlend(l_emissionBaseColorBlend);

        l_isChanged = true;
    }

    // 発光で元の色を置き換える : 0.0Fで元の色に足す、1.0Fで発光マスクの濃さの分だけ発光色へ置き換える(髪のグラデーションなど)
    if (float l_emissionReplaceRate = a_ritchToonMaterial.GetVALEmissionReplaceRate();
        ImGui::SliderFloat(k_emissionReplaceLabel.data(), &l_emissionReplaceRate, k_minFactor, k_maxFactor))
    {
        a_ritchToonMaterial.SetEmissionReplaceRate(l_emissionReplaceRate);

        l_isChanged = true;
    }

    return l_isChanged;
}
void FWK::Graphics::RitchToonMaterialEditInspector::ShowTexture(const RitchToonMaterial& a_ritchToonMaterial) const
{
    const auto& l_baseColorTextureFilePath    = a_ritchToonMaterial.GetREFBaseColorTextureFilePath   ();
    const auto& l_normalTextureFilePath       = a_ritchToonMaterial.GetREFNormalTextureFilePath      ();
    const auto& l_shadowMaskTextureFilePath   = a_ritchToonMaterial.GetREFShadowMaskTextureFilePath  ();
    const auto& l_matcapTextureFilePath       = a_ritchToonMaterial.GetREFMatcapTextureFilePath      ();
    const auto& l_matcapMaskTextureFilePath   = a_ritchToonMaterial.GetREFMatcapMaskTextureFilePath  ();
    const auto& l_metallicMaskTextureFilePath = a_ritchToonMaterial.GetREFMetallicMaskTextureFilePath();
    const auto& l_emissionMaskTextureFilePath = a_ritchToonMaterial.GetREFEmissionMaskTextureFilePath();
    const auto& l_outlineMaskTextureFilePath  = a_ritchToonMaterial.GetREFOutlineMaskTextureFilePath ();

    ImGui::Text(k_baseColorTextureLabel.data(),    Utility::WStringToString(l_baseColorTextureFilePath).c_str());
    ImGui::Text(k_normalTextureLabel.data(),       Utility::WStringToString(l_normalTextureFilePath).c_str());
    ImGui::Text(k_shadowMaskTextureLabel.data(),   Utility::WStringToString(l_shadowMaskTextureFilePath).c_str());
    ImGui::Text(k_matcapTextureLabel.data(),       Utility::WStringToString(l_matcapTextureFilePath).c_str());
    ImGui::Text(k_matcapMaskTextureLabel.data(),   Utility::WStringToString(l_matcapMaskTextureFilePath).c_str());
    ImGui::Text(k_metallicMaskTextureLabel.data(), Utility::WStringToString(l_metallicMaskTextureFilePath).c_str());
    ImGui::Text(k_emissionMaskTextureLabel.data(), Utility::WStringToString(l_emissionMaskTextureFilePath).c_str());
    ImGui::Text(k_outlineMaskTextureLabel.data(),  Utility::WStringToString(l_outlineMaskTextureFilePath).c_str());
}