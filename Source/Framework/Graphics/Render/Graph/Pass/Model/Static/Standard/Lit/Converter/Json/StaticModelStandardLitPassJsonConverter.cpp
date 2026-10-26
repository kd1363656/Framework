#include "StaticModelStandardLitPassJsonConverter.h"

void FWK::Converter::StaticModelStandardLitPassJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::StaticModelStandardLitPass& a_staticModelStandardLitPass) const
{
    // 古い設定ファイルにはキーが無いため、無ければ既定値(オクルージョンカリングを行う)にする
    const bool l_isOcclusionCullingEnabled = a_rootJson.value(k_isOcclusionCullingEnabledJsonKey, Constant::k_defaultStaticModelStandardLitPassIsOcclusionCullingEnabled);

    a_staticModelStandardLitPass.SetIsOcclusionCullingEnabled(l_isOcclusionCullingEnabled);
}

nlohmann::json FWK::Converter::StaticModelStandardLitPassJsonConverter::Serialize(const Graphics::StaticModelStandardLitPass& a_staticModelStandardLitPass) const
{
    return nlohmann::json
    {
        { k_isOcclusionCullingEnabledJsonKey.data(), a_staticModelStandardLitPass.IsOcclusionCullingEnabled() }
    };
}