#pragma once

namespace FWK::Utility
{
	inline std::filesystem::path ReplaceRoot(const std::filesystem::path& a_filePath, const std::filesystem::path& a_oldRootDirectoryPath, const std::filesystem::path& a_newRootDirectoryPath)
	{
		if (a_filePath.empty()             ||
			a_oldRootDirectoryPath.empty() ||
			a_newRootDirectoryPath.empty())
		{
			return a_filePath;
		}

		auto l_filePathITR             = a_filePath.begin            ();
		auto l_oldRootDirectoryPathITR = a_oldRootDirectoryPath.begin();

		// a_filePathがa_oldRootDirectoryPath自身、
		// またはその子階層に存在しているか確認する
		// 例 FilePath : Asset/Character/Player/Texture
		//    OldRoot  : Asset/Character
		// の場合、Asset     == Asset
		//         Character == Character
		// なのでOldRoot配下に存在していると判断できる
		// Path全体をstringへ変換して判断せず
		// std::filesystem::pathのComponent単位で比較する
		while (l_oldRootDirectoryPathITR != a_oldRootDirectoryPath.end())
		{
			// OldRootを最後まで比較する前にFilePath側が終了した場合
			// 例 : FilePath : Asset
			//      OldRoot  : Asset/Character
			if (l_filePathITR == a_filePath.end() ||
				*l_filePathITR != *l_oldRootDirectoryPathITR) 
			{
				return a_filePath; 
			}

			++l_filePathITR;
			++l_oldRootDirectoryPathITR;
		}

		// Old部分をNewRootへ置き換える
		std::filesystem::path l_result = a_newRootDirectoryPath;

		// この時点でFilePathITRは
		// OldRootより後ろのComponentを指している
		// 例 : FilePath       : Asset/Character/Player/Texture
		//      OldRoot        : Asset/Character
		//      現在医師       : Player
		//      NewRoot        : Asset/GameCharacter
		// Player/TextureをNewRootの後ろへ追加して
		// Asset/GameCharacter/Player/Texture
		// といった具合のファイルパスを作成する
		while (l_filePathITR != a_filePath.end())
		{
			l_result /= *l_filePathITR;

			++l_filePathITR;
		}

		return l_result;
 	}
}