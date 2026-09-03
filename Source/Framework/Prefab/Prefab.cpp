#include "Prefab.h"

void FWK::Prefab::Load(const std::filesystem::path& a_filePath)
{
	if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerJsonExtension)) { return; }

	const auto& l_rootJson = Utility::LoadJsonFile(a_filePath);

	m_jsonConverter.Load(l_rootJson, *this);
}

bool FWK::Prefab::Save(const std::filesystem::path & a_filePath)
{
	if (a_filePath.empty() ||
		a_filePath.extension() != Constant::k_lowerJsonExtension) 
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "Prefabの保存先FilePathが無効です。\nFilePath : {}", a_filePath.string());

		return false; 
	}

	return m_jsonConverter.Save(a_filePath, *this);
}