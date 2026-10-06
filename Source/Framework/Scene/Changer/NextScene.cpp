#include "NextScene.h"

void FWK::NextScene::Load(const std::filesystem::path& a_filePath)
{
    if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerJsonExtension)) { return; }

    auto&& l_rootJson = Utility::LoadJsonFile(a_filePath);

    m_jsonConverter.Load(*this, std::move(l_rootJson));
}