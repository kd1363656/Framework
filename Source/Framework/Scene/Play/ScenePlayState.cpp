#include "ScenePlayState.h"

void FWK::ScenePlayState::BeginPlay(nlohmann::json&& a_snapshotJson, const std::filesystem::path& a_snapshotSceneFilePath)
{
    // 停止したときに戻す内容を覚えておく
    m_snapshotJson          = std::move(a_snapshotJson);
    m_snapshotSceneFilePath = a_snapshotSceneFilePath;

    m_isPlaying = true;
}
void FWK::ScenePlayState::EndPlay()
{
    // 戻し終えた内容は不要なので捨てる(シーン全体のjsonなので持ち続けない)
    m_snapshotJson          = nlohmann::json{};
    m_snapshotSceneFilePath.clear();

    m_isPlaying = false;
}
