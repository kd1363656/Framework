#include "StaticModelStandardLitPassEditInspector.h"

bool FWK::Graphics::StaticModelStandardLitPassEditInspector::EditInspector(StaticModelStandardLitPass& a_staticModelStandardLitPass) const
{
    bool l_isOcclusionCullingEnabled = a_staticModelStandardLitPass.IsOcclusionCullingEnabled();

    // ImGui::Checkbox(表示する文字列、チェックの状態を読み書きする変数のアドレス)
    // クリックされてチェックの状態が変わったフレームだけtrueを返す
    const bool l_isChanged = ImGui::Checkbox(k_occlusionCullingLabel.data(), &l_isOcclusionCullingEnabled);

    Utility::IMGUIDelayedTooltip(k_occlusionCullingTooltip);

    if (!l_isChanged) { return false; }

    a_staticModelStandardLitPass.SetIsOcclusionCullingEnabled(l_isOcclusionCullingEnabled);

    return true;
}