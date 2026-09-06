#include "AssetBrowserEditorWindowDirectoryChangeBase.h"

bool FWK::Editor::AssetBrowserEditorWindowDirectoryChangeBase::IsChildFilePath(const std::filesystem::path& a_filePath, const std::filesystem::path& a_parentDirectoryPath)
{
	if (a_filePath.empty() ||
		a_parentDirectoryPath.empty())
	{
		return false;
	}

	auto l_filePathITR            = a_filePath.begin           ();
	auto l_parentDirectoryPathITR = a_parentDirectoryPath.begin();

	while (l_parentDirectoryPathITR != a_parentDirectoryPath.end())
	{
		if (l_filePathITR == a_filePath.end() ||
			*l_filePathITR != *l_parentDirectoryPathITR)
		{
			return false; 
		}

		++l_parentDirectoryPathITR;
		++l_filePathITR;
	}

	// Parentと完全一致でなく
	// Parentより後にもPathComponentが存在すれば子孫
	return l_filePathITR != a_filePath.end();
}