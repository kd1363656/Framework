#pragma once

namespace FWK::Utility
{
    // a_filePathがa_rootDirectoryPath自身、又はその子階層に存在しているかを返す
    // (シーンの保存先がAssetフォルダの中かどうかの確認や、ReplaceRootでの置き換え前の確認に使う)
    inline bool IsFilePathUnderRootDirectory(const std::filesystem::path& a_filePath, const std::filesystem::path& a_rootDirectoryPath)
    {
        if (a_filePath.empty() ||
            a_rootDirectoryPath.empty())
        {
            return false;
        }

        auto l_filePathITR = a_filePath.begin();

        // 例 FilePath : Asset/Character/Player/Texture
        //    Root     : Asset/Character
        // の場合、Asset     == Asset
        //         Character == Character
        // なのでRoot配下に存在していると判断できる
        // Path全体をstringへ変換して判断せず
        // std::filesystem::pathのComponent単位で比較する
        // (Component単位なので"Asset/Data"と"Asset\Data"のように区切り文字が違っても同じとみなされる)
        for (const auto& l_rootDirectoryComponent : a_rootDirectoryPath)
        {
            // Rootを最後まで比較する前にFilePath側が終了した場合
            // 例 : FilePath : Asset
            //      Root     : Asset/Character
            if (l_filePathITR == a_filePath.end() ||
                *l_filePathITR != l_rootDirectoryComponent)
            {
                return false;
            }

            ++l_filePathITR;
        }

        return true;
    }

    inline std::filesystem::path ReplaceRoot(const std::filesystem::path& a_filePath, const std::filesystem::path& a_oldRootDirectoryPath, const std::filesystem::path& a_newRootDirectoryPath)
    {
        if (a_newRootDirectoryPath.empty() ||
            !IsFilePathUnderRootDirectory(a_filePath, a_oldRootDirectoryPath))
        {
            return a_filePath;
        }

        // OldRootの階層数(Componentの数)だけFilePath側を読み飛ばす
        // 例 : OldRoot : Asset/Character -> 2つ読み飛ばしてPlayerを指す
        auto l_filePathITR = a_filePath.begin();

        std::advance(l_filePathITR, std::distance(a_oldRootDirectoryPath.begin(), a_oldRootDirectoryPath.end()));

        // Old部分をNewRootへ置き換える
        std::filesystem::path l_result = a_newRootDirectoryPath;

        // この時点でFilePathITRは
        // OldRootより後ろのComponentを指している
        // 例 : FilePath       : Asset/Character/Player/Texture
        //      OldRoot        : Asset/Character
        //      現在位置       : Player
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