#include "ResourceContext.h"

void FWK::Graphics::ResourceContext::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::ResourceContext::PostDeserialize(const Device& a_device)
{
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_rtvDescriptorPool.Create(a_device), "RTVDescriptorPoolの作成処理に失敗しました。", false);
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_srvDescriptorPool.Create(a_device), "SRVDescriptorPoolの作成処理に失敗しました。", false);

    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_uploadSystem.Create(a_device), "アップロードシステムの作成処理に失敗しました。", false);

    return true;
}

nlohmann::json FWK::Graphics::ResourceContext::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}