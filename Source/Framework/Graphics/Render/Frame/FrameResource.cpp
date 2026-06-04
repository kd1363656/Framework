#include "FrameResource.h"

void FWK::Graphics::FrameResource::INIT()
{
	if (!m_directCommandAllocator)
	{
		m_directCommandAllocator = std::make_shared<DirectCommandAllocator>();
	}
}
bool FWK::Graphics::FrameResource::Create(const Device& a_device)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_directCommandAllocator,				   "ダイレクトコマンドアロケータが無効です。",				 false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_directCommandAllocator->Create(a_device), "ダイレクトコマンドアロケータの作成処理に失敗しました。", false);

	return true;
}
void FWK::Graphics::FrameResource::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }
}

nlohmann::json FWK::Graphics::FrameResource::Serialize() const
{
	nlohmann::json l_rootJson = {};

	return l_rootJson;
}